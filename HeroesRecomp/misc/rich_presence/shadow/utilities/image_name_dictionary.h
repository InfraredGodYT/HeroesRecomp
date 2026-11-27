#pragma once

#include <misc/rich_presence/shadow/definitions/stage_id.h>

namespace RichPresence::ImageNameDictionary
{
    // Translates a Shadow The Hedgehog stage id to stage image.
    const char* TryGetValue(StageID stageId);
}
