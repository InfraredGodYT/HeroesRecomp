#pragma once

#include <misc/rich_presence/shadow/definitions/stage_id.h>

namespace RichPresence::StageNameDictionary
{
    // Translates a Shadow The Hedgehog stage id to stage name.
    const char* TryGetValue(StageID stageId);
}
