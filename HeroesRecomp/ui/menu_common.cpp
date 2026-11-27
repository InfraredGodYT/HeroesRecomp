#include <ui/menu_common.h>
#include <kernel/function.h>

void GameMenuCommon::ResetCurrentMode()
{
	g_OptionsOpen = false;
	if (g_currentModeAddr)
	{
		auto base = g_memory.base;
		X86_STORE_U32(g_currentModeAddr, 5);
		g_currentModeAddr = 0;
	}
}

void GameMenuCommon::ResetNextMode()
{
	if (g_nextModeAddr)
	{
		auto base = g_memory.base;
		X86_STORE_U32(g_nextModeAddr, 0);
		g_nextModeAddr = 0;
	}
}