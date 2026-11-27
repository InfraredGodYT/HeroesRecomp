#pragma once

#include <user/config.h>

enum ELedAura
{
    LED_AURA_NEUTRAL = 0, 
    LED_AURA_HERO,
    LED_AURA_VILLAIN
};

class App
{
public:
    static inline bool s_isInit;
    static inline bool s_isLoading;
    static inline bool s_isSaving;
	static inline std::atomic<ELedAura> s_chaosMode = LED_AURA_NEUTRAL;

    static inline ELanguage s_language;
    static inline EVoiceLanguage s_voiceLanguage;
    static inline double s_deltaTime;
    static inline double s_time = 0.0; // How much time elapsed since the game started.
    static inline int32_t s_gamemode = 0;

    static void Restart(std::vector<std::string> restartArgs = {});
    static void Exit();
    static bool isInMainMenu();
};
