
class GameMenuCommon
{
public:
    static inline uint32_t g_nextModeAddr = 0;
    static inline uint32_t g_currentModeAddr = 0;
    static inline bool g_OptionsOpen = false;
    static inline bool g_LegacyOptions = false;
    static void ResetCurrentMode();
    static void ResetNextMode();
};