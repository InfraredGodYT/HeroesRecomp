
enum class AchievementMenuState
{
    Intro, 
    Achievements,
    Outro 
};


class AchievementMenu
{
public:
    static inline AchievementMenuState s_state{};
    static inline bool s_isVisible = false;
    static inline uint32_t s_padIndex = 0;

    static void Init();
    static void Draw(X86Context* ctx);
    static void Open(uint32_t padIndex = 0);
    static void Close();
    static void SetState(AchievementMenuState state);
};
