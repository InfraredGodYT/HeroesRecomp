#pragma once

#include <cstdint>

namespace RichPresence
{
    enum class Modes : int32_t
    {
        Menu            = -1,
        SelectMode      = 0,
        StoryMode       = 1,
        TwoPlayer       = 4,
        ExpertMode      = 6
    };
}
