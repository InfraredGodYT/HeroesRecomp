#pragma once

#include <optional>
#include <string>

#include <misc/rich_presence/shadow/definitions/current_mission.h>
#include <misc/rich_presence/shadow/definitions/menu_state.h>
#include <misc/rich_presence/shadow/definitions/modes.h>
#include <misc/rich_presence/shadow/definitions/stage_id.h>

namespace RichPresence
{
    // Utility methods for reading Shadow The Hedgehog's state from guest memory.
    class Shadow
    {
    public:
        /* Pointers */
        static StageID StageId();
        static std::optional<MenuState> GetMenuState();
        static Modes CurrentMode();
        static CurrentMission GetCurrentMission();
        static bool IsWatchingCutscene();
        static bool IsGameplayFrozen();
        static float StageTimer();

        /*
            ---------------
            Utility Methods
            ---------------
        */

        static bool IsPlayingStage();
        static bool IsBossStage();
        static std::string GetStageName();
        static std::string GetMenuStateName();
        static std::string GetModeName();
        static std::string GetCurrentMissionName();
    };
}
