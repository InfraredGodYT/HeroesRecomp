#pragma once

#include <cstddef>
#include <cstdint>

#include "current_mission.h"

namespace RichPresence
{
    struct MissionManager
    {
        uint8_t pad[0x20];
        CurrentMission mission;
    };

    static_assert(offsetof(MissionManager, mission) == 0x20);
}
