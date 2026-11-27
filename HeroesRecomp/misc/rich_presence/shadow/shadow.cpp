#include "shadow.h"

#include <user/config.h>
#include <patches/fps_patches.h>
#include <patches/guest_variable.h>

#include "definitions/menu_manager.h"
#include "definitions/mission_manager.h"
#include "definitions/stage_entry.h"
#include "utilities/menu_name_dictionary.h"
#include "utilities/mode_name_dictionary.h"
#include "utilities/stage_name_dictionary.h"

namespace RichPresence
{
    /* Pointers */
    static const GuestVariable<StageID>                  g_stageId{ 0x54DDD8 };          // GC: 0x805EF958
    static const xpointer<xpointer<MenuManager>>         g_menuManager{ 0x54BA10u };     // GC: 0x80583ACC
    static const GuestVariable<Modes>                    g_currentMode{ 0x5022A4 };      // GC: 0x805EC170
    static const xpointer<xpointer<MissionManager>>      g_missionManager{ 0x541FA4u };  // GC: 0x80575F1F
    static const GuestVariable<uint8_t>                  g_isGameplayFrozen{ 0x5415A0 }; // GC: 0x8057D768
    static const GuestVariable<float>                    g_stageTimer{ 0x54DDC4 };       // GC: 0x8057D734
    static const GuestVariable<uint32_t>                 g_movieManager{ 0x541808 };

    static const xpointer<StageEntry> g_stageTable{ 0x462E00u };
    static constexpr size_t StageTableCount = 59;

    static bool HasMission(StageID stageId, CurrentMission mission)
    {
        for (size_t i = 0; i < StageTableCount; i++)
        {
            const StageEntry& entry = g_stageTable.get()[i];
            if (entry.stageId == stageId)
                return entry.missions[(size_t)mission].id != -1;
        }

        return false;
    }

    StageID Shadow::StageId()
    {
        return g_stageId;
    }

    std::optional<MenuState> Shadow::GetMenuState()
    {
        // The menu manager is created on first use, so it can be missing early on boot.
        xpointer<MenuManager> menuManager = *g_menuManager;
        if (!menuManager)
            return std::nullopt;

        return menuManager->state;
    }

    Modes Shadow::CurrentMode()
    {
        return g_currentMode;
    }

    CurrentMission Shadow::GetCurrentMission()
    {
        xpointer<MissionManager> missionManager = *g_missionManager;
        if (!missionManager)
            return CurrentMission::Neutral;

        return missionManager->mission;
    }

    // GC: 0x8057FA85.
    bool Shadow::IsWatchingCutscene()
    {
        return IsEventPlaying() || g_movieManager != 0;
    }

    bool Shadow::IsGameplayFrozen()
    {
        return g_isGameplayFrozen != 0;
    }

    float Shadow::StageTimer()
    {
        return g_stageTimer;
    }

    /*
        ---------------
        Utility Methods
        ---------------
    */

    bool Shadow::IsPlayingStage()
    {
        if (GetMenuState() != MenuState::InGame || IsWatchingCutscene() || IsGameplayFrozen())
            return false;

        if (CurrentMode() == Modes::Menu || !ModeNameDictionary::TryGetValue(CurrentMode()))
            return false;

        return StageNameDictionary::TryGetValue(StageId()) != nullptr;
    }

    bool Shadow::IsBossStage()
    {
        int32_t stage = (int32_t)StageId() % 100;
        return StageId() >= StageID::LethalHighwayBlackBull
            && StageId() <= StageID::DevilDoom
            && stage >= 10 && stage <= 18;
    }

    std::string Shadow::GetStageName()
    {
        if (auto stageName = StageNameDictionary::TryGetValue(StageId()))
            return stageName;

        return "";
    }

    std::string Shadow::GetMenuStateName()
    {
        if (auto value = GetMenuState())
            if (auto menuState = MenuNameDictionary::TryGetValue(*value))
                return menuState;

        return "";
    }

    std::string Shadow::GetModeName()
    {
        if (auto modeState = ModeNameDictionary::TryGetValue(CurrentMode()))
            return modeState;

        return "";
    }

    std::string Shadow::GetCurrentMissionName()
    {
        auto mission = GetCurrentMission();
        if (mission > CurrentMission::Hero || !HasMission(StageId(), mission))
            return "";

        switch (mission)
        {
            case CurrentMission::Dark:    return "Dark Mission";
            case CurrentMission::Neutral: return "Neutral Mission";
            case CurrentMission::Hero:    return "Hero Mission";
        }

        return "";
    }
}
