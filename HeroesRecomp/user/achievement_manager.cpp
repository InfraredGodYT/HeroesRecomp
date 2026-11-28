#include "achievement_manager.h"
#include <os/logger.h>
#include <user/config.h>
#include <ui/achievement_overlay.h>
#include <gpu//video.h>


// achievements icons
#include <res/images/achievements/pure_hero.png.h>
#include <res/images/achievements/pure_dark.png.h>
#include <res/images/achievements/neutral_hero.png.h>
#include <res/images/achievements/neutral_dark.png.h>

#include <res/images/achievements/pure_dark_hero.png.h>
#include <res/images/achievements/pure_hero_dark.png.h>
#include <res/images/achievements/hero_hero.png.h>
#include <res/images/achievements/hero_dark.png.h>
#include <res/images/achievements/dark_dark.png.h>
#include <res/images/achievements/dark_hero.png.h>
#include <res/images/achievements/A_Rank.png.h>
#include <res/images/achievements/key.png.h>
#include <res/images/achievements/LastStoryComplete.png.h>
#include <res/images/achievements/bomb.png.h>
#include <res/images/achievements/tank.png.h>
#include <res/images/achievements/shthR.png.h>   
#include <res/images/achievements/expert.png.h> 
#include <res/images/achievements/PlaceHolder.png.h>
#include <res/images/achievements/overlay.png.h>
#include <decompressor.h>
#include <patches/global.h>

enum EAchievementID
{
    Ach_LastStory = 11,
    Ach_AllARanks = 12,
    Ach_AllKeys = 13,
    Ach_ExpertMode = 14,
    Ach_TankVehicle = 15,
    Ach_CentralCityHeroTime = 16,
};
std::unordered_map<uint16_t, std::unique_ptr<GuestTexture>> g_achievementsTextureCache;

#define NUM_RECORDS sizeof(AchievementManager::Data.Records) / sizeof(AchievementData::SHRRecord)

time_t AchievementManager::GetTimestamp(uint16_t id)
{
    for (int i = 0; i < NUM_RECORDS; i++)
    {
        if (Data.Records[i].ID == id)
            return Data.Records[i].Timestamp;
    }

    return 0;
}

size_t AchievementManager::GetTotalRecords()
{
    auto result = 0;

    for (int i = 0; i < NUM_RECORDS; i++)
    {
        if (!Data.Records[i].ID)
            break;

        result++;
    }

    return result;
}

bool AchievementManager::IsUnlocked(uint16_t id)
{
    if (!id)
        return false;

    for (int i = 0; i < NUM_RECORDS; i++)
    {
        if (Data.Records[i].ID == id)
            return true;
    }

    return false;
}

void AchievementManager::Unlock(uint16_t id)
{
    if (IsUnlocked(id) || !id)
        return;

    Data.Records[id - 1].ID = id;
    Data.Records[id - 1].Timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    if (Config::AchievementNotifications)
        AchievementOverlay::Open(id);

    // check if all achievements have been unlocked
    // if so unlock the final one
    const size_t unlockedCount = AchievementManager::GetTotalRecords();

    const uint8_t lastSlotID = SHR_RECORDS - 1;
    if (unlockedCount == lastSlotID)
    {
        Data.Records[lastSlotID].ID = SHR_RECORDS;
        Data.Records[lastSlotID].Timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        if (Config::AchievementNotifications)
            AchievementOverlay::Open(SHR_RECORDS);
    }
}

void AchievementManager::UnlockAll()
{
    for (uint16_t i = 1; i < SHR_RECORDS; i++)
        AchievementManager::Unlock(i);
}

void AchievementManager::Reset()
{
    Data = {};
}

#ifdef _DEBUG
void AchievementManager::UnlockTest()
{
    for (uint16_t i = 1; i < SHR_RECORDS; i++)
    {
        if (i == Ach_AllARanks || i == SHR_RECORDS)
            continue;

        AchievementManager::Unlock(i);
    }
}
#endif

bool AchievementManager::LoadBinary()
{

    AchievementManager::Reset();


    BinStatus = EAchBinStatus::Success;

    auto dataPath = GetDataPath(true);

    if (!std::filesystem::exists(dataPath))
    {
        // Try loading base achievement data as fallback.
        dataPath = GetDataPath(false);

        if (!std::filesystem::exists(dataPath))
            return true;
    }

    std::error_code ec;
    auto fileSize = std::filesystem::file_size(dataPath, ec);
    auto dataSize = sizeof(AchievementData);

    if (fileSize != dataSize)
    {
        BinStatus = EAchBinStatus::BadFileSize;
        return true;
    }

    std::ifstream file(dataPath, std::ios::binary);

    if (!file)
    {
        BinStatus = EAchBinStatus::IOError;
        return true;
    }

    AchievementData data{};

    file.read((char*)&data.Signature, sizeof(data.Signature));

    if (!data.VerifySignature())
    {
        BinStatus = EAchBinStatus::BadSignature;
        file.close();
        return true;
    }

    file.read((char*)&data.Version, sizeof(data.Version));

    if (!data.VerifyVersion())
    {
        BinStatus = EAchBinStatus::BadVersion;
        file.close();
        return true;
    }

    file.seekg(0);
    file.read((char*)&data, sizeof(data));

    if (!data.VerifyChecksum())
    {
        BinStatus = EAchBinStatus::BadChecksum;
        file.close();
        return true;
    }

    file.close();

    memcpy(&Data, &data, dataSize);

    return false;
}

bool AchievementManager::SaveBinary(bool ignoreStatus)
{
    if (!ignoreStatus && BinStatus != EAchBinStatus::Success)
    {
        LOGN_WARNING("Achievement data will not be saved in this session!");
        return false;
    }

    LOGN("Saving achievements...");

    std::ofstream file(GetDataPath(true), std::ios::binary);

    if (!file)
    {
        LOGN_ERROR("Failed to write achievement data.");
        return false;
    }

    Data.Checksum = Data.CalculateChecksum();

    file.write((const char*)&Data, sizeof(AchievementData));
    file.close();

    BinStatus = EAchBinStatus::Success;

    return true;
}

std::unique_ptr<GuestTexture> LoadAchivementTexture(uint16_t id)
{
    switch (id)
    {
    case 1:
        return LOAD_ZSTD_TEXTURE(g_puredark);
    case 2:
        return LOAD_ZSTD_TEXTURE(g_puredarkhero);
    case 3:
        return LOAD_ZSTD_TEXTURE(g_darkdark);
    case 4:
        return LOAD_ZSTD_TEXTURE(g_darkhero);
    case 5:
        return LOAD_ZSTD_TEXTURE(g_neutraldark);
    case 6:
        return LOAD_ZSTD_TEXTURE(g_neutralhero);
    case 7:
        return LOAD_ZSTD_TEXTURE(g_herodark);
    case 8:
        return LOAD_ZSTD_TEXTURE(g_herohero);
    case 9:
        return LOAD_ZSTD_TEXTURE(g_pureherodark);
    case 10:
        return LOAD_ZSTD_TEXTURE(g_purehero);
    case Ach_LastStory:
        return LOAD_ZSTD_TEXTURE(g_laststorycomplete);
    case Ach_AllARanks:
        return LOAD_ZSTD_TEXTURE(g_arank);
    case Ach_AllKeys:
        return LOAD_ZSTD_TEXTURE(g_key);
    case Ach_ExpertMode:
        return LOAD_ZSTD_TEXTURE(g_expert);
    case Ach_TankVehicle:
        return LOAD_ZSTD_TEXTURE(g_tank);
    case Ach_CentralCityHeroTime:
        return LOAD_ZSTD_TEXTURE(g_bomb);
    case 17:
		return LOAD_ZSTD_TEXTURE(g_shthR);
    }

    return LOAD_ZSTD_TEXTURE(g_placeholder);
}

void InitAchievementTextures()
{
    for (uint16_t i = 1; i <= SHR_RECORDS; i++)
    {
        g_achievementsTextureCache[i] = LoadAchivementTexture(i);
    }
}

std::vector<Achievement> GetAchievements(ELanguage language)
{
	std::vector<Achievement> result;

    for (int i = 0; i < SHR_RECORDS; i++)
    {

        Achievement achievement{};
        achievement.ID = i + 1;
        achievement.Name = g_shrLocale[i + 1][language].Name;
        achievement.UnlockedDesc = g_shrLocale[i + 1][language].UnlockedDesc;
        achievement.LockedDesc = g_shrLocale[i + 1][language].LockedDesc;
        achievement.pImageBuffer = (uint8_t*)g_achievementsTextureCache[i + 1].get();


        result.push_back(achievement);
    }

    return result;
}

Achievement GetAchievement(ELanguage language, const uint8_t id)
{
    Achievement achievement{};
    achievement.ID = id;
    achievement.Name = g_shrLocale[id][language].Name;
    achievement.UnlockedDesc = g_shrLocale[id][language].UnlockedDesc;
    achievement.LockedDesc = g_shrLocale[id][language].LockedDesc;
    achievement.pImageBuffer = (uint8_t*)g_achievementsTextureCache[id].get();
    return achievement;
}

// Library has 326 ending, this is checked with index 0-325
bool IsLibrarySequenceUnlocked(int index)
{
#if 0 // HEROES-TODO: Shadow save data (0x549640)
	auto base = g_memory.base;
    auto ptr = X86_LOAD_U32(0x549640);

    if (!ptr)
		return false;

    int bit = 1 << (index & 0x1F);

	uint32_t address = ptr + 4 * ((unsigned int)index >> 5) + 0x16d0;
    int word = X86_LOAD_U32(address);

    // ecx = ebx = count (index)
    //	ctx.eflags.update_test_flags_U32(X86_LOAD_U32(ctx.edx.u32 + ctx.ecx.u32 * 0x4 + 0x16d0) & ctx.eax.u32);
    return (bit & word) != 0;
#else
	return false;
#endif // HEROES-TODO
}

//  this match the route map order so far
// 0 = dark ending, this might be 0-9 to check the 10 endings
// 4 neutral dark ending 
// 5 = neutral hero ending
bool IsEndingUnlocked(uint16_t flag)
{
#if 0 // HEROES-TODO: Shadow save data (0x549640)
    auto base = g_memory.base;
    auto ptr = X86_LOAD_U32(0x549640);

    if (!ptr)
        return false;

    uint32_t address = ptr + 0xB88;

    int ptrAddr = X86_LOAD_U32(address);

    auto ptrFLag = (uint8_t*)g_memory.Translate(ptrAddr);
    if ((*((uint16_t*)ptrFLag) & (uint16_t)flag) != 0)
    {
        return true;

    }

    return false;
#else
	return false;
#endif // HEROES-TODO
}

void SetEndingAchievementMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
   const uint8_t id = ctx.ecx.u8_lo;
    if (id > 9 || id < 0)
        return;

    AchievementManager::Unlock(id + 1);
    AchievementManager::SaveBinary();
}

void UpdateARankCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
    if (ctx.edx.u32 >= 71)
    {
        AchievementManager::Unlock(Ach_AllARanks);
        AchievementManager::SaveBinary();
    }
}

// This function update key count in the save file for each stage
// The game doesn't seem to have a full key count data so we have to calculate manually
// dword_54DDDC is the index used per stage as an offset to browse the save data
// it starts at 5 in westopolis and increase following level internal order skipping bosses.
// thus, digital circuit which is the second internal stage will be 6, glyphic canyon will be 7 etc.
#if 0 // HEROES-TODO: Shadow address(es) sub_1B2C30 not matched in Heroes yet (docs/shadow-matches.md)
X86_FUNC_IMPL(__imp__sub_1B2C30);
X86_OVERRIDE_FUNC(sub_1B2C30)
{
    __imp__sub_1B2C30(ctx, base);

    auto pointerAddr = X86_LOAD_U32(0x549640);
    if (!pointerAddr)
        return;
   
    uint16_t keyCount = 0;
    // the game has 23 stages (22 on stage select + last way) 
    for (int i = 0; i < 23; i++)
    {
        int index = (i + 5); // index start at 5 because they count debug level (0-4)
   
        // check if each key has been found (5 keys per level)
        for (int j = 0; j < 5; j++)
        {
            auto indexSaveStruct = index + index * 0x2;
            indexSaveStruct <<= 3;
            auto offset = 0xf4 + (j * 4);
            uint32_t res = X86_LOAD_U32(pointerAddr + indexSaveStruct * 0x4 + offset);
            keyCount += res != 0xFFFFFFFF;
        }

    }

    if (keyCount >= 115)
    {
        AchievementManager::Unlock(Ach_AllKeys);
        AchievementManager::SaveBinary();
    }
}
#endif // HEROES-TODO

const uint32_t g_menuModeAddr = 0x5022A4;
const uint32_t g_stageActionModeAddr = 0x5496C0;
const uint32_t g_currentStageIDAddr = 0x54DDD8;

// stage timer (use float)
const uint32_t g_stageTimerSecAddr = 0x54DDC4;
constexpr float g_centralCityHeroTimeLimit = 180.0f;


//StageAction Goal action destructor
void StageActionGoalActionMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
    auto menuModeAddr = X86_LOAD_U32(g_menuModeAddr);
    auto currentLevel = X86_LOAD_U32(g_currentStageIDAddr);
  
    // if we are in expert mode
    if (menuModeAddr == 6)
    {
        if (currentLevel == 700) // if level is last way
        {
            AchievementManager::Unlock(Ach_ExpertMode);
            AchievementManager::SaveBinary();
        }
    }
    else if (menuModeAddr == 1) // are we in (last) story mode 
    {
        if (currentLevel == 710) // if level is Devil Doom
        {
            AchievementManager::Unlock(Ach_LastStory);
            AchievementManager::SaveBinary();
        }
    }
}

// Ending achievement check
// the route table at 0x4EB160 contains the boss node of each ending (10 entries). 
#if 0 // HEROES-TODO: Shadow address(es) sub_2378B0 not matched in Heroes yet (docs/shadow-matches.md)
X86_FUNC_IMPL(__imp__sub_2378B0);
X86_OVERRIDE_FUNC(sub_2378B0)
{
    __imp__sub_2378B0(ctx, base);

    const uint32_t missionType = X86_LOAD_U32(0x54DDF4);


    // central city hero in under 3 min
    if (X86_LOAD_U32(g_currentStageIDAddr) == 400 && missionType == 2 //is hero mission
        && X86_LOAD_F32(g_stageTimerSecAddr) < g_centralCityHeroTimeLimit)
    {
        AchievementManager::Unlock(Ach_CentralCityHeroTime);
        AchievementManager::SaveBinary();
    }

    if (X86_LOAD_U32(g_menuModeAddr) != 1)
        return;

    const uint32_t node = X86_LOAD_U32(0x54DDDC);

    for (uint32_t ending = 0; ending < 10; ending++)
    {
        if (X86_LOAD_U32(0x4EB160 + (60 + ending) * 4) != node)
            continue;

        // Both neutral endings share node 0x27,
        // so we split them with the mission picked (0 dark, 2 hero).  
        if (node == 0x27)
        {
            if (missionType == 1) // nani
                return;

            ending = missionType == 0 ? 4 : 5;
        }

        AchievementManager::Unlock(ending + 1);
        AchievementManager::SaveBinary();
        return;
    }
}
#endif // HEROES-TODO


// Memo: State / Behavior 27 is DriveVehicle
bool IsAnyPlayerDriving()
{
    auto base = g_memory.base;

    for (uint32_t i = 0; i < 4; i++)
    {
        auto player = g_playerStructAddrs[i];
        if (!player)
            continue;

        auto executer = X86_LOAD_U32(player + 0x22C);
        if (!executer)
            continue;

        auto behavior = X86_LOAD_U32(executer + 0x30);
        if (behavior && X86_LOAD_U32(behavior + 0x10) == 27)
            return true;
    }

    return false;
}

// Lethal Highway tank (BKTank::SetBKTank) Exec sub_1D0C40 calls this
// with 0 when HP drops to 50 and with 1 once when HP hits 0
#if 0 // HEROES-TODO: Shadow address(es) sub_1D0900 not matched in Heroes yet (docs/shadow-matches.md)
X86_FUNC_IMPL(__imp__sub_1D0900);
X86_OVERRIDE_FUNC(sub_1D0900)
{
    const bool isDestroyed = X86_LOAD_U32(ctx.esp.u32 + 4) == 1;
    __imp__sub_1D0900(ctx, base);

    if (isDestroyed && X86_LOAD_U32(g_currentStageIDAddr) == 202 && IsAnyPlayerDriving())
    {
        AchievementManager::Unlock(Ach_TankVehicle);
        AchievementManager::SaveBinary();
    }
}
#endif // HEROES-TODO
