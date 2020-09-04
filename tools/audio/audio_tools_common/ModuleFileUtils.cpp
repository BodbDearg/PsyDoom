#include "ModuleFileUtils.h"

#include "Module.h"

#include <cstdio>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a 'Module' data structure to the .json file at the given path.
// If writing fails return 'false' and give an error reason in the output string.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ModuleFileUtils::writeJsonFile(const char* const jsonFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept {
    // Write the module to a JSON document with an object at the root
    rapidjson::Document document;
    document.SetObject();
    moduleIn.writeToJson(document);

    // Write the json to the given file
    std::FILE* const pJsonFile = std::fopen(jsonFilePath, "w");

    if (!pJsonFile)
        return false;

    bool bFileWrittenOk = false;

    try {
        char writeBuffer[4096];
        rapidjson::FileWriteStream writeStream(pJsonFile, writeBuffer, sizeof(writeBuffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> fileWriter(writeStream);
        document.Accept(fileWriter);
        bFileWrittenOk = true;
    } catch (...) {
        // Ignore...
    }

    if (std::fflush(pJsonFile) != 0) {
        bFileWrittenOk = false;
    }

    // Cleanup and set an error message if something went wrong
    std::fclose(pJsonFile);

    if (!bFileWrittenOk) {
        errorMsgOut = "Error writing to the .json file '";
        errorMsgOut += jsonFilePath;
        errorMsgOut += "'!";
    }

    return bFileWrittenOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a .WMD (Williams Module File) from the given path and store in the given 'Module' data structure.
// If reading fails return 'false' and give an error reason in the output string.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ModuleFileUtils::readWmdFile(const char* const wmdFilePath, Module& moduleOut, std::string& errorMsgOut) noexcept {
    // Open the WMD file
    FILE* const pWmdFile = std::fopen(wmdFilePath, "rb");

    if (!pWmdFile) {
        errorMsgOut = "Could not open the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! Is the path valid?";
        return false;
    }

    // This lambda will read from the WMD file
    const StreamReadFunc fileReader = [=](void* const pDst, const size_t size) noexcept(false) {
        // Zero sized read operations always succeed
        if (size == 0)
            return;

        // Are we reading or seeking past bytes? (No destination means skipping bytes)
        if (pDst) {
            if (std::fread(pDst, size, 1, pWmdFile) != 1)
                throw std::exception("Error reading from file!");
        } else {
            if (std::fseek(pWmdFile, (long) size, SEEK_CUR) != 0)
                throw std::exception("Error seeking within file!");
        }
    };

    // Attempt to read the .WMD file into the 'Module' data structure
    bool bReadModuleOk = false;

    try {
        moduleOut.readFromWmd(fileReader);
        bReadModuleOk = true;
    } catch (std::exception e) {
        errorMsgOut = "An error occurred while reading the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! It may be corrupt. Error reason: ";
        errorMsgOut += e.what();
    } catch (...) {
        errorMsgOut = "An unknown error occurred while reading the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! It may be corrupt.";
    }

    // Close up the input .WMD file and return the result
    std::fclose(pWmdFile);
    return bReadModuleOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a 'Module' data structure to the .WMD file (Williams Module File) at the given path.
// If writing fails return 'false' and give an error reason in the output string.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ModuleFileUtils::writeWmdFile(const char* const wmdFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept {
    // Open the WMD file
    FILE* const pWmdFile = std::fopen(wmdFilePath, "wb");

    if (!pWmdFile) {
        errorMsgOut = "Could not open the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! Is the path valid?";
        return false;
    }

    // This lambda will write to the WMD file
    const StreamWriteFunc fileWriter = [=](const void* const pDst, const size_t size) noexcept(false) {
        // Zero sized write operations always succeed
        if (size == 0)
            return;
        
        if (std::fwrite(pDst, size, 1, pWmdFile) != 1)
            throw std::exception("Error writing to file!");
    };

    // Attempt to write the 'Module' data structure to the .WMD file
    bool bModuleWrittenOk = false;

    try {
        moduleIn.writeToWmd(fileWriter);
        bModuleWrittenOk = true;
    } catch (std::exception e) {
        errorMsgOut = "An error occurred while writing to the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! Error reason: ";
        errorMsgOut += e.what();
    } catch (...) {
        errorMsgOut = "An unknown error occurred while writing to the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'!";
    }

    // Pad the file out to 2,048 byte multiples.
    // This is the way the .WMD files are stored on the PSX Doom CD.
    {
        std::byte zeroBytes[2048];
        const long fileLoc = std::ftell(pWmdFile);
        const long paddedFileSize = ((fileLoc + 2047) / 2048) * 2048;
        const long numPadBytes = paddedFileSize - fileLoc;

        if (numPadBytes > 0) {
            std::memset(zeroBytes, 0, numPadBytes);

            if (std::fwrite(zeroBytes, numPadBytes, 1, pWmdFile) != 1) {
                errorMsgOut = "An error occurred while writing to the .WMD file '";
                errorMsgOut += wmdFilePath;
                errorMsgOut += "'! Error reason: Error writing to file!";
                bModuleWrittenOk = false;
            }
        }
    }

    // Flush the output to make sure it was successfully written
    if (bModuleWrittenOk && (std::fflush(pWmdFile) != 0)) {
        errorMsgOut = "An error occurred while finishing up writing to the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'!";
        bModuleWrittenOk = false;
    }

    // Close up the input .WMD file and return the result
    std::fclose(pWmdFile);
    return bModuleWrittenOk;
}
