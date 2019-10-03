/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* CDInterface_MT.cpp - Multi-threaded CD Reading Interface
**  Copyright (C) 2009-2018 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <mednafen/mednafen.h>
#include "CDInterface_MT.h"

using namespace CDUtility;

CDInterface_MT::CDInterface_Message::CDInterface_Message()
{
 message = 0;

 memset(args, 0, sizeof(args));
}

CDInterface_MT::CDInterface_Message::CDInterface_Message(unsigned int message_, uint32 arg0, uint32 arg1, uint32 arg2, uint32 arg3)
{
 message = message_;
 args[0] = arg0;
 args[1] = arg1;
 args[2] = arg2;
 args[3] = arg3;
}

CDInterface_MT::CDInterface_Message::CDInterface_Message(unsigned int message_, const std::string &str)
{
 message = message_;
 str_message = str;
}

CDInterface_MT::CDInterface_Message::~CDInterface_Message()
{

}

CDInterface_MT::CDInterface_Queue::CDInterface_Queue()
{
 ze_mutex = MThreading::CreateMutex();
 ze_cond = MThreading::CreateCond();
}

CDInterface_MT::CDInterface_Queue::~CDInterface_Queue()
{
 MThreading::DestroyMutex(ze_mutex);
 MThreading::DestroyCond(ze_cond);
}

// Returns false if message not read, true if it was read.  Will always return true if "blocking" is set.
// Will throw MDFN_Error if the read message code is CDInterface_MSG_FATAL_ERROR
bool CDInterface_MT::CDInterface_Queue::Read(CDInterface_Message *message, bool blocking)
{
 bool ret = true;

 //
 //
 //
 MThreading::LockMutex(ze_mutex);

 if(blocking)
 {
  while(ze_queue.size() == 0)	// while, not just if.
  {
   MThreading::WaitCond(ze_cond, ze_mutex);
  }
 }

 if(ze_queue.size() == 0)
  ret = false;
 else
 {
  *message = ze_queue.front();
  ze_queue.pop();
 }  

 MThreading::UnlockMutex(ze_mutex);
 //
 //
 //

 if(ret && message->message == CDInterface_MSG_FATAL_ERROR)
  throw MDFN_Error(0, "%s", message->str_message.c_str());

 return ret;
}

void CDInterface_MT::CDInterface_Queue::Write(const CDInterface_Message &message)
{
 MThreading::LockMutex(ze_mutex);

 try
 {
  ze_queue.push(message);
 }
 catch(...)
 {
  fprintf(stderr, "\n\nCDInterface_Message queue push failed!!!  (We now return you to your regularly unscheduled lockup)\n\n");
 }

 MThreading::SignalCond(ze_cond);	// Signal while the mutex is held to prevent icky race conditions.

 MThreading::UnlockMutex(ze_mutex);
}

static int ReadThreadStart_C(void* arg)
{
 return ((CDInterface_MT*)arg)->ReadThreadStart();
}

int CDInterface_MT::ReadThreadStart()
{
 bool Running = true;

 SBWritePos = 0;
 ra_lba = 0;
 ra_count = 0;
 last_read_lba = LBA_Read_Maximum + 1;

 try
 {
  disc_cdaccess->Read_TOC(&disc_toc);

  if(disc_toc.first_track < 1 || disc_toc.last_track > 99 || disc_toc.first_track > disc_toc.last_track)
  {
   throw(MDFN_Error(0, _("TOC first(%d)/last(%d) track numbers bad."), disc_toc.first_track, disc_toc.last_track));
  }

  SBWritePos = 0;
  ra_lba = 0;
  ra_count = 0;
  last_read_lba = LBA_Read_Maximum + 1;
  memset(SectorBuffers, 0, SBSize * sizeof(CDInterface_Sector_Buffer));
 }
 catch(std::exception &e)
 {
  EmuThreadQueue.Write(CDInterface_Message(CDInterface_MSG_FATAL_ERROR, std::string(e.what())));
  return 0;
 }

 EmuThreadQueue.Write(CDInterface_Message(CDInterface_MSG_DONE));

 while(Running)
 {
  CDInterface_Message msg;

  //printf("%d %d %d\n", last_read_lba, ra_lba, ra_count);

  // Only do a blocking-wait for a message if we don't have any sectors to read-ahead.
  if(ReadThreadQueue.Read(&msg, ra_count ? false : true))
  {
   if(msg.message == CDInterface_MSG_DIEDIEDIE)
    Running = false;
   else if(msg.message == CDInterface_MSG_READ_SECTOR)
   {
    static const int max_ra = 16;
    static const int initial_ra = 1;
    static const int speedmult_ra = 2;
    //
    const int32 new_lba = msg.args[0];

    static_assert((unsigned int)max_ra < (SBSize / 4), "Max readahead too large.");

    if(new_lba == (last_read_lba + 1))
    {
     int how_far_ahead = ra_lba - new_lba;

     if(how_far_ahead <= max_ra)
      ra_count = std::min(speedmult_ra, 1 + max_ra - how_far_ahead);
     else
      ra_count++;
    }
    else if(new_lba != last_read_lba)
    {
     ra_lba = new_lba;
     ra_count = initial_ra;
    }

    last_read_lba = new_lba;
   }
  }

  //
  // Don't read beyond what the disc (image) readers can handle sanely.
  //
  if(ra_count && ra_lba == LBA_Read_Maximum)
  {
   ra_count = 0;
   //printf("Ephemeral scarabs: %d!\n", ra_lba);
  }

  if(ra_count)
  {
   uint8 tmpbuf[2352 + 96];
   bool error_condition = false;

   try
   {
    disc_cdaccess->Read_Raw_Sector(tmpbuf, ra_lba);
   }
   catch(std::exception &e)
   {
    MDFN_Notify(MDFN_NOTICE_ERROR, _("Sector %u read error: %s"), ra_lba, e.what());
    memset(tmpbuf, 0, sizeof(tmpbuf));
    error_condition = true;
   }
   
   //
   //
   MThreading::LockMutex(SBMutex);

   SectorBuffers[SBWritePos].lba = ra_lba;
   memcpy(SectorBuffers[SBWritePos].data, tmpbuf, 2352 + 96);
   SectorBuffers[SBWritePos].valid = true;
   SectorBuffers[SBWritePos].error = error_condition;
   SBWritePos = (SBWritePos + 1) % SBSize;

   MThreading::SignalCond(SBCond);

   MThreading::UnlockMutex(SBMutex);
   //
   //

   ra_lba++;
   ra_count--;
  }
 }

 return 1;
}

CDInterface_MT::CDInterface_MT(std::unique_ptr<CDAccess> cda) : disc_cdaccess(std::move(cda)), CDReadThread(NULL), SBMutex(NULL), SBCond(NULL)
{
 try
 {
  CDInterface_Message msg;

  if(!(SBMutex = MThreading::CreateMutex()))
   throw MDFN_Error(0, _("Error creating CD read thread mutex."));

  if(!(SBCond = MThreading::CreateCond()))
   throw MDFN_Error(0, _("Error creating CD read thread condition variable."));

  UnrecoverableError = false;

  if(!(CDReadThread = MThreading::CreateThread(ReadThreadStart_C, this, "MDFN CD Read")))
   throw MDFN_Error(0, _("Error creating CD read thread."));

  EmuThreadQueue.Read(&msg);
 }
 catch(...)
 {
  if(CDReadThread)
  {
   MThreading::WaitThread(CDReadThread, NULL);
   CDReadThread = NULL;
  }

  if(SBMutex)
  {
   MThreading::DestroyMutex(SBMutex);
   SBMutex = NULL;
  }

  if(SBCond)
  {
   MThreading::DestroyCond(SBCond);
   SBCond = NULL;
  }

  throw;
 }
}


CDInterface_MT::~CDInterface_MT()
{
 bool thread_deaded_failed = false;

 try
 {
  ReadThreadQueue.Write(CDInterface_Message(CDInterface_MSG_DIEDIEDIE));
 }
 catch(std::exception &e)
 {
  MDFND_OutputNotice(MDFN_NOTICE_ERROR, e.what());
  thread_deaded_failed = true;
 }

 if(!thread_deaded_failed)
  MThreading::WaitThread(CDReadThread, NULL);

 if(SBMutex)
 {
  MThreading::DestroyMutex(SBMutex);
  SBMutex = NULL;
 }

 if(SBCond)
 {
  MThreading::DestroyCond(SBCond);
  SBCond = NULL;
 }
}

bool CDInterface_MT::ReadRawSector(uint8 *buf, int32 lba)
{
 bool found = false;
 bool error_condition = false;

 if(UnrecoverableError)
 {
  memset(buf, 0, 2352 + 96);
  return false;
 }

 if(lba < LBA_Read_Minimum || lba > LBA_Read_Maximum)
 {
  printf("Attempt to read sector out of bounds; LBA=%d\n", lba);
  memset(buf, 0, 2352 + 96);
  return false;
 }
 //fprintf(stderr, "%d\n", ra_lba - lba);

 ReadThreadQueue.Write(CDInterface_Message(CDInterface_MSG_READ_SECTOR, lba));

 //
 //
 //
 MThreading::LockMutex(SBMutex);

 do
 {
  for(int i = 0; i < SBSize; i++)
  {
   if(SectorBuffers[i].valid && SectorBuffers[i].lba == lba)
   {
    error_condition = SectorBuffers[i].error;
    memcpy(buf, SectorBuffers[i].data, 2352 + 96);
    found = true;
   }
  }

  if(!found)
  {
   //int32 swt = MDFND_GetTime();
   MThreading::WaitCond(SBCond, SBMutex);
   //printf("SB Waited: %d\n", MDFND_GetTime() - swt);
  }
 } while(!found);

 MThreading::UnlockMutex(SBMutex);
 //
 //
 //
 return !error_condition;
}

bool CDInterface_MT::ReadRawSectorPWOnly(uint8* pwbuf, int32 lba, bool hint_fullread)
{
 if(UnrecoverableError)
 {
  memset(pwbuf, 0, 96);
  return false;
 }

 if(lba < LBA_Read_Minimum || lba > LBA_Read_Maximum)
 {
  printf("Attempt to read sector out of bounds; LBA=%d\n", lba);
  memset(pwbuf, 0, 96);
  return false;
 }

 if(disc_cdaccess->Fast_Read_Raw_PW_TSRE(pwbuf, lba))
 {
  if(hint_fullread)
   ReadThreadQueue.Write(CDInterface_Message(CDInterface_MSG_READ_SECTOR, lba));

  return true;
 }
 else
 {
  uint8 tmpbuf[2352 + 96];
  bool ret;

  ret = ReadRawSector(tmpbuf, lba);
  memcpy(pwbuf, tmpbuf + 2352, 96);

  return ret;
 }
}

void CDInterface_MT::HintReadSector(int32 lba)
{
 if(UnrecoverableError)
  return;

 ReadThreadQueue.Write(CDInterface_Message(CDInterface_MSG_READ_SECTOR, lba));
}
