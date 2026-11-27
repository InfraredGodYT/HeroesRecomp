#pragma once

#include <misc/rich_presence/shadow/definitions/menu_state.h>

namespace RichPresence::MenuNameDictionary
{
    // Translates a Shadow The Hedgehog stage id to stage name.
    const char* TryGetValue(MenuState menuState);
}
