#pragma once

enum class OptionsMenuState
{
    Intro, // pause only, background zooms in
    Opening,
    Idle,
    Closing,
    Outro, // pause only, background zooms out
    Restarting
};

enum class OptionsMenuCategory
{
    System,
    Input,
    Audio,
    Video,
    //Misc,
    Debug,
    Count
};

enum class OptionsMenuFlowState
{
    CategoryCursor,
    OptionCursor,
    OptionSelected
};


class OptionsMenu
{
public:
    static inline OptionsMenuState s_state{};
    static inline OptionsMenuFlowState s_flowState{};
    static inline bool s_isVisible = false;
    static inline bool s_isPause = false;
    static inline bool s_isRestartRequired = false;
    static inline bool s_isDebugUnlocked = false;
    static inline bool s_GoToLegacyMenu = false;
    static inline uint32_t s_padIndex = 0;


    static void Init();
    static void Draw(X86Context* ctx);
    static void Open(bool isPause = false, uint32_t padIndex = 0);
    static void Close();

    static bool IsRestartRequired();
    static void SetFlowState(OptionsMenuFlowState flowState);

    static bool CanClose();

    static void UpdateSoundTest(X86Context& ctx);
};

void DrawAnimatedFire(ImVec2 min, ImVec2 max, float fade = 1.0f);
void DrawLoopLines(ImVec2 min, ImVec2 max, float fade = 1.0f);
void DrawDescription(std::string s, const double scrollDelay = 0.5);