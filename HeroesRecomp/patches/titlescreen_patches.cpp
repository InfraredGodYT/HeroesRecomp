#include <kernel/function.h>
#include <input/input.h>
#include <ui/fader.h>
#include <app.h>
#include <ui/message_window.h>
#include <input/padstate.h>
#include <ui/options_menu.h>
#include <ui/menu_common.h>
#include <install/update_checker.h>

static std::atomic<bool> g_faderBegun = false;

bool g_quitMessageOpen = false;
static int g_quitMessageResult = -1;
static float g_timerAllowedToQuit = 0.f;

// The B button is locked for a bit when we reach the title screen
// to avoid accidentally open the quit prompt by mashing.
constexpr float QUIT_FIRST_DELAY = 4.f;
constexpr float QUIT_REOPEN_DELAY = 0.25f; // reduce the delay after a first cancel.

static bool g_updateAvailableMessageShown = false;
static bool g_updateAvailableMessageOpen = false;
static int g_updateAvailableMessageResult = -1;


static bool ProcessQuitMessage()
{
	if (!g_quitMessageOpen)
		return false;

	std::array<std::string, 2> options = { Localise("Common_Yes"), Localise("Common_No") };

	if (MessageWindow::Open(Localise("Title_Message_Quit"), &g_quitMessageResult, options, 0) == MSG_CLOSED)
	{
		g_timerAllowedToQuit = QUIT_FIRST_DELAY - QUIT_REOPEN_DELAY;
		if (!g_quitMessageResult) // if user asked to quit, fade out and quit the game
		{
			Fader::FadeOut(1, []() { App::Exit(); });
			g_faderBegun = true;
		}

		g_quitMessageOpen = false;
		g_quitMessageResult = -1;
	}

	return true;
}

static bool ProcessUpdateAvailableMessage()
{
	if (!g_updateAvailableMessageShown && !g_quitMessageOpen && !g_faderBegun && UpdateChecker::check() == UpdateChecker::Result::UpdateAvailable)
	{
		g_updateAvailableMessageShown = true;
		g_updateAvailableMessageOpen = true;
	}

	if (!g_updateAvailableMessageOpen)
	{
		return false;
	}

	std::array<std::string, 2> options = { Localise("Common_Yes"), Localise("Common_No") };
	if (MessageWindow::Open(Localise("Title_Message_UpdateAvailable"), &g_updateAvailableMessageResult, options) == MSG_CLOSED)
	{
		if (!g_updateAvailableMessageResult)
		{
			Fader::FadeOut(1, []()
			{
				UpdateChecker::visitWebsite();
				App::Exit();
			});
			g_faderBegun = true;
		}

		g_timerAllowedToQuit = 0;
		g_updateAvailableMessageOpen = false;
		g_updateAvailableMessageResult = -1;
	}
	return true;
}

// This function controls the title screen, it checks if it should reset the title screen or move to main menu depending on player action.
X86_FUNC_IMPL(__imp__sub_292CF0);
X86_OVERRIDE_FUNC(sub_292CF0)
{
	auto menu = ctx.ecx.u32;

	// ensure the msg box has finished closing
	if (!MessageWindow::s_isVisible)
		g_timerAllowedToQuit += App::s_deltaTime;

	if (g_timerAllowedToQuit >= QUIT_FIRST_DELAY)
	{
		if (PJSPeripheral::IsJustPressed(0, PJSPeripheral::BTN_B) && !g_faderBegun && !g_updateAvailableMessageOpen)
		{
			g_quitMessageOpen = true;
		}
	}

	if (ProcessUpdateAvailableMessage() || ProcessQuitMessage())
	{
		ctx.esp.u32 += 4;
	}
	else
	{
		__imp__sub_292CF0(ctx, base);
	}
}

// This function controls the timer logic during title screen and menu
// to prevent the game from trying to reset the title screen while we have the message open we block
// the function until player close the message box
X86_FUNC_IMPL(__imp__sub_17ED0);
X86_OVERRIDE_FUNC(sub_17ED0)
{
	if (g_quitMessageOpen || !g_quitMessageResult || g_updateAvailableMessageOpen)
	{
		ctx.eax.u8_lo = 0;
		ctx.AddEsp(1);
	}
	else
	{
		__imp__sub_17ED0(ctx, base);
	}

}

// Options logic
X86_FUNC_IMPL(__imp__sub_28F720);
X86_OVERRIDE_FUNC(sub_28F720)
{
	
	// for the custom reimplementation of the sound test menu in custom settings
	OptionsMenu::UpdateSoundTest(ctx); 

	GameMenuCommon::g_currentModeAddr = X86_LOAD_U32(ctx.ecx.u32 + 4);
	auto menuMode = X86_LOAD_U32(GameMenuCommon::g_currentModeAddr);
	auto subMode = X86_LOAD_U32(GameMenuCommon::g_currentModeAddr + 0x4);

	if (menuMode == 0) // if user requested options access, init our custom option menu
	{
		OptionsMenu::Open(false);
		GameMenuCommon::g_OptionsOpen = true;
		__imp__sub_28F720(ctx, base);
	}
	else if (GameMenuCommon::g_OptionsOpen == false) // we are no longer in options mode, let original code logic run
	{
		__imp__sub_28F720(ctx, base);
	}
	else
	{
		ctx.AddEsp(1); // prevent original function to run while our custom option menu run
	}
}

// Options rendering stubbed
X86_FUNC_IMPL(__imp__sub_28F200);
X86_OVERRIDE_FUNC(sub_28F200)
{
	if (GameMenuCommon::g_LegacyOptions)
	{
		__imp__sub_28F200(ctx, base);
	}
	else
	{
		ctx.AddEsp(1);
	}
}

// get next mode to force it to 0 when we quit options 
X86_FUNC_IMPL(__imp__sub_28E770);
X86_OVERRIDE_FUNC(sub_28E770)
{
	GameMenuCommon::g_nextModeAddr = ctx.ecx.u32 + 0x20;

	__imp__sub_28E770(ctx, base);
}
