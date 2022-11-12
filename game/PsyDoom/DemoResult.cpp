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
#include "Doom/UI/in_main.h"

#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

BEGIN_NAMESPACE(DemoResult)

// Default 'null' value that can be returned when looking up json fields
static const rapidjson::Value JSON_NULL(rapidjson::kNullType);

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

    if ((!field.Is<T>()) || (field.Get<T>() != expectedVal))
        return false;

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verify an array field inside the given json object matches the expected value
//------------------------------------------------------------------------------------------------------------------------------------------
template <class JsonT, class CppT>
static bool verifyJsonArrayFieldMatches(
    const rapidjson::Value& jsonObj,
    const char* const arrayFieldName,
    const CppT* const expectedValues,
    const unsigned arraySize
) {
    const rapidjson::Value& field = getJsonFieldOrNull(jsonObj, arrayFieldName);

    if ((!field.IsArray()) || (field.Size() != arraySize))
        return false;

    for (unsigned i = 0; i < arraySize; ++i) {
        const rapidjson::Value& arrayValue = field[i];

        if ((!arrayValue.Is<JsonT>()) || ((CppT) arrayValue.Get<JsonT>() != expectedValues[i])) {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Serializes and adds the player to the specified json object (or document) via the given member name
//------------------------------------------------------------------------------------------------------------------------------------------
static void addPlayerToJson(
    rapidjson::Value& jsonRoot,
    rapidjson::Document::AllocatorType& jsonAllocator,
    player_t& player,
    const char* const jsonMemberName
) noexcept {
    // Position angle and momentum
    rapidjson::Value playerJson(rapidjson::kObjectType);

    {
        mobj_t& mobj = *player.mo;
        playerJson.AddMember("x", mobj.x, jsonAllocator);
        playerJson.AddMember("y", mobj.y, jsonAllocator);
        playerJson.AddMember("z", mobj.z, jsonAllocator);
        playerJson.AddMember("angle", mobj.angle, jsonAllocator);
        playerJson.AddMember("momx", mobj.momx, jsonAllocator);
        playerJson.AddMember("momy", mobj.momy, jsonAllocator);
        playerJson.AddMember("momz", mobj.momz, jsonAllocator);
    }

    // Health and armor
    playerJson.AddMember("health", player.health, jsonAllocator);
    playerJson.AddMember("armorpoints", player.armorpoints, jsonAllocator);
    playerJson.AddMember("armortype", player.armortype, jsonAllocator);

    // How long left for each power
    {
        rapidjson::Value powersJson(rapidjson::kArrayType);

        for (int32_t i = 0; i < NUMPOWERS; ++i) {
            powersJson.PushBack(player.powers[i], jsonAllocator);
        }

        playerJson.AddMember("powers", powersJson, jsonAllocator);
    }

    // Which keycards are owned
    {
        rapidjson::Value cardsJson(rapidjson::kArrayType);

        for (int32_t i = 0; i < NUMCARDS; ++i) {
            cardsJson.PushBack((int32_t) player.cards[i], jsonAllocator);   // N.B: serialize as int
        }

        playerJson.AddMember("cards", cardsJson, jsonAllocator);
    }

    // Backpack ownership and equipped weapon
    playerJson.AddMember("backpack", (int32_t) player.backpack, jsonAllocator);     // N.B: serialize as int
    playerJson.AddMember("readyweapon", player.readyweapon, jsonAllocator);

    // Which weapons are owned
    {
        rapidjson::Value weaponownedJson(rapidjson::kArrayType);

        for (int32_t i = 0; i < NUMWEAPONS; ++i) {
            weaponownedJson.PushBack((int32_t) player.weaponowned[i], jsonAllocator);   // N.B: serialize as int
        }

        playerJson.AddMember("weaponowned", weaponownedJson, jsonAllocator);
    }

    // Ammo amounts
    {
        rapidjson::Value ammoJson(rapidjson::kArrayType);

        for (int32_t i = 0; i < NUMAMMO; ++i) {
            ammoJson.PushBack(player.ammo[i], jsonAllocator);
        }

        playerJson.AddMember("ammo", ammoJson, jsonAllocator);
    }

    // Level stats

  
    playerJson.AddMember("killcount", player.killcount, jsonAllocator);
    playerJson.AddMember("itemcount", player.itemcount, jsonAllocator);
    playerJson.AddMember("secretcount", player.secretcount, jsonAllocator);

    if (gNetGame == gt_deathmatch) {
        playerJson.AddMember("frags", player.frags, jsonAllocator);
    }

    // Add the player to the document
    {
        rapidjson::Value rapidName(jsonMemberName, jsonAllocator);
        jsonRoot.AddMember(rapidName, playerJson, jsonAllocator);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verifies that the specified player matches the json result in the given json object
//------------------------------------------------------------------------------------------------------------------------------------------
static bool verifyPlayerMatchesJson(player_t& player, const rapidjson::Value& playerJson) noexcept {
    // A null object can never match!
    if (playerJson.IsNull())
        return false;

    // Check all the fields of interest:
    mobj_t& mobj = *player.mo;
    const bool bDeathmatch = (gNetGame == gt_deathmatch);

    return (
        verifyJsonFieldMatches(playerJson, "x", mobj.x.value) &&
        verifyJsonFieldMatches(playerJson, "y", mobj.y.value) &&
        verifyJsonFieldMatches(playerJson, "z", mobj.z.value) &&
        verifyJsonFieldMatches(playerJson, "angle", mobj.angle) &&
        verifyJsonFieldMatches(playerJson, "momx", mobj.momx) &&
        verifyJsonFieldMatches(playerJson, "momy", mobj.momy) &&
        verifyJsonFieldMatches(playerJson, "momz", mobj.momz) &&
        verifyJsonFieldMatches(playerJson, "health", player.health) &&
        verifyJsonFieldMatches(playerJson, "armorpoints", player.armorpoints) &&
        verifyJsonFieldMatches(playerJson, "armortype", player.armortype) &&
        verifyJsonArrayFieldMatches<int32_t>(playerJson, "powers", player.powers, NUMPOWERS) &&                 // N.B: is serialized as int
        verifyJsonArrayFieldMatches<int32_t>(playerJson, "cards", player.cards, NUMCARDS) &&                    // N.B: is serialized as int
        verifyJsonFieldMatches(playerJson, "backpack", (int32_t) player.backpack) &&
        verifyJsonFieldMatches(playerJson, "readyweapon", (int32_t) player.readyweapon) &&                      // N.B: is serialized as int
        verifyJsonArrayFieldMatches<int32_t>(playerJson, "weaponowned", player.weaponowned, NUMWEAPONS) &&      // N.B: is serialized as int
        verifyJsonArrayFieldMatches<int32_t>(playerJson, "ammo", player.ammo, NUMAMMO) &&
        verifyJsonFieldMatches(playerJson, "killcount", player.killcount) &&
        verifyJsonFieldMatches(playerJson, "itemcount", player.itemcount) &&
        verifyJsonFieldMatches(playerJson, "secretcount", player.secretcount) &&
        ((!bDeathmatch) || verifyJsonFieldMatches(playerJson, "frags", player.frags))
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Save the demo result consisting of the the player's main attributes to the given json file.
// Returns 'false' on failure to save.
//------------------------------------------------------------------------------------------------------------------------------------------
bool saveToJsonFile(const char* const jsonFilePath) noexcept {
    // Create the json document
    rapidjson::Document document;
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.SetObject();

    // Are we dealing with a single player or multiplayer game? (affects naming)
    if (gNetGame == gt_single) {
        addPlayerToJson(document, allocator, gPlayers[0], "player");
    } else {
        addPlayerToJson(document, allocator, gPlayers[0], "player1");
        addPlayerToJson(document, allocator, gPlayers[1], "player2");
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
// Verify the current demo result consisting of the the player's main attributes matches the one stored in the given json file.
// Returns 'true' if the result matches.
//------------------------------------------------------------------------------------------------------------------------------------------
bool verifyMatchesJsonFileResult(const char* const jsonFilePath) noexcept {
    // Read the input json file
    const FileData fileData = FileUtils::getContentsOfFile(jsonFilePath, 8, std::byte(0));

    if (!fileData.bytes)
        return false;

    // Parse the json
    rapidjson::Document document;

    if (document.ParseInsitu((char*) fileData.bytes.get()).HasParseError())
        return false;

    // Validate the demo result
    if (gNetGame == gt_single) {
        return verifyPlayerMatchesJson(gPlayers[0], document["player"]);
    } else {
        return (
            verifyPlayerMatchesJson(gPlayers[0], document["player1"]) &&
            verifyPlayerMatchesJson(gPlayers[1], document["player2"])
        );
    }
}

END_NAMESPACE(EndResult)
