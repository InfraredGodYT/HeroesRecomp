#pragma once

#include <cstddef>
#include <cstdint>

#include "stage_id.h"

namespace RichPresence
{
    // One row of the game's stage table.
    struct StageEntry
    {
        StageID stageId;
        uint8_t pad[0x1C];

        // -1 when the stage doesn't have that mission (e.g. Lava Shelter has no Neutral).
        struct
        {
            int32_t id;
            uint8_t pad[0x8];
        } missions[3];

        uint8_t pad2[0xC];
    };

    static_assert(offsetof(StageEntry, missions) == 0x20);
    static_assert(sizeof(StageEntry) == 0x50);
}
