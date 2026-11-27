#include "shadow_rpc.h"

#include <condition_variable>
#include <ctime>
#include <thread>

#include <discord_rpc.h>

#include "shadow/shadow.h"
#include "shadow/utilities/image_name_dictionary.h"

namespace RichPresence::ShadowRpc
{
    // It's not like you could just extract this using a decompiler. Obfuscation? Screw that!
    static constexpr const char* ClientId = "1570232378503336058";

    static constexpr auto TickInterval = std::chrono::seconds(5);

    static std::mutex s_mutex;
    static std::condition_variable s_stopSignal;
    static std::thread s_thread;
    static bool s_stopRequested = false;

    static void Tick()
    {
        DiscordRichPresence richPresence{};

        std::string details;
        std::string state;

        // Menus
        if (Shadow::CurrentMode() == Modes::Menu)
        {
            details = Shadow::GetMenuStateName();
            if (details.empty())
                details = "In Menu";
        }

        // Library events
        else if (Shadow::GetModeName().empty())
        {
            details = "Watching Cutscene";
        }

        // Cutscenes
        else if (Shadow::IsWatchingCutscene())
        {
            details = "Watching Cutscene";
        }

        // Loading
        else if (Shadow::GetStageName().empty())
        {
            details = "Loading…";
        }

        // Normal Game
        else
        {
            details = Shadow::GetStageName();

            if (Shadow::IsGameplayFrozen())
                state = "Paused";
            else if (Shadow::CurrentMode() == Modes::TwoPlayer)
                state = "Duking it Out!";
            else if (Shadow::IsBossStage())
                state = "Boss Fight!";
            else if (Shadow::CurrentMode() == Modes::ExpertMode)
                state = Shadow::GetModeName();
            else
                state = Shadow::GetCurrentMissionName();
        }

        // Set Timestamp.
        if (Shadow::GetMenuState() == MenuState::InGame)
        {
            if (Shadow::IsPlayingStage())
            {
                auto stageStartTime = std::time(nullptr) - (int64_t)Shadow::StageTimer();
                richPresence.startTimestamp = stageStartTime;
            }

            richPresence.largeImageKey = ImageNameDictionary::TryGetValue(Shadow::StageId());
        }

        richPresence.details = details.empty() ? nullptr : details.c_str();
        richPresence.state = state.empty() ? nullptr : state.c_str();
        Discord_UpdatePresence(&richPresence);
    }

    static void Run()
    {
        Discord_Initialize(ClientId, nullptr, 0, nullptr);
        std::unique_lock lock(s_mutex);

        while (!s_stopRequested)
        {
            lock.unlock();
            Tick();
            Discord_RunCallbacks();
            lock.lock();

            s_stopSignal.wait_for(lock, TickInterval, [] { return s_stopRequested; });
        }

        Discord_Shutdown();
    }

    void Start()
    {
        if (s_thread.joinable())
            return;

        s_stopRequested = false;
        s_thread = std::thread(Run);
    }

    void Stop()
    {
        if (!s_thread.joinable())
            return;

        {
            std::lock_guard lock(s_mutex);
            s_stopRequested = true;
        }

        s_stopSignal.notify_all();
        s_thread.join();
    }
}
