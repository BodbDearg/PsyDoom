#ifndef __MDFN_NET_NETWS2_H
#define __MDFN_NET_NETWS2_H

// DC: This got moved to workaround issues with no symlinks on Windows
#if 1
    #include <mednafen/net/Net.h>
#else
    #include "Net.h"
#endif

namespace Net
{

std::unique_ptr<Connection> WS2_Connect(const char* host, unsigned int port);
std::unique_ptr<Connection> WS2_Accept(unsigned int port);

}

#endif
