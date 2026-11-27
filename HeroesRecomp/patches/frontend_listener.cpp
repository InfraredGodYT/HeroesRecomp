#include <kernel/memory.h>
#include <ui/options_menu.h>
#include <os/logger.h>
#include <user/config.h>
#include <sdl_listener.h>

void ToggleHud(bool toggle);

static class FrontendListener : public SDLEventListener
{
    bool m_isF8KeyDown = false;

public:
    bool OnSDLEvent(SDL_Event* event) override
    {
        if (OptionsMenu::s_isVisible)
            return false;

        switch (event->type)
        {
        case SDL_EVENT_KEY_DOWN:
        {
            if (event->key.key != SDLK_F8 || m_isF8KeyDown)
                break;

            auto base = g_memory.base;
            bool enabled = X86_LOAD_U8(0x54DE78) != 0;
            ToggleHud(enabled == false);
        
            m_isF8KeyDown = true;

            break;
        }

        case SDL_EVENT_KEY_UP:
            if (event->key.key == SDLK_F8)
                m_isF8KeyDown = false;
            break;
        }

        return false;
    }
}
g_frontendListener;
