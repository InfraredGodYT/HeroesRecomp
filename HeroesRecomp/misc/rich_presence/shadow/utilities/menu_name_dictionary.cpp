#include "menu_name_dictionary.h"

namespace RichPresence::MenuNameDictionary
{
    const char* TryGetValue(MenuState menuState)
    {
        switch (menuState)
        {
            case MenuState::InGame:          return "In-Game";
            case MenuState::MainMenu:        return "Main Menu";
            case MenuState::Options:         return "Adjusting Options";

            case MenuState::StoryAndLibrary: return "Story";
            case MenuState::LibraryMusic:    return "Chillin' in the Library";
            case MenuState::StoryRecap:      return "Listening to Shadow Monologue";

            case MenuState::SelectMode:      return "Picking a Stage!";
            case MenuState::TwoPlayer:       return "Setting up 2P with a Buddy!";

            case MenuState::OpeningMovie:    return "Watching Cutscene";
            case MenuState::BootMessage:     return "Booting Up";
            case MenuState::Logos:           return "Looking at Cool Logos";
        }

        return nullptr;
    }
}
