#include "app.h"
#include <gpu/video.h>
#include <kernel/function.h>
#include <ui/game_window.h>
#include <os/process.h>
#include <user/config.h>
#include <audio/audio.h>
#include <volk.h>
#include <user/paths.h>
#include <user/registry.h>
#include <audio/driver/sdl3_driver.h>
#include <cpu/guest_thread.h>
#include <gpu/frame_pacing/timer_resolution.h>
#include <input/input.h>
#include <patches/multiplayer_mod_patches.h>
#include <misc/rich_presence/shadow_rpc.h>

static std::thread::id g_mainThreadId = std::this_thread::get_id();

static void CloseAndSaveResources()
{
#if 0 // HEROES-TODO: Shadow's Discord presence
	RichPresence::ShadowRpc::Stop();
#endif // HEROES-TODO
	Video::Shutdown();
	AudioShutDown();
	Config::Save();
	TimerResolution::Release();
	if (m_TLS)
	{
		delete(m_TLS);
		m_TLS = nullptr;
	}
}

void App::Restart(std::vector<std::string> restartArgs)
{
	CloseAndSaveResources();
	os::process::StartProcess(os::process::GetExecutablePath(), restartArgs, os::process::GetWorkingDirectory());
	std::_Exit(0);
}

void App::Exit()
{
	CloseAndSaveResources();
	std::_Exit(0);
}

bool App::isInMainMenu() // 7 is library
{
	return s_gamemode == -1 || s_gamemode == 7;
}

bool Window_OnSDLEvent(void*, SDL_Event* event);

//main game loop
#if 0 // HEROES-TODO: Shadow address(es) sub_57630 not matched in Heroes yet (docs/shadow-matches.md)
X86_FUNC_IMPL(__imp__sub_57630);
X86_OVERRIDE_FUNC(sub_57630)
{
	double targetFps = GetTargetFPS();
	if (targetFps >= FPS_MIN && targetFps < GetUnlockedFPS())
	{
		double targetDeltaTime = 1.0 / targetFps;

		auto frameTime_HHC = X86_LOAD_F32(0x541FA8);
		if (abs(frameTime_HHC - targetDeltaTime) < 0.00001)
			X86_STORE_F32(0x541FA8, targetDeltaTime);
	}

	App::s_deltaTime = X86_LOAD_F32(0x541FA8);
	App::s_time += App::s_deltaTime;
	App::s_gamemode = X86_LOAD_U32(0x5022A4);
	Video::BeginGameThreadProfiler();
	MultiplayerPatchesUpdate(ctx, base);
	__imp__sub_57630(ctx, base);
	Video::EndGameThreadProfiler();
	//std::this_thread::sleep_for(std::chrono::milliseconds(16));

}
#endif // HEROES-TODO

//init
#if 0 // HEROES-TODO: Shadow address(es) sub_11AD0 not matched in Heroes yet (docs/shadow-matches.md)
X86_FUNC_IMPL(__imp__sub_11AD0);
X86_OVERRIDE_FUNC(sub_11AD0)
{
	App::s_isInit = true;
	App::s_language = Config::Language;
	App::s_voiceLanguage = Config::VoiceLanguage;

	Registry::Save();

	__imp__sub_11AD0(ctx, base);
}
#endif // HEROES-TODO

//Set Voice Language
#if 0 // HEROES-TODO: Shadow address(es) sub_55BE0 not matched in Heroes yet (docs/shadow-matches.md)
X86_FUNC_IMPL(__imp__sub_55BE0);
X86_OVERRIDE_FUNC(sub_55BE0)
{
	__imp__sub_55BE0(ctx, base);

	X86_STORE_U32(0x54DD8C, (uint32_t)Config::VoiceLanguage.Value);
}
#endif // HEROES-TODO

std::atomic<bool> shouldQuit = false; 

// We need to call SDL_PumpEvents and SDL_FlushEvents in the Main Thread of our program in a function that run every frames.
// If we don't, the game window will act unresponsive, preventing resizing, closing, etc to work.
// unfortunately Shadow immediately create a new thread in the entry point function and move on
// So we do a mid asm hook in the entry point function and put our loop there instead
void UpdateSDLEventsMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	SDL_Event event;
	while (!shouldQuit)
	{
		// fix unresponsive window
		if (std::this_thread::get_id() == g_mainThreadId)
		{
			MainThreadDispatcher::Get().Flush(); // This runs the callbacks needed from user's config
			SDL_PumpEvents();
			SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
			GameWindow::Update();
			input::Update();
		}

		SDL_WaitEventTimeout(&event, 8);
	}
}
