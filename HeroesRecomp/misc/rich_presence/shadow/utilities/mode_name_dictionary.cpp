#include "mode_name_dictionary.h"

namespace RichPresence::ModeNameDictionary
{
    const char* TryGetValue(Modes mode)
    {
        switch (mode)
        {
            case Modes::Menu:       return "In-Menu";
            case Modes::ExpertMode: return "Expert Mode";
            case Modes::StoryMode:  return "Story Mode";
            case Modes::TwoPlayer:  return "Two Player";
            case Modes::SelectMode: return "Select Mode";
        }

        return nullptr;
    }
}
