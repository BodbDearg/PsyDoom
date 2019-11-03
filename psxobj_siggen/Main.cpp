#include <cstdio>
#include <fstream>

//----------------------------------------------------------------------------------------------------------------------
// Entry point for 'PSXObjSigGen'
//----------------------------------------------------------------------------------------------------------------------
// Program purpose:
//
//  This program parses the human readable textual output of the 'DUMPOBJ.EXE' tool included in the PlayStation SDK 
//  (PsyQ SDK) when invoked with the '/c' option (print code and data). Based on this dump output, it generates both
//  function and data signatures for the contents of the .OBJ file as well as reference disassembly for code.
//
//  These signatures and disassembly are then appended to specified output files, so the results of many invocations can
//  be gathered together to form one database of all this info. This database can then be used to try and identify the
//  same functions and data within a given PlayStation executable.
//
//  Basically this tool was created so we can identify PsyQ SDK constructs within a PlayStation executable.
//
//  The process to using this tool would be as follows:
//      (1) Dump the contents of all SDK .LIB files to .OBJ using the 'PSYLIB.EXE' tool and gather other SDK .OBJ files.
//          For more info, see: https://www.retroreversing.com/ps1-psylink
//      (2) Invoke 'DUMPOBJ.EXE' on every SDK .OBJ file to produce the textual output that is an input to this tool.
//      (3) Use this tool on all the dump output to build up the signature database.
//      (4) Run pattern matching on the .EXE using the generated signature database to identify PsyQ constructs in an EXE.
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) noexcept {
    return 0;
}
