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
#include "Finally.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <rapidjson/document.h>
    #include <rapidjson/filewritestream.h>
    #include <rapidjson/prettywriter.h>
END_THIRD_PARTY_INCLUDES

BEGIN_NAMESPACE(DemoResult)

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
                cardsJson.PushBack((player.cards[i] != 0), allocator);
            }

            playerJson.AddMember("cards", cardsJson, allocator);
        }

        // Backpack ownership and equipped weapon
        playerJson.AddMember("backpack", (player.backpack != 0), allocator);
        playerJson.AddMember("readyweapon", player.readyweapon, allocator);

        // Which weapons are owned
        {
            rapidjson::Value weaponownedJson(rapidjson::kArrayType);

            for (int32_t i = 0; i < NUMCARDS; ++i) {
                weaponownedJson.PushBack((player.weaponowned[i] != 0), allocator);
            }

            playerJson.AddMember("weaponowned", weaponownedJson, allocator);
        }

        // Ammo amounts
        {
            rapidjson::Value ammoJson(rapidjson::kArrayType);

            for (int32_t i = 0; i < NUMCARDS; ++i) {
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

bool verifyMatchesJsonFileResult(const char* const jsonFilePath) noexcept {
    return true;
}

END_NAMESPACE(EndResult)
