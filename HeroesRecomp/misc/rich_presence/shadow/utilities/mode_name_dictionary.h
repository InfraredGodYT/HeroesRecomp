#pragma once

#include <misc/rich_presence/shadow/definitions/modes.h>

namespace RichPresence::ModeNameDictionary
{
    // Translates a Shadow The Hedgehog stage id to stage name.
    const char* TryGetValue(Modes mode);
}
