//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities for writing the high level results of demo execution (basic player state) to a json file and for verifying demo execution
// results against those previously saved to a json file.
//
// Used to help implement automated testing of the game logic, to verify that it behaves the same as the original .EXE given the exact
// same set of inputs. Verfies basically that there is no demo 'de-sync' against demos recorded using the original game.
//
// In order to generate the expected results (json) for test, a demo recorded against the original PSXDOOM.EXE should be played back in
// PsyDoom and manually verified to be OK, and have it's results written out to a json file. Once that is done, future result checking
// of the same demo can be automated.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "DemoResult.h"

#include "Doom/Game/g_game.h"
#include "FileUtils.h"
#include "Finally.h"
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

BEGIN_NAMESPACE(DemoResult)

// Default 'null' value that can be returned when looking up json fields
static const rapidjson::Value JSON_NULL(rapidjson::kNullType);

//------------------------------------------------------------------------------------------------------------------------------------------
// Save the demo result consisting of the the player's main attributes to the given json file.
// Returns 'false' on failure to save.
//------------------------------------------------------------------------------------------------------------------------------------------
bool saveToJsonFile(const char* const jsonFilePath) noexcept {
    // Create the json document
    rapidjson::Document document;
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.SetObject();

    // Add the player to the document
    {
        player_t& player = gPlayers[0];
        mobj_t& mobj = *player.mo;

        rapidjson::Value playerJson(rapidjson::kObjectType);

        // Position angle and momentum
        playerJson.AddMember("x", mobj.x, allocator);
        playerJson.AddMember("y", mobj.y, allocator);
        playerJson.AddMember("z", mobj.z, allocator);
        playerJson.AddMember("angle", mobj.angle, allocator);
        playerJson.AddMember("momx", mobj.momx, allocator);
        playerJson.AddMember("momy", mobj.momy, allocator);
        playerJson.AddMember("momz", mobj.momz, allocator);

        // Health and armor
        playerJson.AddMember("health", player.health, allocator);
        playerJson.AddMember("armorpoints", player.armorpoints, allocator);
        playerJson.AddMember("armortype", player.armortype, allocator);

        // How long left for each power
        {
            rapidjson::Value powersJson(rapidjson::kArrayType);

            for (int32_t i = 0; i < NUMPOWERS; ++i) {
                powersJson.PushBack(player.powers[i], allocator);
            }

            playerJson.AddMember("powers", powersJson, allocator);
        }

        // Which keycards are owned
        {
            rapidjson::Value cardsJson(rapidjson::kArrayType);

            for (int32_t i = 0; i < NUMCARDS; ++i) {
                cardsJson.PushBack(player.cards[i], allocator);
            }

            playerJson.AddMember("cards", cardsJson, allocator);
        }

        // Backpack ownership and equipped weapon
        playerJson.AddMember("backpack", player.backpack, allocator);
        playerJson.AddMember("readyweapon", player.readyweapon, allocator);

        // Which weapons are owned
        {
            rapidjson::Value weaponownedJson(rapidjson::kArrayType);

            for (int32_t i = 0; i < NUMWEAPONS; ++i) {
                weaponownedJson.PushBack(player.weaponowned[i], allocator);
            }

            playerJson.AddMember("weaponowned", weaponownedJson, allocator);
        }

        // Ammo amounts
        {
            rapidjson::Value ammoJson(rapidjson::kArrayType);

            for (int32_t i = 0; i < NUMAMMO; ++i) {
                ammoJson.PushBack(player.ammo[i], allocator);
            }

            playerJson.AddMember("ammo", ammoJson, allocator);
        }
    
        // Level stats
        playerJson.AddMember("killcount", player.killcount, allocator);
        playerJson.AddMember("itemcount", player.itemcount, allocator);
        playerJson.AddMember("secretcount", player.secretcount, allocator);

        // Add the player to the document
        document.AddMember("player", playerJson, allocator);
    }

    // Write the result to the given file
    std::FILE* const pFile = std::fopen(jsonFilePath, "w");

    if (!pFile)
        return false;

    auto closeFile = finally([&]() noexcept {
        std::fflush(pFile);
        std::fclose(pFile);
    });

    try {
        char writeBuffer[4096];
        rapidjson::FileWriteStream writeStream(pFile, writeBuffer, C_ARRAY_SIZE(writeBuffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> fileWriter(writeStream);
        document.Accept(fileWriter);
    } catch (...) {
        return false;
    }

    // All good if we get to here!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the specified field from the given json object.
// Returns a 'null' json value on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
static const rapidjson::Value& getJsonFieldOrNull(const rapidjson::Value& jsonObj, const char* const fieldName) noexcept {
    if (!jsonObj.IsObject())
        return JSON_NULL;

    auto iter = jsonObj.FindMember(fieldName);

    if (iter == jsonObj.MemberEnd())
        return JSON_NULL;
    
    return iter->value;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verify a single field inside the given json object matches the expected value
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static bool verifyJsonFieldMatches(const rapidjson::Value& jsonObj, const char* const fieldName, const T expectedVal) noexcept {
    const rapidjson::Value& field = getJsonFieldOrNull(jsonObj, fieldName);
    return (field.Is<T>() && (field.Get<T>() == expectedVal));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verify an array field inside the given json object matches the expected value
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static bool verifyJsonArrayFieldMatches(
    const rapidjson::Value& jsonObj,
    const char* const arrayFieldName,
    const T* const expectedValues,
    const unsigned arraySize
) {
    const rapidjson::Value& field = getJsonFieldOrNull(jsonObj, arrayFieldName);

    if ((!field.IsArray()) || (field.Size() != arraySize))
        return false;

    for (unsigned i = 0; i < arraySize; ++i) {
        const rapidjson::Value& arrayValue = field[i];
        
        if ((!arrayValue.Is<T>()) || (arrayValue.Get<T>() != expectedValues[i])) {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verify the current demo result consisting of the the player's main attributes matches the one stored in the given json file.
// Returns 'true' if the result matches.
//------------------------------------------------------------------------------------------------------------------------------------------
bool verifyMatchesJsonFileResult(const char* const jsonFilePath) noexcept {
    // Read the input json file
    std::byte* pJsonFileBytes = nullptr;
    size_t jsonFileSize = 0;

    if (!FileUtils::getContentsOfFile(jsonFilePath, pJsonFileBytes, jsonFileSize, 8, (std::byte) 0))
        return false;

    auto freeJsonMem = finally([&]() noexcept {
        delete[] pJsonFileBytes;
    });

    // Parse the json
    rapidjson::Document document;

    if (document.ParseInsitu((char*) pJsonFileBytes).HasParseError())
        return false;

    // Validate everything
    if (!document.HasMember("player"))
        return false;

    // Validate the demo result
    player_t& player = gPlayers[0];
    mobj_t& mobj = *player.mo;

    rapidjson::Value& playerJson = document["player"];

    return (
        verifyJsonFieldMatches(playerJson, "x", mobj.x) &&
        verifyJsonFieldMatches(playerJson, "y", mobj.y) &&
        verifyJsonFieldMatches(playerJson, "z", mobj.z) &&
        verifyJsonFieldMatches(playerJson, "angle", mobj.angle) &&
        verifyJsonFieldMatches(playerJson, "momx", mobj.momx) &&
        verifyJsonFieldMatches(playerJson, "momy", mobj.momy) &&
        verifyJsonFieldMatches(playerJson, "momz", mobj.momz) &&
        verifyJsonFieldMatches(playerJson, "health", mobj.health) &&
        verifyJsonFieldMatches(playerJson, "armorpoints", player.armorpoints) &&
        verifyJsonFieldMatches(playerJson, "armortype", player.armortype) &&
        verifyJsonArrayFieldMatches(playerJson, "powers", player.powers, NUMPOWERS) &&
        verifyJsonArrayFieldMatches(playerJson, "cards", player.cards, NUMCARDS) &&
        verifyJsonFieldMatches(playerJson, "backpack", player.backpack) &&
        verifyJsonFieldMatches(playerJson, "readyweapon", (int32_t) player.readyweapon) &&
        verifyJsonArrayFieldMatches(playerJson, "weaponowned", player.weaponowned, NUMWEAPONS) &&
        verifyJsonArrayFieldMatches(playerJson, "ammo", player.ammo, NUMAMMO) &&
        verifyJsonFieldMatches(playerJson, "killcount", player.killcount) &&
        verifyJsonFieldMatches(playerJson, "itemcount", player.itemcount) &&
        verifyJsonFieldMatches(playerJson, "secretcount", player.secretcount)
    );

    return true;
}

END_NAMESPACE(EndResult)
