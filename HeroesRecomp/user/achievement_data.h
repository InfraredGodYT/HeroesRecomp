#pragma once

#include <user/paths.h>
#include <locale/achievement_locale.h>

#define SHR_FILENAME  "ShadowAchievementData.bin"
#define SHR_SIGNATURE { 'S', 'H', 'R', ' ' }
#define SHR_VERSION   1

#define SHR_RECORDS   17

struct Achievement
{
    uint16_t ID;
    std::string Name;
    std::string UnlockedDesc;
    std::string LockedDesc;
    const uint8_t* pImageBuffer;
    uint16_t Score;
};

class AchievementData
{
public:
#pragma pack(push, 1)
    struct SHRRecord
    {
        uint16_t ID;
        time_t Timestamp;
        uint16_t Reserved[3];
    };
#pragma pack(pop)

    char Signature[4] SHR_SIGNATURE;
    uint32_t Version{ SHR_VERSION };
    uint32_t Checksum{};
    SHRRecord Records[SHR_RECORDS]{};

    bool VerifySignature() const;
    bool VerifyVersion() const;
    bool VerifyChecksum();
    uint32_t CalculateChecksum();
};

