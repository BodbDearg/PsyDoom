#include "ModuleFileUtils.h"

#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "FileUtils.h"
#include "Module.h"

#include <cstdio>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a 'Module' data structure from the .json file at the given path.
// If reading fails return 'false' and give an error reason in the output string.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ModuleFileUtils::readJsonFile(const char* const jsonFilePath, Module& moduleOut, std::string& errorMsgOut) noexcept {
    // Read the input json file
    const FileData fileData = FileUtils::getContentsOfFile(jsonFilePath, 8, std::byte(0));

    if (!fileData.bytes) {
        errorMsgOut = "Could not read the .json file '";
        errorMsgOut += jsonFilePath;
        errorMsgOut += "'! Is the path valid or readable?";
        return false;
    }

    // Parse the json
    rapidjson::Document jsonDoc;

    if (jsonDoc.ParseInsitu((char*) fileData.bytes.get()).HasParseError()) {
        errorMsgOut = "Failed to parse the .json file '";
        errorMsgOut += jsonFilePath;
        errorMsgOut += "'! File may not be valid JSON.";
        return false;
    }

    // Try to read the module from the json
    bool bReadModuleOk = false;

    try {
        moduleOut.readFromJson(jsonDoc);
        bReadModuleOk = true;
    } catch (const char* const exceptionMsg) {
        errorMsgOut = "An error occurred while reading a module from the .json file '";
        errorMsgOut += jsonFilePath;
        errorMsgOut += "'! It may not be a valid module definition. Error reason: ";
        errorMsgOut += exceptionMsg;
    } catch (...) {
        errorMsgOut = "An unknown error occurred while reading a module from the .json file '";
        errorMsgOut += jsonFilePath;
        errorMsgOut += "'! It may not be a valid module definition.";
    }

    return bReadModuleOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a 'Module' data structure to the .json file at the given path.
// If writing fails return 'false' and give an error reason in the output string.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ModuleFileUtils::writeJsonFile(const char* const jsonFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept {
    // Write the module to a JSON document with an object at the root
    rapidjson::Document jsonDoc;
    jsonDoc.SetObject();
    moduleIn.writeToJson(jsonDoc);

    // Write the json to the given file
    std::FILE* const pJsonFile = std::fopen(jsonFilePath, "w");

    if (!pJsonFile) {
        errorMsgOut = "Could not open the .json file '";
        errorMsgOut += jsonFilePath;
        errorMsgOut += "' for writing! Is the path valid or writable?";
        return false;
    }

    bool bFileWrittenOk = false;

    try {
        char writeBuffer[4096];
        rapidjson::FileWriteStream writeStream(pJsonFile, writeBuffer, sizeof(writeBuffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> fileWriter(writeStream);
        jsonDoc.Accept(fileWriter);
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
    bool bReadModuleOk = false;

    try {
        FileInputStream fileIn(wmdFilePath);
        moduleOut.readFromWmdFile(fileIn);
        bReadModuleOk = true;
    } catch (const char* const exceptionMsg) {
        errorMsgOut = "An error occurred while reading the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! It may be corrupt. Error reason: ";
        errorMsgOut += exceptionMsg;
    } catch (...) {
        errorMsgOut = "Failed to read .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! It may be corrupt or may not exist.";
    }

    return bReadModuleOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a 'Module' data structure to the .WMD file (Williams Module File) at the given path.
// If writing fails return 'false' and give an error reason in the output string.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ModuleFileUtils::writeWmdFile(const char* const wmdFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept {
    // Attempt to write the 'Module' data structure to the .WMD file
    bool bModuleWrittenOk = false;

    try {
        // Write the entire module file
        FileOutputStream fileOut(wmdFilePath, false);
        moduleIn.writeToWmdFile(fileOut);

        // Pad the file out to 2,048 byte multiples.
        // This is the way the .WMD files are stored on the PSX Doom CD.
        fileOut.padAlign(2048, std::byte(0));

        // Flush to make sure all bytes are written to finish up
        fileOut.flush();
        bModuleWrittenOk = true;
    } catch (const char* const exceptionMsg) {
        errorMsgOut = "An error occurred while writing to the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'! Error reason: ";
        errorMsgOut += exceptionMsg;
    } catch (...) {
        errorMsgOut = "An error occurred while writing to the .WMD file '";
        errorMsgOut += wmdFilePath;
        errorMsgOut += "'!";
    }

    return bModuleWrittenOk;
}
