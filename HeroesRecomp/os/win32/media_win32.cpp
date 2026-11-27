#include <os/media.h>
#include <os/logger.h>
#include <os/version.h>
#include <user/config.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

using namespace winrt;
using namespace winrt::Windows::Media::Control;

static constexpr auto POLL_INTERVAL = std::chrono::milliseconds(250);

static std::atomic<bool> g_isPlaying;
static std::once_flag g_pollThreadOnce;

static bool QueryIsPlaying(GlobalSystemMediaTransportControlsSessionManager& sessionManager)
{
    try
    {
        if (!sessionManager)
            sessionManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();

        auto session = sessionManager.GetCurrentSession();

        if (!session)
            return false;

        return session.GetPlaybackInfo().PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
    }
    catch (...)
    {
        static bool s_loggedError;

        if (!s_loggedError)
        {
            LOGF_ERROR("Failed to query GSMTC playback status: 0x{:X}", to_hresult().value);
            s_loggedError = true;
        }

        sessionManager = nullptr;
        return false;
    }
}

static void PollThread()
{
    try
    {
        init_apartment(apartment_type::multi_threaded);
    }
    catch (...)
    {
        LOGF_ERROR("Failed to init WinRT apartment: 0x{:X}", to_hresult().value);
        return;
    }

    GlobalSystemMediaTransportControlsSessionManager sessionManager = nullptr;

    while (true)
    {
        g_isPlaying = Config::MusicAttenuation && QueryIsPlaying(sessionManager);
        std::this_thread::sleep_for(POLL_INTERVAL);
    }
}

bool os::media::IsSupported()
{
    static bool s_isSupported = []
        {
            auto version = os::version::GetOSVersion();
            return version.Major > 10 || (version.Major == 10 && version.Build >= 17763);
        }();

    return s_isSupported;
}

bool os::media::IsExternalMediaPlaying()
{
    if (!IsSupported())
        return false;

    std::call_once(g_pollThreadOnce, []
        {
            std::thread(PollThread).detach();
        });

    return g_isPlaying;
}