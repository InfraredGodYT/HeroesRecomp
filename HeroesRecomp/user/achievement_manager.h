#pragma once

#include <user/achievement_data.h>
#include <gpu/video.h>

enum class EAchBinStatus
{
    Success,
    IOError,
    BadFileSize,
    BadSignature,
    BadVersion,
    BadChecksum
};

class AchievementManager
{
public:
    static inline AchievementData Data{};
    static inline EAchBinStatus BinStatus{ EAchBinStatus::Success };

    static std::filesystem::path GetDataPath(bool checkForMods)
    {
        return GetSavePath(checkForMods) / SHR_FILENAME;
    }

    static time_t GetTimestamp(uint16_t id);
    static size_t GetTotalRecords();
    static bool IsUnlocked(uint16_t id);
    static void Unlock(uint16_t id);
    static void UnlockAll();
    static void Reset();
    static bool LoadBinary();
    static bool SaveBinary(bool ignoreStatus = false);
#ifdef _DEBUG
    static void UnlockTest();
#endif
};

std::vector<Achievement> GetAchievements(ELanguage language);
void InitAchievementTextures();
Achievement GetAchievement(ELanguage language, const uint8_t id);