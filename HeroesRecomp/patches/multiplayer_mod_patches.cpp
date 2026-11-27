#include "global.h"
#include "multiplayer_mod_patches.h"
#include <kernel/function.h>
#include <user/config.h>
#include <input/padstate.h>
#include <patches/patch_utils.h>


// Extended Multiplayer
// 2P, 3P, or 4P Shadow the Hedgehog

uint32_t g_multiplayerPlayerCount = 2;
void MultiplayerPatchesInit()
{
	g_multiplayerPlayerCount = std::clamp(Config::ExtendedMultiplayerPlayerCount.Value, 2u, 4u);
}

static uint32_t GetPlayerHandleByNumber(uint32_t number)
{
	auto base = g_memory.base;
	for (uint32_t entry = X86_LOAD_U32(0x544EB0 + 0x4); entry < X86_LOAD_U32(0x544EB0 + 0x8); entry += 0x8)
	{
		const uint32_t holder = X86_LOAD_U32(entry);
		const uint32_t player = holder ? X86_LOAD_U32(holder) : 0;
		if (player && X86_LOAD_U32(player + 0x54) == number)
		{
			return holder;
		}
	}
	return 0;
}

static uint32_t GetPlayerByNumber(uint32_t number)
{
	auto base = g_memory.base;
	const uint32_t holder = GetPlayerHandleByNumber(number);
	return holder ? X86_LOAD_U32(holder) : 0;
}

static bool IsMultiplayerPlayer(uint32_t player)
{
	auto base = g_memory.base;
	if (!player)
	{
		return false;
	}
	const uint32_t number = X86_LOAD_U32(player + 0x54);
	return number < g_multiplayerPlayerCount && GetPlayerByNumber(number) == player;
}

bool IsExtraPlayerMode()
{
	return Config::ExtendedMultiplayer && g_multiplayerPlayerCount > 2;
}

static float GetDistanceSq(uint32_t a, uint32_t b)
{
	auto base = g_memory.base;
	const float dx = X86_LOAD_F32(a) - X86_LOAD_F32(b);
	const float dy = X86_LOAD_F32(a + 0x4) - X86_LOAD_F32(b + 0x4);
	const float dz = X86_LOAD_F32(a + 0x8) - X86_LOAD_F32(b + 0x8);
	return dx * dx + dy * dy + dz * dz;
}

static void CopyVec3(uint32_t dst, uint32_t src)
{
	memcpy(g_memory.base + dst, g_memory.base + src, 0xC);
}

static uint32_t GetClosestPlayer(uint32_t pos)
{
	uint32_t closest = 0;
	float closestDistSq = FLT_MAX;
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		if (!player)
		{
			continue;
		}
		const float distSq = GetDistanceSq(player + 0x4, pos);
		if (distSq < closestDistSq)
		{
			closest = player;
			closestDistSq = distSq;
		}
	}
	return closest;
}

static void SetClosestPlayerId(X86Register& reg, uint32_t pos)
{
	auto base = g_memory.base;
	const uint32_t player = Config::ExtendedMultiplayer ? GetClosestPlayer(pos) : 0;
	if (player)
	{
		reg.u32 = X86_LOAD_U32(player + 0x298);
	}
}

// CameraManager @ 0x537770
static uint32_t GetCameraCount()
{
	auto base = g_memory.base;
	const uint32_t cameras = X86_LOAD_U32(0x537770);
	return cameras ? (X86_LOAD_U32(cameras + 0xC) - X86_LOAD_U32(cameras + 0x8)) / 0x8 : 0;
}

static uint32_t GetCameraPos(const X86Context& ctx, uint32_t index)
{
	auto base = g_memory.base;
	const uint32_t cameras = X86_LOAD_U32(0x537770);
	return cameras ? GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_124F0, cameras, index) : 0;
}

// Chaos Powers Enabled for MP Fix (Xbox exclusive patch where its disabled)
X86_FUNC_IMPL(__imp__sub_22D250);
X86_OVERRIDE_FUNC(sub_22D250)
{
	const bool awake = isAwakeAboutToInvoke;
	isAwakeAboutToInvoke = false;
	if (Config::ExtendedMultiplayer && !awake)
	{
		ctx.esp.u32 += 8;
		return;
	}
	__imp__sub_22D250(ctx, base);
}

X86_FUNC_IMPL(__imp__sub_22D260);
X86_OVERRIDE_FUNC(sub_22D260)
{
	const bool awake = isAwakeAboutToRevoke;
	isAwakeAboutToRevoke = false;
	if (Config::ExtendedMultiplayer && !awake)
	{
		ctx.esp.u32 += 8;
		return;
	}
	__imp__sub_22D260(ctx, base);
}
// End Chaos Powers Enabled for MP Fix

// ChaosPowers Sync
// pBehavior bitfield @ player_struct + 0xAC
static uint32_t CountActiveAwakePlayers(uint8_t awakeBit)
{
	auto base = g_memory.base;
	uint32_t count = 0;
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		if (!g_playerStructAddrs[i])
		{
			continue;
		}
		uint8_t stateByte = X86_LOAD_U8(g_playerStructAddrs[i] + 0xAC);
		if (stateByte & awakeBit)
		{
			count++;
		}
	}
	return count;
}

// Awake Idle drain reduction (per player)
uint32_t GetAwakeDrainDivisor(uint32_t awake)
{
	if (!Config::ExtendedMultiplayer)
	{
		return 1;
	}
	auto base = g_memory.base;
	uint32_t mode = X86_LOAD_U32(awake + 0x40);   // 0 = Dark, 1 = Hero
	uint8_t awakeBit = (mode == 1) ? 0x04 : 0x08;    // HeroShadow / DarkShadow
	return std::max(CountActiveAwakePlayers(awakeBit), 1u);
}

void AwakeDarkThresholdBypassMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}

	int32_t darkThreshold = (int32_t)X86_LOAD_U32(ctx.eax.u32 + 0x71C);
	if ((int32_t)ctx.edx.u32 >= darkThreshold)
	{
		return;
	}

	// handle vehicle case or egg dealer
	if (CountActiveAwakePlayers(0x08) > 0)
	{
		ctx.edx.u32 = darkThreshold;
	}
}

void AwakeHeroThresholdBypassMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}

	int32_t heroThreshold = (int32_t)X86_LOAD_U32(ctx.eax.u32 + 0x710);
	if ((int32_t)ctx.ecx.u32 >= heroThreshold)
	{
		return;
	}

	if (CountActiveAwakePlayers(0x04) > 0)
	{
		ctx.ecx.u32 = heroThreshold;
	}
}
// End ChaosPowerSync

bool ExtendedMultiplayerCameraAttachMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		return true;
	}
	auto val = X86_LOAD_U8(ctx.eax.u32 + 8);
	return (val & 4) != 0;
}

bool ExtendedMultiplayerPlayerSpawnMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		return true;
	}
	auto val = X86_LOAD_U8(ctx.eax.u32 + 8);
	return (val & 4) != 0;
}

bool ExtendedMultiplayerCameraAllocationLoopMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	uint32_t limit = Config::ExtendedMultiplayer ? g_multiplayerPlayerCount : 2;
	return ctx.edi.u32 < limit;
}

void ExtendedMultiplayerCharIdMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayerP1CharId >= 0)
	{
		ctx.eax.u32 = Config::ExtendedMultiplayerP1CharId;
	}
	if (Config::ExtendedMultiplayerP2CharId >= 0)
	{
		ctx.esi.u32 = Config::ExtendedMultiplayerP2CharId;
	}
}

static bool s_extraSpawnActive = false;
static bool s_extraSpawnDone = false;
static uint32_t s_extraSpawnRemaining = 0;
static uint32_t s_extraSpawnPlayerIndex = 2;

bool ExtendedMultiplayerExtraSpawnMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (s_extraSpawnDone)
	{
		s_extraSpawnDone = false;
		s_extraSpawnActive = false;
		ctx.esp.u32 += 3 * (g_multiplayerPlayerCount - 2) * 4;
		return false;
	}
	if (!Config::ExtendedMultiplayer || g_multiplayerPlayerCount <= 2)
	{
		return false;
	}
	if (!s_extraSpawnActive)
	{
		s_extraSpawnActive = true;
		s_extraSpawnRemaining = g_multiplayerPlayerCount - 2;
		s_extraSpawnPlayerIndex = 2;
	}
	ctx.esp.u32 -= 4;
	X86_STORE_U32(ctx.esp.u32, s_extraSpawnPlayerIndex);
	ctx.esp.u32 -= 4;
	X86_STORE_U32(ctx.esp.u32, s_extraSpawnPlayerIndex);
	ctx.esp.u32 -= 4;
	int32_t charId = s_extraSpawnPlayerIndex == 2 ? Config::ExtendedMultiplayerP3CharId : Config::ExtendedMultiplayerP4CharId;
	X86_STORE_U32(ctx.esp.u32, charId >= 0 ? charId : 14 + s_extraSpawnPlayerIndex);
	s_extraSpawnRemaining--;
	s_extraSpawnPlayerIndex++;
	return true;
}

bool ExtendedMultiplayerExtraSpawnRedirectMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (s_extraSpawnActive)
	{
		if (s_extraSpawnRemaining == 0)
		{
			s_extraSpawnDone = true;
		}
		return true;
	}
	return false;
}

static uint32_t s_extraPlayerStartPos = 0;

// special cases for aligning P3 and P4 differently
static constexpr uint32_t EXTRA_START_SIDE_STAGES[] =
{
	201,
	301,
	401,
	501,
	504,
};
static constexpr float EXTRA_START_SIDE[3] = { 0.0f, 0.0f, 20.0f };
static constexpr float EXTRA_START_HEIGHT = 30.0f;

// nukkoro spacing assumed from P1 -> P2 difference (in the future maybe we extend the nukkoro2 parser... but screw that for now)
static uint32_t GetExtraPlayerStartPos(uint32_t id, uint32_t pos)
{
	auto base = g_memory.base;
	const uint32_t gameState = X86_LOAD_U32(0x54963C);

	// stored checkpoint check
	if (id < 2 || !gameState || X86_LOAD_U32(gameState) != 0)
	{
		return pos;
	}
	if (!s_extraPlayerStartPos)
	{
		s_extraPlayerStartPos = g_memory.MapVirtual(g_userHeap.Alloc(0xC));
	}
	const uint32_t p1 = gameState + 0xC;
	const uint32_t p2 = p1 + 0x20 * 0x1C;
	const uint32_t stage = X86_LOAD_U32(0x54DDD8);
	if (std::find(std::begin(EXTRA_START_SIDE_STAGES), std::end(EXTRA_START_SIDE_STAGES), stage) != std::end(EXTRA_START_SIDE_STAGES))
	{
		const uint32_t from = id == 2 ? p1 : pos;
		for (uint32_t i = 0; i < 3; i++)
		{
			X86_STORE_F32(s_extraPlayerStartPos + i * 0x4, X86_LOAD_F32(from + i * 0x4) + EXTRA_START_SIDE[i]);
		}
		return s_extraPlayerStartPos;
	}

	float step[3];
	float lengthSq = 0.0f;
	for (uint32_t i = 0; i < 3; i++)
	{
		step[i] = X86_LOAD_F32(p2 + i * 0x4) - X86_LOAD_F32(p1 + i * 0x4);
		lengthSq += step[i] * step[i];
	}

	// stack above case
	if (lengthSq < 1.0f || lengthSq > 200.0f * 200.0f)
	{
		step[0] = 0.0f;
		step[1] = EXTRA_START_HEIGHT;
		step[2] = 0.0f;
	}
	const float scale = float(id - 1);
	for (uint32_t i = 0; i < 3; i++)
	{
		X86_STORE_F32(s_extraPlayerStartPos + i * 0x4, X86_LOAD_F32(pos + i * 0x4) + step[i] * scale);
	}
	return s_extraPlayerStartPos;
}

static uint32_t s_extraPlayerPositionID = 0;

void ExtendedMultiplayerPlayerPositionIDOverrideMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && s_extraPlayerPositionID != 0)
	{
		ctx.edx.u32 = s_extraPlayerPositionID;
		X86_STORE_U32(ctx.esp.u32, GetExtraPlayerStartPos(s_extraPlayerPositionID, X86_LOAD_U32(ctx.esp.u32)));
	}
}

bool ExtendedMultiplayerPlayerPositionLoopMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return false;
	}

	if (s_extraPlayerPositionID == 0)
	{
		s_extraPlayerPositionID = 2;
	}
	else
	{
		s_extraPlayerPositionID++;
	}

	if (s_extraPlayerPositionID <= g_multiplayerPlayerCount - 1)
	{
		return true;
	}

	s_extraPlayerPositionID = 0;
	return false;
}

static uint32_t s_extraPlayerPositionID_Continue0 = 0;
void ExtendedMultiplayerPlayerPositionIDOverrideContinue0MidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && s_extraPlayerPositionID_Continue0 != 0)
	{
		ctx.edx.u32 = s_extraPlayerPositionID_Continue0;
		X86_STORE_U32(ctx.esp.u32, GetExtraPlayerStartPos(s_extraPlayerPositionID_Continue0, X86_LOAD_U32(ctx.esp.u32)));
	}
}

// Checkpoint Table @ 0x544FB8
// Save Data @ 0x545780 (overflow read on P3/P4, so force to P2)
bool ExtendedMultiplayerPlayerPositionLoopContinue0MidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || g_multiplayerPlayerCount <= 2)
	{
		return false;
	}

	if (s_extraPlayerPositionID_Continue0 == 0)
	{
		s_extraPlayerPositionID_Continue0 = 2;
	}
	else
	{
		s_extraPlayerPositionID_Continue0++;
	}

	if (s_extraPlayerPositionID_Continue0 <= g_multiplayerPlayerCount - 1)
	{
		ctx.esp.u32 += 12;
		ctx.eax.u32 = X86_LOAD_U32(0x4EE658); // force read P2 data
		return true;
	}
	s_extraPlayerPositionID_Continue0 = 0;
	return false;
}

static uint32_t s_assignPadPlayerId = 0;
static bool s_playerPadAssign = false;

// Player ctor (sub_97340)
// Marking so we know its a player when PAD construction (sub_91AD0) runs
void PlayerPADAssignBeginMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	s_playerPadAssign = true;
}

// Cleanup hook from above
void PlayerPADAssignEndMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	s_playerPadAssign = false;
}

// Partner PAD assignment (we would extend this also if we later want to do multi partner separate controllers)
// Using the last unused next PAD/PORT
// If playing 4P partners are AI controlled only
void PartnerPADMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && !s_playerPadAssign)
	{
		ctx.eax.u32 = g_multiplayerPlayerCount < 4 ? g_multiplayerPlayerCount : 4;
	}
}

void AssignPadPlayerIdSaveMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	s_assignPadPlayerId = ctx.ecx.u32;
}

void AssignPadPlayerIdOverrideMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && s_playerPadAssign)
	{
		ctx.edx.u32 = s_assignPadPlayerId;
	}
}

bool PADBoundsCheckMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && s_playerPadAssign && g_multiplayerPlayerCount >= 4 && ctx.eax.u32 >= 4)
	{
		return false;
	}
	return true;
}

static bool ViewportDiv(X86Register& reg)
{
	if (Config::ExtendedMultiplayer && g_multiplayerPlayerCount > 2)
	{
		reg.u32 = (uint32_t)((int32_t)reg.u32 / (int32_t)g_multiplayerPlayerCount);
		return true;
	}
	return false;
}

bool ViewportDivEaxMidAsmHook(X86Context& __restrict ctx, uint8_t* base) { return ViewportDiv(ctx.eax); }
bool ViewportDivEcxMidAsmHook(X86Context& __restrict ctx, uint8_t* base) { return ViewportDiv(ctx.ecx); }

void ViewportExtendedPlayerCalcOverrideMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || g_multiplayerPlayerCount <= 2)
	{
		return;
	}

	uint32_t screenWidth = X86_LOAD_U32(0x4E5D88);
	uint32_t screenHeight = X86_LOAD_U32(0x4E5D8C);
	uint32_t dimA = X86_LOAD_U32(0x541A5C);
	uint32_t dimB = X86_LOAD_U32(0x541A60);

	// P3 / P4 views get user ids 2 / 3 for world UI
	X86_STORE_U32(0x4E5E08, 2);
	X86_STORE_U32(0x4E5E38, 3);

	if (g_multiplayerPlayerCount == 4)
	{
		uint32_t w = screenWidth / 2;
		uint32_t h = screenHeight / 2;
		uint32_t dimAdiv = dimA / 2;
		uint32_t dimBdiv = dimB / 2;

		X86_STORE_U32(0x4E5DB0, 0);
		X86_STORE_U32(0x4E5DB4, 0);
		X86_STORE_U32(0x4E5DB8, w);
		X86_STORE_U32(0x4E5DBC, h);
		X86_STORE_U32(0x4E5DC0, 0);
		X86_STORE_U32(0x4E5DC4, 0);
		X86_STORE_U32(0x4E5DC8, dimAdiv);
		X86_STORE_U32(0x4E5DCC, dimBdiv);

		X86_STORE_U32(0x4E5DE0, w);
		X86_STORE_U32(0x4E5DE4, 0);
		X86_STORE_U32(0x4E5DE8, w);
		X86_STORE_U32(0x4E5DEC, h);
		X86_STORE_U32(0x4E5DF0, dimAdiv); // 2D rect origins (item icons only)
		X86_STORE_U32(0x4E5DF4, 0);
		X86_STORE_U32(0x4E5DF8, dimAdiv);
		X86_STORE_U32(0x4E5DFC, dimBdiv);

		X86_STORE_U32(0x4E5E10, 0);
		X86_STORE_U32(0x4E5E14, h);
		X86_STORE_U32(0x4E5E18, w);
		X86_STORE_U32(0x4E5E1C, h);
		X86_STORE_U32(0x4E5E20, 0);
		X86_STORE_U32(0x4E5E24, dimBdiv);
		X86_STORE_U32(0x4E5E28, dimAdiv);
		X86_STORE_U32(0x4E5E2C, dimBdiv);

		X86_STORE_U32(0x4E5E40, w);
		X86_STORE_U32(0x4E5E44, h);
		X86_STORE_U32(0x4E5E48, w);
		X86_STORE_U32(0x4E5E4C, h);
		X86_STORE_U32(0x4E5E50, dimAdiv);
		X86_STORE_U32(0x4E5E54, dimBdiv);
		X86_STORE_U32(0x4E5E58, dimAdiv);
		X86_STORE_U32(0x4E5E5C, dimBdiv);

		// FOV
		X86_STORE_F32(0x4E5DD0, 0.75f); // P1
		X86_STORE_F32(0x4E5DD4, 0.75f);
		X86_STORE_F32(0x4E5E00, 0.75f); // P2
		X86_STORE_F32(0x4E5E04, 0.75f);
		X86_STORE_F32(0x4E5E30, 0.75f); // P3
		X86_STORE_F32(0x4E5E34, 0.75f);
		X86_STORE_F32(0x4E5E60, 0.75f); // P4
		X86_STORE_F32(0x4E5E64, 0.75f);
	}
	else
	{
		uint32_t w = screenWidth / g_multiplayerPlayerCount;
		uint32_t x = 2 * w;
		uint32_t dimAdiv = dimA / g_multiplayerPlayerCount;

		X86_STORE_U32(0x4E5E10, x);
		X86_STORE_U32(0x4E5E18, screenWidth - x);
		X86_STORE_U32(0x4E5E1C, screenHeight);
		X86_STORE_U32(0x4E5E20, 2 * dimAdiv); // P3 2D x
		X86_STORE_U32(0x4E5E24, 0);
		X86_STORE_U32(0x4E5E28, dimA - 2 * dimAdiv); // P3 2D width (gets the excess pixels since / 3)
		X86_STORE_U32(0x4E5E2C, dimB);
		X86_STORE_U32(0x4E5E44, screenHeight);
		X86_STORE_U32(0x4E5E48, w);
		X86_STORE_U32(0x4E5E4C, screenHeight);
		X86_STORE_U32(0x4E5E54, dimB);
		X86_STORE_U32(0x4E5E58, dimAdiv);
		X86_STORE_U32(0x4E5E5C, dimB);

		// FOV
		X86_STORE_F32(0x4E5E30, 0.5f); // P3
		X86_STORE_F32(0x4E5E34, 1.0f);
		X86_STORE_F32(0x4E5E60, 0.5f); // P4
		X86_STORE_F32(0x4E5E64, 1.0f);
	}
}

float GetMultiplayerViewScale()
{
	if (!IsExtraPlayerMode())
	{
		return 1.0f;
	}
	auto base = g_memory.base;
	const uint32_t type = X86_LOAD_U32(0x541A50);
	if (type == 0 || type > 4)
	{
		return 1.0f;
	}
	const uint32_t screen = 0x4E5D78 + type * 0x30;
	const int32_t viewHeight = X86_LOAD_U32(screen + 0x14);
	const int32_t screenHeight = X86_LOAD_U32(0x4E5D8C);
	const float scaleY = X86_LOAD_F32(screen + 0x2C) * 1.25f;
	const float fullScaleY = X86_LOAD_F32(0x4E5DA4);
	if (viewHeight <= 0 || screenHeight <= 0 || scaleY <= 0.0f || fullScaleY <= 0.0f)
	{
		return 1.0f;
	}
	return std::min(float(viewHeight) / float(screenHeight) * fullScaleY / scaleY, 1.0f);
}

void HPBarViewScaleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const float scale = GetMultiplayerViewScale();
	X86_STORE_F32(ctx.esi.u32 + 0x38, X86_LOAD_F32(ctx.esi.u32 + 0x38) * scale);
	X86_STORE_F32(ctx.esi.u32 + 0x3C, X86_LOAD_F32(ctx.esi.u32 + 0x3C) * scale);
}

// fix scaling for CSD per viewport
void WorldCsdViewScaleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_F32(ctx.esp.u32 + 0x28, X86_LOAD_F32(ctx.esp.u32 + 0x28) * GetMultiplayerViewScale());
}

static uint32_t s_renderDivExtraPass = 0;
static uint32_t s_renderDivSavedThis = 0;

void RenderDivSaveThisMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	s_renderDivSavedThis = ctx.edi.u32;
}

bool RenderDivExtraPassMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && g_multiplayerPlayerCount > 2)
	{
		if (s_renderDivExtraPass < g_multiplayerPlayerCount - 2)
		{
			s_renderDivExtraPass++;
			ctx.edi.u32 = s_renderDivSavedThis;
			return true;
		}
		s_renderDivExtraPass = 0;
	}
	return false;
}

bool RenderDivCameraLookupMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (s_renderDivExtraPass > 0)
	{
		ctx.eax.u32 = s_renderDivExtraPass + 1;
		return true;
	}
	return false;
}

bool RenderDivPlayerOverrideMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && g_multiplayerPlayerCount > 2 && s_renderDivExtraPass > 0)
	{
		ctx.esp.u32 -= 4;
		X86_STORE_U32(ctx.esp.u32, s_renderDivExtraPass + 2);
		return true;
	}
	return false;
}

bool RenderDivCameraTargetOverrideMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && g_multiplayerPlayerCount > 2 && s_renderDivExtraPass > 0)
	{
		X86_STORE_U32(0x54DD9C, s_renderDivExtraPass + 2);
		return true;
	}
	return false;
}

static uint32_t s_cameraAttachPlayerIndex = 0;
bool CameraAttachSetupMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		X86_STORE_U32(ctx.ebp.u32, g_multiplayerPlayerCount);
		if (s_cameraAttachPlayerIndex == 0)
		{
			s_cameraAttachPlayerIndex = 1;
		}
		return true;
	}
	return false;
}

bool CameraAttachPlayerIdMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return false;
	}
	ctx.esp.u32 -= 4;
	X86_STORE_U32(ctx.esp.u32, s_cameraAttachPlayerIndex);
	s_cameraAttachPlayerIndex++;
	return true;
}

bool CameraAttachLoopMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return false;
	}
	if (s_cameraAttachPlayerIndex <= g_multiplayerPlayerCount - 1)
	{
		return true;
	}
	s_cameraAttachPlayerIndex = 0;
	return false;
}

bool PlayerPostManagerLoopMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		return ctx.eax.u32 < g_multiplayerPlayerCount;
	}
	return ctx.eax.u32 < 2;
}

// ItemIconManager fix
bool ItemIconManagerLoopPushCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.esp.u32 -= 4;
		X86_STORE_U32(ctx.esp.u32, g_multiplayerPlayerCount);
		return true;
	}
	return false;
}

bool ItemIconManagerLoopCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		return ctx.eax.u32 < g_multiplayerPlayerCount;
	}
	return ctx.eax.u32 < 2;
}

// ItemIcon fix for 3P/4P
bool ItemIconCreatePlayerBoundMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return ctx.ebx.i32 >= int32_t(Config::ExtendedMultiplayer ? g_multiplayerPlayerCount : 2);
}

// needs to rescale for 3P/4P otherwise we push toward edge
bool AdjustMultiplayerItemIcon(const X86Context& ctx)
{
	if (!IsExtraPlayerMode())
	{
		return false;
	}
	auto base = g_memory.base;
	const uint32_t entry = X86_LOAD_U32(ctx.edi.u32);
	const uint32_t player = entry ? X86_LOAD_U32(entry) : UINT32_MAX;
	if (player >= 4)
	{
		return false;
	}
	const uint32_t rect = 0x4E5D90 + (player + 1) * 0x30;
	const float rectX = float(int32_t(X86_LOAD_U32(rect)));
	const float rectY = float(int32_t(X86_LOAD_U32(rect + 0x4)));
	const float rectW = float(int32_t(X86_LOAD_U32(rect + 0x8)));
	const float rectH = float(int32_t(X86_LOAD_U32(rect + 0xC)));
	const float fullH = float(int32_t(X86_LOAD_U32(0x4E5D9C)));
	if (rectH <= 0.0f || fullH <= 0.0f || rectH >= fullH)
	{
		return true;
	}
	const float scale = rectH / fullH;
	const float centreX = rectX + rectW * 0.5f;
	const float oldCentreY = rectY + 372.0f;
	const float newCentreY = rectY + 372.0f * scale;
	for (uint32_t i = 0; i < 4; i++)
	{
		const uint32_t vertex = 0x559688 + i * 0x1C;
		X86_STORE_F32(vertex, centreX + (X86_LOAD_F32(vertex) - centreX) * scale);
		X86_STORE_F32(vertex + 0x4, newCentreY + (X86_LOAD_F32(vertex + 0x4) - oldCentreY) * scale);
	}
	return true;
}

// handle on init and unpause
bool SplitscreenTypeInitializeUnpauseMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		return true;
	}
	return (ctx.eax.u8_lo & 0x2) != 0;
}

void AddAllScoreToP1MidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		uint32_t gameStateBase = X86_LOAD_U32(0x54963C);
		if (gameStateBase != 0)
		{
			ctx.ecx.u32 = gameStateBase + 0x754;
		}
	}
}

// Object Touch Cooldown (extended, previously thought this was the effect slot but nope)
static constexpr uint32_t TOUCH_COOLDOWN_EMPTY = 0xFFFFFFFF;
static uint32_t GetTouchCooldownSlot(uint32_t table, uint32_t i)
{
	return table + (i < 2 ? i * 0x8 : 0x14 + (i - 2) * 0x8);
}

bool ObjectTouchCooldownAllocMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return false;
	}
	ctx.esp.u32 -= 4;
	X86_STORE_U32(ctx.esp.u32, 0x24);
	return true;
}

void ObjectTouchCooldownInitMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	for (uint32_t i = 0; i < 4; i++)
	{
		const uint32_t slot = GetTouchCooldownSlot(ctx.eax.u32, i);
		X86_STORE_F32(slot, 0.0f);
		X86_STORE_U32(slot + 0x4, TOUCH_COOLDOWN_EMPTY);
	}
}

bool ObjectTouchCooldownRegisterMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t table = ctx.esi.u32;
	const uint32_t id = ctx.edx.u32;
	const uint32_t count = Config::ExtendedMultiplayer ? 4 : 2;
	for (uint32_t i = 0; i < count; i++)
	{
		if (X86_LOAD_U32(GetTouchCooldownSlot(table, i) + 0x4) == id)
		{
			return false;
		}
	}
	for (uint32_t i = 0; i < count; i++)
	{
		const uint32_t slot = GetTouchCooldownSlot(table, i);
		if (X86_LOAD_F32(slot) <= 0.0f)
		{
			X86_STORE_F32(slot, X86_LOAD_F32(table + 0x10));
			X86_STORE_U32(slot + 0x4, id);
			return true;
		}
	}
	return false;
}

bool ObjectTouchCooldownTickMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return false;
	}
	const uint32_t table = X86_LOAD_U32(ctx.ecx.u32);
	const float dt = X86_LOAD_F32(ctx.esp.u32 + 0x4);
	for (uint32_t i = 0; i < 4; i++)
	{
		const uint32_t slot = GetTouchCooldownSlot(table, i);
		const float timer = X86_LOAD_F32(slot);
		if (timer > 0.0f)
		{
			const float left = timer - dt;
			if (left <= 0.0f)
			{
				X86_STORE_F32(slot, 0.0f);
				X86_STORE_U32(slot + 0x4, TOUCH_COOLDOWN_EMPTY);
			}
			else
			{
				X86_STORE_F32(slot, left);
			}
		}
	}
	return true;
}

// Start Layout Distance Check Patches
bool SetObjLoadMultiplayerBranchMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		return false;
	}
	return ctx.eax.u8_lo == 0;
}

bool SetObjUnloadMultiplayerBranchMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		return false;
	}
	return X86_LOAD_U8(0x54DDE2) == 0;
}

// Object <-> Camera distance check (inside SetObjLoad)
X86_FUNC_IMPL(__imp__sub_2595A0);
X86_OVERRIDE_FUNC(sub_2595A0)
{
	const uint32_t objEntryAddr = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const uint32_t subObjAddr = X86_LOAD_U32(objEntryAddr);

	__imp__sub_2595A0(ctx, base);

	if (Config::ExtendedMultiplayer && g_multiplayerPlayerCount > 2 && ctx.eax.u8_lo == 0 && subObjAddr)
	{
		const uint8_t thresholdByte = X86_LOAD_U8(subObjAddr + 0x23);
		const float thresholdSq = (float)(thresholdByte * thresholdByte) * X86_LOAD_F32(0x448F28);
		const uint32_t count = std::min(GetCameraCount(), g_multiplayerPlayerCount);
		for (uint32_t i = 2; i < count; i++)
		{
			const uint32_t cameraPos = GetCameraPos(ctx, i);
			if (cameraPos && GetDistanceSq(cameraPos, subObjAddr) < thresholdSq)
			{
				ctx.eax.u8_lo = 1;
				return;
			}
		}
	}
}

// Object <-> Camera distance check (inside SetObjUnload)
X86_FUNC_IMPL(__imp__sub_181F80);
X86_OVERRIDE_FUNC(sub_181F80)
{
	const uint32_t objPosAddr = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const float thresholdSq = X86_LOAD_F32(ctx.esp.u32 + 0x8);

	__imp__sub_181F80(ctx, base);

	if (Config::ExtendedMultiplayer && g_multiplayerPlayerCount > 2 && ctx.eax.u8_lo != 0)
	{
		const uint32_t count = std::min(GetCameraCount(), g_multiplayerPlayerCount);
		for (uint32_t i = 2; i < count; i++)
		{
			const uint32_t cameraPos = GetCameraPos(ctx, i);
			if (cameraPos && GetDistanceSq(cameraPos, objPosAddr) < thresholdSq)
			{
				ctx.eax.u8_lo = 0;
				return;
			}
		}
	}
}
// END Layout Distance Check Patches

bool SkipCameraDemoMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Bosses Target Closest Player
static void SetBossTargetPlayer(const X86Context& ctx, X86Register& reg)
{
	auto base = g_memory.base;
	const uint32_t boss = X86_LOAD_U32(X86_LOAD_U32(ctx.esi.u32 - 0x8) + 0x1C);
	if (boss)
	{
		SetClosestPlayerId(reg, boss + 0x4);
	}
}

void BossTargetClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	SetBossTargetPlayer(ctx, ctx.ecx);
}

void BossTargetClosestPlayerEdxMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	SetBossTargetPlayer(ctx, ctx.edx);
}

static uint32_t s_contactPlayer = 0;
static std::vector<uint32_t> s_contactPlayers;
X86_FUNC_IMPL(__imp__sub_177F70);
X86_OVERRIDE_FUNC(sub_177F70)
{
	s_contactPlayer = 0;
	s_contactPlayers.clear();
	__imp__sub_177F70(ctx, base);
	if (Config::ExtendedMultiplayer && s_contactPlayer)
	{
		ctx.eax.u8_lo = 1;
	}
}

void ContactPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (ctx.eax.u8_lo == 0)
	{
		return;
	}
	const uint32_t object = X86_LOAD_U32(ctx.esp.u32 + 0x18);
	if (!s_contactPlayer)
	{
		s_contactPlayer = object;
	}
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	if (IsMultiplayerPlayer(object) && std::find(s_contactPlayers.begin(), s_contactPlayers.end(), object) == s_contactPlayers.end())
	{
		s_contactPlayers.push_back(object);
	}
	ctx.eax.u8_lo = 0;
}

// Zipline/Pulley/Coaster Fix
void CoasterClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t coaster = X86_LOAD_U32(ctx.esi.u32 + 0x38);
	if (coaster)
	{
		SetClosestPlayerId(ctx.ecx, coaster + 0x4);
	}
}

static std::unordered_map<uint32_t, uint32_t> s_coasterRiders;
static std::unordered_map<uint32_t, uint32_t> s_coasterRiderNumbers;

// rider = touching player, else closest (this may still be buggy? Needs more testing but so far so good)
void CoasterGrabMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	uint32_t player = IsMultiplayerPlayer(s_contactPlayer) ? s_contactPlayer : 0;
	const uint32_t coaster = X86_LOAD_U32(ctx.esi.u32 + 0x38);
	if (!player && coaster)
	{
		player = GetClosestPlayer(coaster + 0x4);
	}
	if (player)
	{
		ctx.edx.u32 = X86_LOAD_U32(player + 0x298);
	}
	s_coasterRiders[ctx.esi.u32] = ctx.edx.u32;
	s_coasterRiderNumbers[ctx.esi.u32] = player ? X86_LOAD_U32(player + 0x54) : 0;
}

bool CoasterCameraPushMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const auto it = s_coasterRiderNumbers.find(ctx.esi.u32);
	return Config::ExtendedMultiplayer && it != s_coasterRiderNumbers.end() && it->second != 0;
}

void CoasterRiderMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	auto it = s_coasterRiders.find(ctx.esi.u32);
	if (Config::ExtendedMultiplayer && it != s_coasterRiders.end())
	{
		ctx.ecx.u32 = it->second;
	}
}

// player gravity 0xE8
static bool HasGravity(uint32_t player, uint32_t index)
{
	auto base = g_memory.base;
	const uint32_t target = 0x4EAE40 + index * 0xC;
	const float tx = X86_LOAD_F32(target);
	const float ty = X86_LOAD_F32(target + 0x4);
	const float tz = X86_LOAD_F32(target + 0x8);
	const float px = X86_LOAD_F32(player + 0xE8);
	const float py = X86_LOAD_F32(player + 0xEC);
	const float pz = X86_LOAD_F32(player + 0xF0);
	const float dot = tx * px + ty * py + tz * pz;
	return dot > 0.0f && dot * dot > 0.999f * (tx * tx + ty * ty + tz * tz) * (px * px + py * py + pz * pz);
}

// GravitySwitcher Fix
static std::unordered_map<uint32_t, std::vector<uint32_t>> s_gravityTouchers;
static std::vector<uint32_t> s_gravityApplyPlayers;
static uint32_t s_gravityPlayer = 0;

void GravityTouchMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || ctx.eax.u8_lo == 0)
	{
		return;
	}
	const uint32_t object = ctx.esi.u32;
	const uint32_t index = X86_LOAD_U32(object + (X86_LOAD_U32(object) == 0x46C9BC ? 0x3C : 0x30));
	auto it = s_gravityTouchers.find(object);
	for (const uint32_t player : s_contactPlayers)
	{
		if (index < 6 && HasGravity(player, index))
		{
			continue;
		}
		if (it == s_gravityTouchers.end())
		{
			it = s_gravityTouchers.emplace(object, std::vector<uint32_t>{}).first;
		}
		if (std::find(it->second.begin(), it->second.end(), player) == it->second.end())
		{
			it->second.push_back(player);
		}
	}
	if (it == s_gravityTouchers.end())
	{
		ctx.eax.u8_lo = 0;
	}
}

// keep touch collision
bool GravityKeepTouchableMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// apply gravity (map players)
void GravityApplyMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	s_gravityApplyPlayers.clear();
	auto it = s_gravityTouchers.find(ctx.esi.u32);
	if (it == s_gravityTouchers.end())
	{
		return;
	}
	for (const uint32_t player : it->second)
	{
		if (IsMultiplayerPlayer(player))
		{
			s_gravityApplyPlayers.push_back(player);
		}
	}
	s_gravityTouchers.erase(it);
}

// Apply once per touch
X86_FUNC_IMPL(__imp__sub_58050);
X86_OVERRIDE_FUNC(sub_58050)
{
	if (!Config::ExtendedMultiplayer || s_gravityApplyPlayers.empty())
	{
		__imp__sub_58050(ctx, base);
		return;
	}
	const std::vector<uint32_t> players = std::exchange(s_gravityApplyPlayers, {});
	const uint32_t esp = ctx.esp.u32;
	const uint32_t index = X86_LOAD_U32(esp + 0x4);
	for (const uint32_t player : players)
	{
		ctx.esp.u32 = esp;
		X86_STORE_U32(esp + 0x4, index);
		s_gravityPlayer = player;
		__imp__sub_58050(ctx, base);
	}
}

// World gravity (signboards)
void GravityPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && s_gravityPlayer)
	{
		ctx.ecx.u32 = X86_LOAD_U32(s_gravityPlayer + 0x298);
	}
	s_gravityPlayer = 0;
}

// GravityChangeCollision Fix
void GravityCollisionCompareMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || ctx.edi.u32 >= 6)
	{
		return;
	}
	auto it = s_gravityTouchers.find(ctx.esi.u32);
	if (it == s_gravityTouchers.end())
	{
		const uint32_t player = GetPlayerByNumber(0);
		ctx.eax.u32 = player && HasGravity(player, ctx.edi.u32) ? ctx.edi.u32 : ctx.edi.u32 + 1;
		return;
	}
	auto& touchers = it->second;
	std::erase_if(touchers, [&](uint32_t player) { return !IsMultiplayerPlayer(player) || HasGravity(player, ctx.edi.u32); });
	ctx.eax.u32 = touchers.empty() ? ctx.edi.u32 : ctx.edi.u32 + 1;
	if (touchers.empty())
	{
		s_gravityTouchers.erase(it);
	}
}

bool GravityCollisionKeepMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Airbug Fix VERSION 9. WORK. DAMNIT. I PATCHED AIR STREAM, I PATCHED FAN AND WIND. WHAT ELSE DO YOU WANT FROM ME.
struct AirStreamSlots
{
	uint32_t handles[4][2] = {};
	uint8_t touching = 0;
};

static std::unordered_map<uint32_t, AirStreamSlots> s_airStreams;
static int32_t s_airStreamPlayer = -1;
static uint32_t s_airStreamLoop = 0;
static int32_t s_airStreamLoopPlayer = -1;

static bool IsAirStreamSlotUsed(const uint32_t* slot)
{
	return slot[0] || slot[1];
}

static void LoadHandle(uint32_t field, uint32_t* handle)
{
	auto base = g_memory.base;
	X86_STORE_U32(field, handle[0]);
	X86_STORE_U32(field + 0x4, handle[1]);
	handle[0] = 0;
	handle[1] = 0;
}

static void StoreHandle(uint32_t field, uint32_t* handle)
{
	auto base = g_memory.base;
	handle[0] = X86_LOAD_U32(field);
	handle[1] = X86_LOAD_U32(field + 0x4);
	X86_STORE_U32(field, 0);
	X86_STORE_U32(field + 0x4, 0);
}

static AirStreamSlots& GetAirStreamSlots(const X86Context& ctx, uint32_t field)
{
	auto base = g_memory.base;
	AirStreamSlots& slots = s_airStreams[field];
	const uint32_t ptr = X86_LOAD_U32(field);
	const uint32_t ref = X86_LOAD_U32(field + 0x4);
	if (!ptr && !ref)
	{
		return slots;
	}
	const uint32_t number = IsMultiplayerPlayer(ptr) ? X86_LOAD_U32(ptr + 0x54) : 4;
	if (number < 4 && !IsAirStreamSlotUsed(slots.handles[number]))
	{
		StoreHandle(field, slots.handles[number]);
		return slots;
	}
	X86_STORE_U32(field, 0);
	X86_STORE_U32(field + 0x4, 0);
	if (ref)
	{
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12070, ref);
	}
	return slots;
}

static int32_t GetNextAirStreamPlayer(const AirStreamSlots& slots, int32_t after, bool present)
{
	for (int32_t i = after + 1; i < int32_t(g_multiplayerPlayerCount); i++)
	{
		if (IsAirStreamSlotUsed(slots.handles[i]) || (present && GetPlayerByNumber(i)))
		{
			return i;
		}
	}
	return -1;
}

static void ReleaseAirStreamSlots(const X86Context& ctx, uint32_t field)
{
	auto it = s_airStreams.find(field);
	if (it == s_airStreams.end())
	{
		return;
	}
	for (const auto& slot : it->second.handles)
	{
		if (slot[1])
		{
			GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12070, slot[1]);
		}
	}
	s_airStreams.erase(it);
}

// nearest player = looped player? Maybe?
X86_FUNC_IMPL(__imp__sub_91F90);
X86_OVERRIDE_FUNC(sub_91F90)
{
	if (s_airStreamPlayer < 0)
	{
		__imp__sub_91F90(ctx, base);
		return;
	}
	const uint32_t out = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_91D30, ctx.ecx.u32, out, uint32_t(s_airStreamPlayer));
	ctx.eax.u32 = out;
	ctx.esp.u32 += 0xC;
}

// skip other player contacts
static bool SkipAirStreamContact(X86Context& ctx, uint8_t* base, bool players)
{
	if (s_airStreamPlayer < 0)
	{
		return false;
	}
	const uint32_t object = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const bool player = IsMultiplayerPlayer(object);
	if (player ? X86_LOAD_U32(object + 0x54) == uint32_t(s_airStreamPlayer) : !players && s_airStreamPlayer == 0)
	{
		return false;
	}
	const uint32_t ref = X86_LOAD_U32(ctx.esp.u32 + 0x8);
	if (ref)
	{
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12350, ref);
	}
	ctx.esp.u32 += 0x4;
	return true;
}

// Fan Enter
X86_FUNC_IMPL(__imp__sub_199DC0);
X86_OVERRIDE_FUNC(sub_199DC0)
{
	if (!SkipAirStreamContact(ctx, base, true))
	{
		__imp__sub_199DC0(ctx, base);
	}
}

// Fan Stream
X86_FUNC_IMPL(__imp__sub_199F30);
X86_OVERRIDE_FUNC(sub_199F30)
{
	if (!SkipAirStreamContact(ctx, base, false))
	{
		__imp__sub_199F30(ctx, base);
	}
}

static void SetFanTouching(uint32_t fan, bool touching)
{
	auto base = g_memory.base;
	X86_STORE_U32(fan + 0x98, (X86_LOAD_U32(fan + 0x98) & ~0x1u) | (touching ? 0x1 : 0x0));
}

// "in fan" once per player (holy shit FUCK this object THIS BETTER WORK THIS TIME)
X86_FUNC_IMPL(__imp__sub_19A420);
X86_OVERRIDE_FUNC(sub_19A420)
{
	const uint32_t fan = ctx.ecx.u32;
	if (!Config::ExtendedMultiplayer)
	{
		__imp__sub_19A420(ctx, base);
		return;
	}
	const uint32_t field = fan + 0x48;
	AirStreamSlots& slots = GetAirStreamSlots(ctx, field);
	const uint32_t esp = ctx.esp.u32;
	const uint32_t arg = X86_LOAD_U32(esp + 0x4);
	slots.touching = 0;
	for (int32_t i = GetNextAirStreamPlayer(slots, -1, true); i >= 0; i = GetNextAirStreamPlayer(slots, i, true))
	{
		ctx.esp.u32 = esp;
		ctx.ecx.u32 = fan;
		X86_STORE_U32(esp + 0x4, arg);
		SetFanTouching(fan, false);
		LoadHandle(field, slots.handles[i]);
		s_airStreamPlayer = i;
		__imp__sub_19A420(ctx, base);
		s_airStreamPlayer = -1;
		StoreHandle(field, slots.handles[i]);
		if (X86_LOAD_U32(fan + 0x98) & 0x1)
		{
			slots.touching |= 1 << i;
		}
	}
	SetFanTouching(fan, slots.touching);
	ctx.esp.u32 = esp + 0x8;
}

// Fan stream loop
void FanStreamBeginMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t fan = ctx.esi.u32;
	const uint32_t field = fan + 0x48;
	if (!Config::ExtendedMultiplayer || s_airStreamLoop == fan)
	{
		return;
	}
	AirStreamSlots& slots = GetAirStreamSlots(ctx, field);
	if (!(X86_LOAD_U32(fan + 0x98) & 0x1))
	{
		slots.touching = 0;
	}
	const int32_t i = GetNextAirStreamPlayer(slots, -1, false);
	if (i < 0)
	{
		return;
	}
	LoadHandle(field, slots.handles[i]);
	SetFanTouching(fan, slots.touching & (1 << i));
	s_airStreamLoop = fan;
	s_airStreamLoopPlayer = i;
}

bool FanStreamEndMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t fan = ctx.esi.u32;
	const uint32_t field = fan + 0x48;
	if (s_airStreamLoop != fan)
	{
		return false;
	}
	AirStreamSlots& slots = s_airStreams[field];
	StoreHandle(field, slots.handles[s_airStreamLoopPlayer]);
	const int32_t i = GetNextAirStreamPlayer(slots, s_airStreamLoopPlayer, false);
	if (i < 0)
	{
		SetFanTouching(fan, slots.touching);
		s_airStreamLoop = 0;
		s_airStreamLoopPlayer = -1;
		return false;
	}
	LoadHandle(field, slots.handles[i]);
	SetFanTouching(fan, slots.touching & (1 << i));
	s_airStreamLoopPlayer = i;
	return true;
}

// Fan dtor
X86_FUNC_IMPL(__imp__sub_19A180);
X86_OVERRIDE_FUNC(sub_19A180)
{
	ReleaseAirStreamSlots(ctx, ctx.ecx.u32 + 0x48);
	__imp__sub_19A180(ctx, base);
}

// ElecFan stream loop
void ElecFanStreamBeginMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t fan = ctx.esi.u32;
	if (!Config::ExtendedMultiplayer || s_airStreamLoop == fan)
	{
		return;
	}
	AirStreamSlots& slots = GetAirStreamSlots(ctx, fan + 0x38);
	const int32_t i = GetNextAirStreamPlayer(slots, -1, true);
	if (i < 0)
	{
		return;
	}
	LoadHandle(fan + 0x38, slots.handles[i]);
	s_airStreamLoop = fan;
	s_airStreamLoopPlayer = i;
}

// ElecFan capture
X86_FUNC_IMPL(__imp__sub_1FB940);
X86_OVERRIDE_FUNC(sub_1FB940)
{
	if (s_airStreamLoop == ctx.ecx.u32)
	{
		s_airStreamPlayer = s_airStreamLoopPlayer;
	}
	__imp__sub_1FB940(ctx, base);
	s_airStreamPlayer = -1;
}

// release the locked player, next loop
bool ElecFanStreamEndMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t fan = ctx.esi.u32;
	if (s_airStreamLoop != fan)
	{
		return false;
	}
	AirStreamSlots& slots = s_airStreams[fan + 0x38];
	StoreHandle(fan + 0x38, slots.handles[s_airStreamLoopPlayer]);
	const int32_t i = GetNextAirStreamPlayer(slots, s_airStreamLoopPlayer, true);
	if (i < 0)
	{
		s_airStreamLoop = 0;
		s_airStreamLoopPlayer = -1;
		return false;
	}
	const uint32_t ref = X86_LOAD_U32(ctx.esp.u32 + 0x1C);
	X86_STORE_U32(ctx.esp.u32 + 0x18, 0);
	X86_STORE_U32(ctx.esp.u32 + 0x1C, 0);
	if (ref)
	{
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12350, ref);
	}
	LoadHandle(fan + 0x38, slots.handles[i]);
	s_airStreamLoopPlayer = i;
	ctx.eax.u32 = X86_LOAD_U32(fan + 0x50);
	return true;
}

// ElecFan dtor
X86_FUNC_IMPL(__imp__sub_1FBD60);
X86_OVERRIDE_FUNC(sub_1FBD60)
{
	ReleaseAirStreamSlots(ctx, ctx.ecx.u32 + 0x38);
	__imp__sub_1FBD60(ctx, base);
}

// Fix Vehicles for MP
static uint32_t GetVehicleRider(uint32_t vehicle)
{
	auto base = g_memory.base;
	const uint32_t ref = X86_LOAD_U32(vehicle + 0x84);
	const uint32_t rider = ref && X86_LOAD_U32(ref + 0x4) ? X86_LOAD_U32(vehicle + 0x80) : 0;
	return Config::ExtendedMultiplayer && IsMultiplayerPlayer(rider) ? rider : 0;
}

static uint32_t s_mountRider = 0; // necessary because some Volts don't map camera until post intro spline alignment >_>
static std::unordered_map<uint32_t, uint32_t> s_vehicleRiders;

X86_FUNC_IMPL(__imp__sub_23F7F0);
X86_OVERRIDE_FUNC(sub_23F7F0)
{
	const uint32_t handle = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const uint32_t player = handle ? X86_LOAD_U32(handle) : 0;
	s_mountRider = Config::ExtendedMultiplayer && IsMultiplayerPlayer(player) ? player : 0;
	s_vehicleRiders[ctx.ecx.u32] = s_mountRider;
	__imp__sub_23F7F0(ctx, base);
}

// Select proper port on mount
void VehicleCarPortMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (s_mountRider)
	{
		ctx.eax.u32 = GetPlayerPort(s_mountRider);
	}
}

void VehicleMountCameraMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (s_mountRider)
	{
		X86_STORE_U32(ctx.esp.u32 + 0x8, X86_LOAD_U32(s_mountRider + 0x54));
	}
	s_mountRider = 0;
}

// Vehicles/Turrets/Flyables Camera Fix
static void SetRiderCamera(X86Context& ctx, uint8_t* base, uint32_t vehicle)
{
	auto it = s_vehicleRiders.find(vehicle);
	const uint32_t rider = it != s_vehicleRiders.end() && IsMultiplayerPlayer(it->second) ? it->second : (vehicle ? GetVehicleRider(vehicle) : 0);
	if (rider)
	{
		X86_STORE_U32(ctx.esp.u32 + 0x8, X86_LOAD_U32(rider + 0x54));
	}
}

// GunLift
void GunLiftMountCameraMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t lift = ctx.esi.u32;
	const uint32_t camera = X86_LOAD_U32(lift + 0x154);
	auto it = s_vehicleRiders.find(lift);
	const uint32_t rider = it != s_vehicleRiders.end() ? it->second : 0;
	if (!camera || !IsMultiplayerPlayer(rider))
	{
		return;
	}
	const uint32_t number = X86_LOAD_U32(rider + 0x54);
	if (X86_LOAD_U32(camera + 0xA4) == number)
	{
		return;
	}
	const uint32_t out = ctx.esp.u32 + 0x20;
	GuestToHostFunction<uint32_t, CallConvention::Cdecl>(ctx, sub_2E0E0, out, 0u, number);
	const uint32_t newCamera = X86_LOAD_U32(out);
	if (!newCamera)
	{
		return;
	}
	GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_CD080, newCamera + 0xC4, camera + 0xC4);
	const uint32_t ref = X86_LOAD_U32(lift + 0x158);
	X86_STORE_U32(lift + 0x154, newCamera);
	X86_STORE_U32(lift + 0x158, X86_LOAD_U32(out + 0x4));
	if (ref)
	{
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12350, ref);
	}
}

// BirdBase
void VehicleBirdCameraMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	SetRiderCamera(ctx, base, X86_LOAD_U32(ctx.esi.u32));
}

// Checkpoint Warping
static std::unordered_map<uint32_t, uint32_t> s_savePointUsers;
static uint32_t s_savePointMenuPort = UINT32_MAX;
static uint32_t s_savePointMenuPlayer = 0;
void SavePointUserMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t player = X86_LOAD_U32(ctx.eax.u32 + 0x14);
	if (IsMultiplayerPlayer(player))
	{
		s_savePointUsers[ctx.ecx.u32] = X86_LOAD_U32(player + 0x298);
		s_savePointMenuPort = GetPlayerPort(player);
		s_savePointMenuPlayer = player;
	}
}

// Warp Hole / Checkpoint Movement Allowed (lock player who opened it)
static uint32_t s_lockedPlayer = 0;
static uint32_t s_lockedPad = 0;

static void LockPlayerInput(uint32_t player)
{
	auto base = g_memory.base;
	const uint32_t input = player ? X86_LOAD_U32(player + 0x218) : 0;
	if (input && !s_lockedPlayer)
	{
		s_lockedPlayer = player;
		s_lockedPad = X86_LOAD_U32(input + 0x10);
		X86_STORE_U32(input + 0x10, 0);
	}
}

static void UnlockPlayerInput()
{
	auto base = g_memory.base;
	const uint32_t input = s_lockedPlayer ? X86_LOAD_U32(s_lockedPlayer + 0x218) : 0;
	if (input && X86_LOAD_U32(input + 0x10) == 0)
	{
		X86_STORE_U32(input + 0x10, s_lockedPad);
	}
	s_lockedPlayer = 0;
}

// savepoint menu open
bool SavePointNoWorldPauseMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return false;
	}
	LockPlayerInput(IsMultiplayerPlayer(s_savePointMenuPlayer) ? s_savePointMenuPlayer : 0);
	return true;
}

// savepoint menu cancel
void SavePointCancelMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	UnlockPlayerInput();
}

// warp task start (checkpoint warp and warp holes)
bool WarpNoWorldPauseMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// warp without fade
bool WarpNoFadeMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

void SavePointMenuPortMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && s_savePointMenuPort < 4)
	{
		X86_STORE_U32(ctx.esp.u32 + 0x4, s_savePointMenuPort);
	}
}

void SavePointWarpUserMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	UnlockPlayerInput();
	auto it = s_savePointUsers.find(ctx.edi.u32);
	if (Config::ExtendedMultiplayer && it != s_savePointUsers.end())
	{
		ctx.edx.u32 = it->second;
	}
}

// Castle Monster Targets Closest Player
void MonsterClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t monster = X86_LOAD_U32(ctx.esi.u32 + 0x30);
	if (monster)
	{
		SetClosestPlayerId(ctx.ecx, monster + 0x4);
	}
}

// Westopolis Lasers Fix
void CityLaserClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t laser = X86_LOAD_U32(ctx.edi.u32 + 0xC);
	if (laser)
	{
		SetClosestPlayerId(ctx.ecx, laser + 0x4);
	}
}

// GUN Fortress SecurityCamera Targets Closest Player
void WatchCameraClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t camera = X86_LOAD_U32(ctx.esi.u32 + 0x30);
	if (camera)
	{
		SetClosestPlayerId(ctx.ecx, camera + 0x4);
	}
}

// BkGunship Targets Closest Player
void GunShipClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	SetClosestPlayerId(ctx.ecx, ctx.esp.u32 + 0x18);
}

// BkWorm Targets Closest Player
void BkWormClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t worm = ctx.ebp.u32 - 0x2B0 - X86_LOAD_U32(ctx.esp.u32 + 0x10) * 0x8;
	const uint32_t vbase = worm + 0x100;
	const uint32_t object = X86_LOAD_U32(vbase + X86_LOAD_U32(X86_LOAD_U32(vbase) + 0x4));
	const uint32_t task = object ? X86_LOAD_U32(object + 0x30) : 0;
	if (task)
	{
		SetClosestPlayerId(ctx.edx, task + 0x4);
	}
}

// Keydoors Detect Fix
void SecretDoorClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t door = X86_LOAD_U32(ctx.esi.u32 + 0x30);
	const uint32_t player = Config::ExtendedMultiplayer && door ? GetClosestPlayer(door + 0x4) : 0;
	if (player)
	{
		X86_STORE_U32(ctx.esp.u32 + 0x10, player);
	}
}

// Computer Room Goal Fix
void MissionClearClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t goal = X86_LOAD_U32(ctx.esi.u32 + 0x30);
	const uint32_t player = Config::ExtendedMultiplayer && goal ? GetClosestPlayer(goal + 0x4) : 0;
	if (player)
	{
		X86_STORE_U32(ctx.esp.u32, X86_LOAD_U32(player + 0x54));
	}
}

// Helicopter Attach to Closest Player
bool HelicopterGrabAnyPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer && IsMultiplayerPlayer(ctx.eax.u32))
	{
		X86_STORE_U32(ctx.esp.u32 + 0x1C, ctx.eax.u32);
		return true;
	}
	return X86_LOAD_U32(ctx.esp.u32 + 0x1C) == ctx.eax.u32;
}

// Prevent Vehicle Despawn
bool VehicleDespawnMultiplayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	uint32_t count = X86_LOAD_U8(0x54DDE2) != 0 ? 2 : 1;
	if (Config::ExtendedMultiplayer)
	{
		count = std::min(GetCameraCount(), g_multiplayerPlayerCount);
	}
	const float radiusSq = X86_LOAD_F32(ctx.esp.u32 + 0x10);
	for (uint32_t i = 1; i < count; i++)
	{
		const uint32_t cameraPos = GetCameraPos(ctx, i);
		if (cameraPos && GetDistanceSq(cameraPos, ctx.esi.u32) < radiusSq)
		{
			return true;
		}
	}
	return false;
}

// UI for 1P Modes
bool MultiplayerHudMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer || X86_LOAD_U8(0x54DDE2) != 0;
}

// 3P / 4P HUD handled in imgui
bool MultiplayerHudCounterSkipMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return IsExtraPlayerMode();
}

// Keep Chaos Powers & Score on Player Death
bool KeepChaosPowersOnDeathMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

bool KeepScoreOnDeathMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Timer No Reset On Death
bool TimerNoResetOnDeathMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Do Not Target Other Player
bool NoTargetOtherPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Chaos Powers Glow Per Player
// own clump clone per Awake, freed in its dtor
// Thank you JetEffect shoes for showing us we can do this!
static std::unordered_map<uint32_t, uint32_t> s_awakeGlowClones;
void AwakeGlowCloneMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || ctx.eax.u32 == 0 || X86_LOAD_U32(ctx.esp.u32 + 0x2C) == 0)
	{
		return;
	}
	const uint32_t awake = ctx.esi.u32;
	const uint32_t clone = GuestToHostFunction<uint32_t, CallConvention::Cdecl>(ctx, sub_2CB460, ctx.eax.u32);
	ctx.eax.u32 = clone;
	if (clone != 0)
	{
		s_awakeGlowClones[awake] = clone;
	}
	else
	{
		s_awakeGlowClones.erase(awake);
	}
}

// Stay away from my Aura hoooome wreckkker
uint32_t PopAwakeGlowClone(uint32_t awake)
{
	auto it = s_awakeGlowClones.find(awake);
	if (it == s_awakeGlowClones.end())
	{
		return 0;
	}
	const uint32_t clone = it->second;
	s_awakeGlowClones.erase(it);
	auto base = g_memory.base;
	const uint32_t wrapper = X86_LOAD_U32(awake + 0x30);
	return (wrapper && X86_LOAD_U32(wrapper) == clone) ? clone : 0;
}

// Disable Torch SFX
// This is technically not needed with the extra slots, but eh screw it keeping this since we have 3P/4P now too
bool DisableTorchSfxMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// So long Barrier/Vacuum Effects
X86_FUNC_IMPL(__imp__sub_31C720);
X86_OVERRIDE_FUNC(sub_31C720)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.esp.u32 += 4;
		return;
	}
	__imp__sub_31C720(ctx, base);
}

// P3/P4 player blocks to fix overflows (only has P1/P2 originally, most of this is unused, but just incase)
static constexpr uint32_t GAME_STATE = 0x544FB8;
static constexpr uint32_t PLAYER_BLOCK_SIZE = 0x4C;
static uint32_t s_extraPlayerBlocks = 0;

// Kind of overkill, but incase anyone restores the cut Super Shadow mode
static bool IsSuperStage()
{
	auto base = g_memory.base;
	return X86_LOAD_U8(0x462E08 + X86_LOAD_U32(0x54DDDC) * 0x50) & 0x8;
}

static uint32_t GetPlayerBlock(uint32_t number)
{
	if (number < 2)
	{
		return GAME_STATE + 0x728 + number * PLAYER_BLOCK_SIZE;
	}
	if (!s_extraPlayerBlocks)
	{
		void* blocks = g_userHeap.Alloc(PLAYER_BLOCK_SIZE * 2);
		memset(blocks, 0, PLAYER_BLOCK_SIZE * 2);
		s_extraPlayerBlocks = g_memory.MapVirtual(blocks);
	}
	return s_extraPlayerBlocks + (number - 2) * PLAYER_BLOCK_SIZE;
}

// player+0x238 [ShadowWeapon] -> +0x34
// for 3p/4p UI to hide built-in and endless weapons
// I decided against doing id checks incase we mod the android slots or extend them later
static uint32_t GetAmmoWeaponType(uint32_t player)
{
	auto base = g_memory.base;
	const uint32_t holder = X86_LOAD_U32(player + 0x238);
	if (!holder || X86_LOAD_U32(holder) != 0x44B5A4)
	{
		return 0;
	}
	const uint32_t weapon = X86_LOAD_U32(holder + 0x34);
	if (!weapon)
	{
		return 0;
	}
	switch (X86_LOAD_U32(weapon))
	{
		case 0x4737AC: // EmbeddedGun::AutoRifle
		case 0x4737F0: // EmbeddedBazooka::Bazooka
		case 0x47413C: // ChaosSpear
			return 0;
		default:
			return X86_LOAD_U32(weapon + 0x18);
	}
}

bool GetMultiplayerHudCounters(uint32_t number, MultiplayerHudCounters& counters)
{
	if (number >= g_multiplayerPlayerCount || (number >= 2 && !s_extraPlayerBlocks))
	{
		return false;
	}
	const uint32_t player = GetPlayerByNumber(number);
	if (!player)
	{
		return false;
	}
	auto base = g_memory.base;
	const uint32_t block = GetPlayerBlock(number);
	counters.rings = X86_LOAD_U32(block + 0x28);
	counters.ammo = X86_LOAD_U32(block + 0x1C);
	counters.weaponType = GetAmmoWeaponType(player);
	return true;
}

static MultiplayerHudMission s_hudMission[2] = {};
static bool s_hudBossGauge = false;

// matching flag 0x5595E8
void NotifyMultiplayerHudBossGauge()
{
	s_hudBossGauge = true;
}

bool MultiplayerHudTimerSkipMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return IsExtraPlayerMode();
}

bool MultiplayerHudMissionMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!IsExtraPlayerMode())
	{
		return false;
	}
	if (ctx.ebx.u32 < 2)
	{
		s_hudMission[ctx.ebx.u32] = { true, int32_t(X86_LOAD_U32(ctx.esp.u32 + 0x8)), int32_t(X86_LOAD_U32(ctx.esp.u32 + 0xC)) };
	}
	ctx.esp.u32 += 0x10;
	return true;
}

static float GetHudGauge(uint32_t gauge)
{
	auto base = g_memory.base;
	const int32_t value = X86_LOAD_U32(gauge);
	const int32_t max = X86_LOAD_U32(gauge + 0x4);
	return max > 0 ? std::clamp(float(value) / float(max), 0.0f, 1.0f) : 0.0f;
}

void GetMultiplayerHudShared(MultiplayerHudShared& shared)
{
	auto base = g_memory.base;
	shared = {};
	const bool limit = X86_LOAD_F32(0x54DDCC) > 0.0f && X86_LOAD_U8(0x54DE7A) != 0;
	shared.time = X86_LOAD_F32(limit ? 0x54DDD0 : 0x54DDC4); // time limits shown (space gadget, central city, cosmic fall)
	const uint32_t gameState = X86_LOAD_U32(0x54963C);
	if (gameState)
	{
		shared.heroGauge = GetHudGauge(gameState + 0x70C);
		shared.darkGauge = GetHudGauge(gameState + 0x718);
	}
	shared.dark = s_hudMission[0];
	shared.hero = s_hudMission[1];
	s_hudMission[0].visible = false;
	s_hudMission[1].visible = false;
	shared.bossVisible = s_hudBossGauge;
	shared.bossGauge = std::clamp(float(X86_LOAD_F32(0x5595EC)), 0.0f, 1.0f);
	s_hudBossGauge = false;
}

X86_FUNC_IMPL(__imp__sub_173AF0);
X86_OVERRIDE_FUNC(sub_173AF0)
{
	const int32_t number = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	if (Config::ExtendedMultiplayer && ctx.ecx.u32 == GAME_STATE && number >= 2 && number < 4)
	{
		ctx.eax.u32 = GetPlayerBlock(number);
		ctx.esp.u32 += 0x8;
		return;
	}
	__imp__sub_173AF0(ctx, base);
}

// clear P3 / P4 blocks
X86_FUNC_IMPL(__imp__sub_173B30);
X86_OVERRIDE_FUNC(sub_173B30)
{
	const uint32_t gameState = ctx.ecx.u32;
	__imp__sub_173B30(ctx, base);
	if (Config::ExtendedMultiplayer && gameState == GAME_STATE)
	{
		for (uint32_t i = 2; i < 4; i++)
		{
			const uint32_t block = GetPlayerBlock(i);
			memset(base + block, 0, PLAYER_BLOCK_SIZE);
			X86_STORE_U32(block + 0x28, IsSuperStage() ? 50 : 0);
		}
	}
}

// P3 / P4 death reset
void PlayerBlockDeathResetMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	for (uint32_t i = 2; i < 4; i++)
	{
		const uint32_t block = GetPlayerBlock(i);
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_22D0D0, block);
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_5C6D0, block + 0x1C, 0u);
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_22D3C0, block + 0x28);
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_22D610, block + 0x2C);
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_22D6A0, block + 0x44, 0u);
	}
}

// Super for DevilDoom
static void MakeSuper(X86Context& ctx, uint32_t player)
{
	auto base = g_memory.base;
	GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_A4700, X86_LOAD_U32(player + 0x298));
}

// incase we make a super cheat later or death timing desyncs weird
static bool HasSuper(uint32_t player)
{
	auto base = g_memory.base;
	const uint32_t owner = X86_LOAD_U32(player + 0x23C);
	const uint32_t holder = owner ? X86_LOAD_U32(owner + 0x28) : 0;
	return !holder || X86_LOAD_U32(holder + 0x1C);
}

void SuperAllPlayersMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	for (uint32_t i = 1; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		if (player)
		{
			MakeSuper(ctx, player);
		}
	}
}

// DevilDoom BalloonRings
void DevilDoomSharedRingsMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const int32_t amount = X86_LOAD_U32(ctx.esp.u32);
	if (!Config::ExtendedMultiplayer || X86_LOAD_U32(0x54DDD8) != 710 || amount <= 0)
	{
		return;
	}
	const uint32_t block = ctx.ecx.u32;
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t other = GetPlayerBlock(i);
		if (other != block)
		{
			GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_22D380, other + 0x28, amount);
		}
	}
	ctx.ecx.u32 = block;
}

// DevilDoom warp distance
void DevilDoomWarpDistanceMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		const uint32_t params = ctx.esi.u32 + 0x74;
		X86_STORE_F32(params + 0x24, 20.0f);
		X86_STORE_F32(params + 0x28, 10000.0f);
	}
}

// Force 10 Ring Checkpoint Bonus (I might change this later, but for now keeping it)
bool CheckpointRingBonusMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// HintRing / ItemCapsule turn fix
void BillboardViewportCameraMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t viewport = X86_LOAD_U32(0x54DDA0);
	if (Config::ExtendedMultiplayer && viewport < g_multiplayerPlayerCount)
	{
		X86_STORE_U32(ctx.esp.u32, viewport);
	}
}

// GUN Fortress SecurityCamera skip
bool WatchCameraNoSpecialCameraMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Start fix SuperSuper model invis bug
bool SuperModelNullMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return ctx.eax.u32 == 0;
}

// Super waits for the player body (OG game race issue)
bool SuperWaitBodyMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t model = X86_LOAD_U32(X86_LOAD_U32(ctx.esi.u32 + 0x4) + 0x230);
	return model && X86_LOAD_U32(model) == 0x44ADC4 && X86_LOAD_U32(model + 0x458) == 0;
}

// skip an unloaded skeleton (OG game race issue, part of the above)
bool SuperSkeletonMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return ctx.ecx.u32 < 0x1000;
}

// handle androids
bool SuperBindNullKeyMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer && ctx.ecx.u32 == 0;
}
// End fix SuperSuper model invis bug

// Other player pause support
static uint32_t s_pausePort = UINT32_MAX;
static bool FindPausePort(uint32_t first, uint32_t buttons)
{
	for (uint32_t i = first; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		const uint32_t port = player ? GetPlayerPort(player) : UINT32_MAX;
		if (port < 4 && (GetPadSlot(port)->justPressed & buttons) != 0)
		{
			s_pausePort = port;
			return true;
		}
	}
	return false;
}

uint32_t GetMultiplayerPausePort()
{
	return Config::ExtendedMultiplayer ? s_pausePort : UINT32_MAX;
}

// pause idle (mode 1)
void PauseIdleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	s_pausePort = UINT32_MAX;
}

// any player Start
bool PauseOpenAnyPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || X86_LOAD_U8(ctx.edi.u32 + 0x4D) == 0)
	{
		return false;
	}
	return FindPausePort(1, PJSPeripheral::BTN_START);
}

// pause manager port reads
void PausePortEcxMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t port = GetMultiplayerPausePort();
	if (port < 4)
	{
		ctx.ecx.u32 = port;
	}
}

void PausePortEbpMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t port = GetMultiplayerPausePort();
	if (port < 4)
	{
		ctx.ebp.u32 = port;
	}
}

// MP Sound Listener Fix
X86_FUNC_IMPL(__imp__sub_25E860);
X86_OVERRIDE_FUNC(sub_25E860)
{
	if (!Config::ExtendedMultiplayer)
	{
		__imp__sub_25E860(ctx, base);
		return;
	}
	const uint32_t pos = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const uint32_t player = GetClosestPlayer(pos);
	double dist = 9999999.0;
	if (player)
	{
		const double dx = X86_LOAD_F32(player + 0x4) - X86_LOAD_F32(pos);
		const double dy = X86_LOAD_F32(player + 0x8) - X86_LOAD_F32(pos + 0x4);
		const double dz = X86_LOAD_F32(player + 0xC) - X86_LOAD_F32(pos + 0x8);
		dist = std::sqrt(dx * dx + dy * dy + dz * dz);
	}
	ctx.fpu.push((float)dist);
	ctx.esp.u32 += 0x4;
}

void SoundMultiplayerFlagEcxMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.ecx.u8_lo = 1;
	}
}

void SoundMultiplayerFlagEaxMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.eax.u8_lo = 1;
	}
}

// centered pan mode
bool SoundPanMultiplayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer || X86_LOAD_U8(0x54DDE2) != 0;
}

void SoundDopplerClosestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	SetClosestPlayerId(ctx.ecx, ctx.edi.u32 + 0x1C);
}

// Independent Respawns
static constexpr uint32_t BEHAVIOR_DEAD = 0x1F;
static bool s_respawnPending[4] = {};
static bool s_respawnReady[4] = {};
static float s_respawnTimers[4] = {};

static uint32_t GetPlayerBehaviorId(uint32_t player)
{
	auto base = g_memory.base;
	const uint32_t executer = X86_LOAD_U32(player + 0x22C);
	const uint32_t behavior = executer ? X86_LOAD_U32(executer + 0x30) : 0;
	return behavior ? X86_LOAD_U32(behavior + 0x10) : UINT32_MAX;
}

static bool IsStagePlaying()
{
	auto base = g_memory.base;
	const uint32_t manager = X86_LOAD_U32(0x5496E4);
	return manager && X86_LOAD_U32(manager + 0x8) == 3 && X86_LOAD_U32(manager + 0xC) == 0;
}

// Player::Behavior::Dead::Exec (handling respawns)
bool IndependentRespawnDeadMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t player = X86_LOAD_U32(ctx.edi.u32 + 0x4);
	const uint32_t number = X86_LOAD_U32(player + 0x54);
	// devil doom is excluded
	if (!Config::ExtendedMultiplayer || number >= 4 || X86_LOAD_U32(0x54DDD8) == 710)
	{
		return false;
	}
	X86_STORE_U8(ctx.edi.u32 + 0x24, 1);
	s_respawnPending[number] = true;
	s_respawnTimers[number] = 0.0f;
	return true;
}

// P3 / P4 use P2's checkpoint
static void RespawnPlayer(X86Context& ctx, uint8_t* base, uint32_t player, uint32_t number)
{
	const uint32_t gameState = X86_LOAD_U32(0x54963C);
	if (!gameState)
	{
		return;
	}
	const uint32_t id = X86_LOAD_U32(player + 0x298);
	const uint32_t checkpointId = std::min(id, 1u);
	const uint32_t pos = GetExtraPlayerStartPos(id, GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_22D440, gameState, checkpointId));
	const uint32_t rot = GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_22D460, gameState, checkpointId);
	GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_A45B0, id, pos, rot);
	// revive command
	const uint32_t command = GuestToHostFunction<uint32_t, CallConvention::Cdecl>(ctx, sub_4F6F0, 0x14u);
	if (command)
	{
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_864E0, command);
		const uint32_t receive = X86_LOAD_U32(X86_LOAD_U32(player) + 0x8);
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, receive, player, command);
	}
	GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_22D3C0, GetPlayerBlock(number) + 0x28);
}

void IndependentRespawnTickMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t player = X86_LOAD_U32(ctx.edi.u32 + 0x4);
	const uint32_t number = X86_LOAD_U32(player + 0x54);
	if (!Config::ExtendedMultiplayer || number >= 4 || !s_respawnPending[number] || !IsStagePlaying())
	{
		return;
	}
	s_respawnTimers[number] += X86_LOAD_F32(ctx.esp.u32 + 0x24);
	if (s_respawnTimers[number] < 2.5f)
	{
		return;
	}
	s_respawnPending[number] = false;
	s_respawnReady[number] = true;
}

static void ReleaseStaleAreaAttacks(const X86Context& ctx);
static void UpdatePlayerTargetModes();

// respawn outside the Dead exec
void MultiplayerPatchesUpdate(X86Context& ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	ReleaseStaleAreaAttacks(ctx);
	UpdatePlayerTargetModes();
	for (uint32_t i = 0; i < 4; i++)
	{
		if (!s_respawnReady[i])
		{
			continue;
		}
		s_respawnReady[i] = false;
		const uint32_t player = GetPlayerByNumber(i);
		if (player && GetPlayerBehaviorId(player) == BEHAVIOR_DEAD && IsStagePlaying())
		{
			RespawnPlayer(ctx, base, player, i);
		}
	}
	// Super for respawned players
	if (!IsStagePlaying() || !IsSuperStage())
	{
		return;
	}
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		const uint32_t behavior = player ? GetPlayerBehaviorId(player) : UINT32_MAX;
		if (behavior != UINT32_MAX && behavior != BEHAVIOR_DEAD && !s_respawnPending[i] && !HasSuper(player))
		{
			MakeSuper(ctx, player);
		}
	}
}

// DevilDoom target lock fix
static uint32_t s_devilDoomTarget = 0;
static float s_devilDoomLockTimer = 0.0f;
static float s_devilDoomGraceTimer = 0.0f;

static bool IsLivePlayer(uint32_t player)
{
	return IsMultiplayerPlayer(player) && GetPlayerBehaviorId(player) != BEHAVIOR_DEAD;
}

void DevilDoomTargetMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	const float dt = X86_LOAD_F32(ctx.esp.u32 + 0x5C);
	const uint32_t devilDoom = X86_LOAD_U32(ctx.esi.u32 - 0x8);
	const uint32_t model = X86_LOAD_U32(devilDoom + 0x1C);
	s_devilDoomLockTimer = std::max(s_devilDoomLockTimer - dt, 0.0f);
	s_devilDoomGraceTimer = std::max(s_devilDoomGraceTimer - dt, 0.0f);

	if (!IsLivePlayer(s_devilDoomTarget) || s_devilDoomLockTimer <= 0.0f)
	{
		const uint32_t closest = model ? GetClosestPlayer(model + 0x4) : 0;
		if (closest && closest != s_devilDoomTarget)
		{
			if (s_devilDoomTarget)
			{
				s_devilDoomGraceTimer = 3.0f;
			}
			s_devilDoomTarget = closest;
			s_devilDoomLockTimer = 4.0f;
		}
	}
	if (IsMultiplayerPlayer(s_devilDoomTarget))
	{
		ctx.ecx.u32 = X86_LOAD_U32(s_devilDoomTarget + 0x298);
	}
}

// skip the too close warp during the grace
bool DevilDoomWarpGraceMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || s_devilDoomGraceTimer <= 0.0f)
	{
		return false;
	}
	const uint32_t devilDoom = ctx.ecx.u32;
	const float dx = X86_LOAD_F32(devilDoom + 0x1D4) - X86_LOAD_F32(devilDoom + 0x1C8);
	const float dz = X86_LOAD_F32(devilDoom + 0x1DC) - X86_LOAD_F32(devilDoom + 0x1D0);
	if (dx * dx + dz * dz < 50.0f * 50.0f)
	{
		s_devilDoomGraceTimer = 0.0f;
		return false;
	}
	return true;
}

// Chunk Fix (same idea as how I did it on GC)
// block visibility as a viewport mask
static uint16_t s_blockMask = 0xF;

static bool IsPointInBlock(uint8_t* base, uint32_t point, uint32_t min, uint32_t max)
{
	for (uint32_t i = 0; i < 3; i++)
	{
		const float p = X86_LOAD_F32(point + i * 0x4);
		if (p < X86_LOAD_F32(min + i * 0x4) || p >= X86_LOAD_F32(max + i * 0x4))
		{
			return false;
		}
	}
	return true;
}

X86_FUNC_IMPL(__imp__sub_12700);
X86_OVERRIDE_FUNC(sub_12700)
{
	const uint32_t min = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const uint32_t max = X86_LOAD_U32(ctx.esp.u32 + 0x8);
	const uint32_t pos = X86_LOAD_U32(ctx.esp.u32 + 0xC);
	const uint32_t cameras = X86_LOAD_U32(0x537770);
	if (!Config::ExtendedMultiplayer || pos != 0 || cameras == 0)
	{
		__imp__sub_12700(ctx, base);
		s_blockMask = 0xF;
		return;
	}
	uint16_t mask = 0;
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t cameraPos = GetCameraPos(ctx, i);
		if (cameraPos && IsPointInBlock(base, cameraPos, min, max))
		{
			mask |= 1 << i;
		}
	}
	s_blockMask = mask;
	ctx.eax.u8_lo = mask != 0;
	ctx.esp.u32 += 0x10;
}

// forced blocks on every viewport
void BlockMaskResetMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	s_blockMask = 0xF;
}

// OR masks per block id
bool BlockMaskStoreMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return false;
	}
	const uint32_t visible = ctx.esp.u32 + ctx.edi.u32 * 0x2 + 0x18;
	X86_STORE_U16(visible, X86_LOAD_U16(visible) | s_blockMask);
	return true;
}

// Render Chunk Fix
void LandDispBlockViewportMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || ctx.eax.u8_lo == 0)
	{
		return;
	}
	const uint32_t viewport = X86_LOAD_U32(0x54DDA0);
	const uint32_t blockManage = X86_LOAD_U32(ctx.esp.u32 + 0x14);
	const uint32_t block = X86_LOAD_U32(ctx.ebx.u32 + 0xBC);
	const uint16_t mask = X86_LOAD_U16(blockManage + X86_LOAD_U32(blockManage + 0x44) * 0x150 + block * 0x2 + 0x48);
	if (viewport < 4 && (mask & (1 << viewport)) == 0)
	{
		ctx.eax.u8_lo = 0;
	}
}

// Remove ScreenShake
bool NoScreenShakeMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Circus Park Hero Mission Co-Op
void RingMissionCoOpMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || X86_LOAD_U32(0x54963C) != GAME_STATE)
	{
		return;
	}
	for (uint32_t i = 1; i < g_multiplayerPlayerCount; i++)
	{
		ctx.eax.u32 += X86_LOAD_U32(GetPlayerBlock(i) + 0x28);
	}
}

// Route Menu - Rewrite PORT Rules
bool RouteMenuAnyPlayerClose()
{
	return Config::ExtendedMultiplayer && FindPausePort(0, PJSPeripheral::BTN_Y | PJSPeripheral::BTN_B);
}

// Vibration Fix for P2
void VibrationUnitCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		X86_STORE_U32(ctx.ebx.u32 + 0x4, g_multiplayerPlayerCount);
	}
}

void VibrationUnitPortMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.edx.u32 = ctx.esi.u32;
	}
}

void VibrationRequestPortMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.eax.u32 = ctx.ebx.u32;
	}
}

void VibrationRequestPosPortMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.eax.u32 = ctx.edi.u32;
	}
}

void VibrationPlayerPortMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.eax.u32 = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	}
}

void VibrationRequestPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t handle = X86_LOAD_U32(ctx.esp.u32 + 0x14);
	const uint32_t player = handle ? X86_LOAD_U32(handle) : 0;
	if (Config::ExtendedMultiplayer && IsMultiplayerPlayer(player))
	{
		X86_STORE_U32(ctx.esp.u32, X86_LOAD_U32(player + 0x54));
	}
}

// Disable ChaosControl CameraEffect
X86_FUNC_IMPL(__imp__sub_8FEC0);
X86_OVERRIDE_FUNC(sub_8FEC0)
{
	if (Config::ExtendedMultiplayer)
	{
		ctx.esp.u32 += 0x4;
		return;
	}
	__imp__sub_8FEC0(ctx, base);
}

bool NoRadialBlurMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return Config::ExtendedMultiplayer;
}

// Boss area attack fix for HeavyDog/BlueFalcon/BlackBull (cursed sphere write implementation)
struct AreaAttackSlots
{
	uint32_t handles[4][2] = {};
	uint32_t frame = 0;
};

static std::unordered_map<uint32_t, AreaAttackSlots> s_areaAttacks;
static uint32_t s_areaAttackFrame = 0;
static uint32_t s_areaAttackUnit = 0;
static int32_t s_areaAttackPlayer = -1;
static int32_t s_areaAttackFirst = -1;
static uint32_t s_areaAttackTarget[3] = {};
static uint32_t s_areaAttackStash[2] = {};

static int32_t GetNextAreaAttackPlayer(int32_t after)
{
	for (int32_t i = after + 1; i < int32_t(g_multiplayerPlayerCount); i++)
	{
		if (GetPlayerByNumber(i))
		{
			return i;
		}
	}
	return -1;
}

static void SetAreaAttackTarget(uint32_t pos, int32_t number)
{
	CopyVec3(pos, GetPlayerByNumber(number) + 0x4);
}

static void ReleaseAreaAttackHandle(const X86Context& ctx, uint32_t* handle)
{
	const uint32_t ref = handle[1];
	handle[0] = 0;
	handle[1] = 0;
	if (ref)
	{
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12350, ref);
	}
}

static void BeginAreaAttack(X86Context& ctx, uint32_t unit, uint32_t pos)
{
	auto base = g_memory.base;
	if (!Config::ExtendedMultiplayer || s_areaAttackUnit == unit)
	{
		return;
	}
	const int32_t first = GetNextAreaAttackPlayer(-1);
	if (first < 0)
	{
		return;
	}
	memcpy(s_areaAttackTarget, base + pos, 0xC);
	s_areaAttackUnit = unit;
	s_areaAttackFirst = first;
	s_areaAttackPlayer = first;
	SetAreaAttackTarget(pos, first);
}

static bool EndAreaAttack(X86Context& ctx, uint32_t unit, uint32_t pos, uint32_t field)
{
	auto base = g_memory.base;
	if (s_areaAttackUnit != unit)
	{
		return false;
	}
	AreaAttackSlots& slots = s_areaAttacks[unit];
	StoreHandle(field, s_areaAttackPlayer == s_areaAttackFirst ? s_areaAttackStash : slots.handles[s_areaAttackPlayer]);
	slots.frame = s_areaAttackFrame;
	const int32_t next = GetNextAreaAttackPlayer(s_areaAttackPlayer);
	if (next >= 0)
	{
		ReleaseAreaAttackHandle(ctx, slots.handles[next]);
		SetAreaAttackTarget(pos, next);
		s_areaAttackPlayer = next;
		return true;
	}
	memcpy(base + pos, s_areaAttackTarget, 0xC);
	LoadHandle(field, s_areaAttackStash);
	s_areaAttackUnit = 0;
	s_areaAttackPlayer = -1;
	return false;
}

static void ReleaseStaleAreaAttacks(const X86Context& ctx)
{
	auto base = g_memory.base;
	s_areaAttackFrame++;
	// handle some bosses calling sub_14E030 (black bull in death ruins)
	if (!s_areaAttacks.empty() && X86_LOAD_U32(0x541BA0) == 0)
	{
		s_areaAttacks.clear();
		return;
	}
	for (auto it = s_areaAttacks.begin(); it != s_areaAttacks.end();)
	{
		if (s_areaAttackFrame - it->second.frame < 2)
		{
			++it;
			continue;
		}
		for (auto& handle : it->second.handles)
		{
			ReleaseAreaAttackHandle(ctx, handle);
		}
		it = s_areaAttacks.erase(it);
	}
}

void BlackBullExplosionBeginMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	BeginAreaAttack(ctx, ctx.edi.u32, X86_LOAD_U32(ctx.edi.u32 - 0x30) + 0x150);
}

bool BlackBullExplosionEndMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!EndAreaAttack(ctx, ctx.edi.u32, X86_LOAD_U32(ctx.edi.u32 - 0x30) + 0x150, ctx.edi.u32 - 0xC))
	{
		return false;
	}
	ctx.esi.u32 = ctx.edi.u32 - 0xC;
	ctx.ebp.u32 = 0;
	return true;
}

void BlackBullFireWallBeginMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	BeginAreaAttack(ctx, ctx.esi.u32, X86_LOAD_U32(ctx.esi.u32 - 0x40) + 0x150);
}

bool BlackBullFireWallEndMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!EndAreaAttack(ctx, ctx.esi.u32, X86_LOAD_U32(ctx.esi.u32 - 0x40) + 0x150, ctx.esi.u32 - 0xC))
	{
		return false;
	}
	ctx.edi.u32 = ctx.esi.u32 - 0xC;
	ctx.ebx.u32 = ctx.esi.u32 - 0x34;
	return true;
}

void BlackBullWaveBeginMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	BeginAreaAttack(ctx, ctx.esi.u32, X86_LOAD_U32(ctx.esi.u32 - 0x60) + 0x150);
}

bool BlackBullWaveEndMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!EndAreaAttack(ctx, ctx.esi.u32, X86_LOAD_U32(ctx.esi.u32 - 0x60) + 0x150, ctx.esi.u32 - 0xC))
	{
		return false;
	}
	ctx.edi.u32 = ctx.esi.u32 - 0xC;
	return true;
}

// Heavy Dog / Blue Falcon
void HeavyDogBlueFalconParticleBeamCannonBeginMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	BeginAreaAttack(ctx, ctx.esi.u32, X86_LOAD_U32(ctx.esi.u32 - 0x3C) + 0x194);
}

bool HeavyDogBlueFalconParticleBeamCannonEndMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!EndAreaAttack(ctx, ctx.esi.u32, X86_LOAD_U32(ctx.esi.u32 - 0x3C) + 0x194, ctx.esi.u32 - 0x14))
	{
		return false;
	}
	ctx.edi.u32 = ctx.esi.u32 - 0x14;
	ctx.ebp.u32 = ctx.esi.u32 - 0x28;
	return true;
}

// BossCams
// FixLookCharCamera per player
struct BossCameraHandle
{
	uint32_t ptr;
	uint32_t ref;
};
static std::unordered_map<uint32_t, std::array<BossCameraHandle, 4>> s_bossCameraHandles;
static uint32_t s_bossCameraIndex = 0;

// per player attach target
struct BossCameraAttach
{
	uint32_t check;
	uint32_t status;
	uint32_t pos;
	uint32_t distance;
	bool mode;
};

// These bosses have special attach/detach behavior
static constexpr BossCameraAttach BOSS_CAMERA_ATTACH[] =
{
	{ 0xDA660, 0x38, 0x150, 0x15C, false }, // Black Bull 0210
	{ 0xDE210, 0x38, 0x150, 0x15C, true }, // Black Bull 0412
	{ 0xEA9D0, 0x38, 0x194, 0x1B0, true }, // Blue Falcon (HeavyDog0510)
	{ 0xF0960, 0x34, 0x13C, 0, false }, // Black Doom (doesn't use distance, has a status check instead for running under)
};

struct BossCameraState
{
	uint32_t params[7];
	uint32_t mode;
};

static std::unordered_map<uint32_t, std::array<BossCameraState, 4>> s_bossCameraStates;
static const BossCameraAttach* GetBossCameraAttach(uint32_t camera)
{
	auto base = g_memory.base;
	const uint32_t check = X86_LOAD_U32(X86_LOAD_U32(camera - 0x34));
	for (const BossCameraAttach& attach : BOSS_CAMERA_ATTACH)
	{
		if (attach.check == check)
		{
			return &attach;
		}
	}
	return nullptr;
}

static void LoadBossCameraState(uint32_t camera, const BossCameraState& state)
{
	auto base = g_memory.base;
	for (uint32_t i = 0; i < 7; i++)
	{
		X86_STORE_U32(camera - 0x2C + i * 0x4, state.params[i]);
	}
	X86_STORE_U32(camera + 0x8, state.mode);
}

static BossCameraState GetBossCameraState(uint32_t camera)
{
	auto base = g_memory.base;
	BossCameraState state;
	for (uint32_t i = 0; i < 7; i++)
	{
		state.params[i] = X86_LOAD_U32(camera - 0x2C + i * 0x4);
	}
	state.mode = X86_LOAD_U32(camera + 0x8);
	return state;
}

static void SetBossCameraTarget(const BossCameraAttach& attach, uint32_t status, uint32_t number)
{
	auto base = g_memory.base;
	const uint32_t player = GetPlayerByNumber(number);
	const uint32_t boss = X86_LOAD_U32(status + 0x1C);
	if (!player || !boss)
	{
		return;
	}
	CopyVec3(status + attach.pos, player + 0x4);
	if (!attach.distance)
	{
		return;
	}
	const float dx = X86_LOAD_F32(player + 0x4) - X86_LOAD_F32(boss + 0x4);
	const float dz = X86_LOAD_F32(player + 0xC) - X86_LOAD_F32(boss + 0xC);
	X86_STORE_F32(status + attach.distance, sqrtf(dx * dx + dz * dz));
}

X86_FUNC_IMPL(__imp__sub_D2B20);
X86_OVERRIDE_FUNC(sub_D2B20)
{
	if (!Config::ExtendedMultiplayer)
	{
		__imp__sub_D2B20(ctx, base);
		return;
	}
	const uint32_t camera = ctx.ecx.u32;
	const uint32_t handle = camera - 0xC;
	auto& handles = s_bossCameraHandles[camera];
	const X86Context entry = ctx;
	const BossCameraHandle own = { X86_LOAD_U32(handle), X86_LOAD_U32(handle + 0x4) };
	const uint8_t cut = X86_LOAD_U8(camera - 0x10);
	const BossCameraAttach* attach = GetBossCameraAttach(camera);
	const uint32_t status = attach ? X86_LOAD_U32(camera - 0x34 + attach->status) : 0;
	uint32_t target[3] = {};
	uint32_t distance = 0;
	if (status)
	{
		memcpy(target, base + status + attach->pos, 0xC);
		if (attach->distance)
		{
			distance = X86_LOAD_U32(status + attach->distance);
		}
	}
	const BossCameraState ownState = GetBossCameraState(camera);
	auto stateIt = s_bossCameraStates.find(camera);
	if (attach && attach->mode && stateIt == s_bossCameraStates.end())
	{
		stateIt = s_bossCameraStates.emplace(camera, std::array<BossCameraState, 4>{ ownState, ownState, ownState, ownState }).first;
	}
	const bool perPlayerState = stateIt != s_bossCameraStates.end();
	auto runPass = [&](uint32_t i, const BossCameraHandle& cameraHandle, const BossCameraState& state)
	{
		ctx = entry;
		X86_STORE_U8(camera - 0x10, cut);
		X86_STORE_U32(handle, cameraHandle.ptr);
		X86_STORE_U32(handle + 0x4, cameraHandle.ref);
		if (status)
		{
			SetBossCameraTarget(*attach, status, i);
		}
		if (perPlayerState)
		{
			LoadBossCameraState(camera, state);
		}
		s_bossCameraIndex = i;
		__imp__sub_D2B20(ctx, base);
	};
	for (uint32_t i = 1; i < g_multiplayerPlayerCount; i++)
	{
		runPass(i, handles[i], perPlayerState ? stateIt->second[i] : ownState);
		handles[i] = { X86_LOAD_U32(handle), X86_LOAD_U32(handle + 0x4) };
		if (perPlayerState)
		{
			stateIt->second[i] = GetBossCameraState(camera);
		}
	}
	runPass(0, own, ownState);
	if (status)
	{
		memcpy(base + status + attach->pos, target, 0xC);
		if (attach->distance)
		{
			X86_STORE_U32(status + attach->distance, distance);
		}
	}
}

// BossCams SpecialFree
void BossCameraReleaseMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const auto it = s_bossCameraHandles.find(ctx.esi.u32);
	if (it == s_bossCameraHandles.end())
	{
		return;
	}
	const uint32_t handle = ctx.esi.u32 - 0xC;
	const BossCameraHandle own = { X86_LOAD_U32(handle), X86_LOAD_U32(handle + 0x4) };
	for (const BossCameraHandle& other : it->second)
	{
		if (!other.ref)
		{
			continue;
		}
		X86_STORE_U32(handle, other.ptr);
		X86_STORE_U32(handle + 0x4, other.ref);
		if (X86_LOAD_U32(other.ref + 0x4))
		{
			GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_165310, handle);
		}
		else
		{
			GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12070, other.ref);
		}
	}
	X86_STORE_U32(handle, own.ptr);
	X86_STORE_U32(handle + 0x4, own.ref);
	s_bossCameraHandles.erase(it);
	s_bossCameraStates.erase(ctx.esi.u32);
}

void BossCameraIndexMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.esp.u32, s_bossCameraIndex);
}

// Note: DON'T merge this with the above, incase we change it later
void BossCameraPlayerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.esp.u32, s_bossCameraIndex);
}

// BossCams chase camera per player, because Black Doom SUCKS
static std::unordered_map<uint32_t, std::array<uint32_t, 4>> s_chaseCameras;
static uint32_t s_chaseCameraIndex = 0;
static bool s_chaseCameraSolo = false;

X86_FUNC_IMPL(__imp__sub_126E10);
X86_OVERRIDE_FUNC(sub_126E10)
{
	if (Config::ExtendedMultiplayer)
	{
		s_chaseCameras[ctx.ecx.u32] = {};
	}
	s_chaseCameraIndex = 0;
	__imp__sub_126E10(ctx, base);
}

// build missing wrappers (woohoo I LOVE consistency in my BOSS CODE)
X86_FUNC_IMPL(__imp__sub_126250);
static void CreateChaseCameras(const X86Context& ctx, uint32_t wrapper, std::array<uint32_t, 4>& others)
{
	auto base = g_memory.base;
	const uint32_t impl = X86_LOAD_U32(wrapper);
	if (!impl)
	{
		return;
	}
	const uint32_t cameraCount = GetCameraCount();
	for (uint32_t i = 1; i < g_multiplayerPlayerCount && i < cameraCount; i++)
	{
		if (others[i])
		{
			continue;
		}
		const uint32_t playerHandle = GetPlayerHandleByNumber(i);
		if (!playerHandle)
		{
			continue;
		}
		const uint32_t other = GuestToHostFunction<uint32_t, CallConvention::Cdecl>(ctx, sub_37D6B3, 0x4u);
		if (!other)
		{
			continue;
		}
		s_chaseCameraIndex = i;
		GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, __imp__sub_126E10, other, playerHandle, impl + 0x18);
		s_chaseCameraIndex = 0;
		others[i] = other;
		const uint32_t otherImpl = X86_LOAD_U32(other);
		memcpy(base + otherImpl + 0x38, base + impl + 0x38, 0x30);
		memcpy(base + otherImpl + 0x70, base + impl + 0x70, 0x28);
		const uint32_t appliedRef = X86_LOAD_U32(impl + 0x4);
		if (appliedRef && X86_LOAD_U32(appliedRef + 0x4))
		{
			GuestToHostFunction<void, CallConvention::Thiscall>(ctx, __imp__sub_126250, other, 0u);
		}
	}
}

// chase camera for every player and boss wrapper (shared for subboss and black doom)
static void RunChaseCameraPerPlayer(X86Context& ctx, uint8_t* base, X86Func* func, uint32_t argCount, bool create = false)
{
	const auto it = s_chaseCameras.find(ctx.ecx.u32);
	if (it == s_chaseCameras.end())
	{
		func(ctx, base);
		return;
	}
	if (create)
	{
		CreateChaseCameras(ctx, it->first, it->second);
	}
	const X86Context entry = ctx;
	std::array<uint32_t, 2> args = {};
	for (uint32_t i = 0; i < argCount; i++)
	{
		args[i] = X86_LOAD_U32(entry.esp.u32 + 0x4 + i * 0x4);
	}
	for (const uint32_t other : it->second)
	{
		if (!other)
		{
			continue;
		}
		ctx = entry;
		ctx.ecx.u32 = other;
		func(ctx, base);
		for (uint32_t i = 0; i < argCount; i++)
		{
			X86_STORE_U32(entry.esp.u32 + 0x4 + i * 0x4, args[i]);
		}
	}
	ctx = entry;
	func(ctx, base);
}

void ChaseCameraIndexMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.esp.u32 + 0x8, s_chaseCameraIndex);
}

// release camera mode
X86_FUNC_IMPL(__imp__sub_125E70);
X86_OVERRIDE_FUNC(sub_125E70)
{
	if (s_chaseCameraSolo)
	{
		__imp__sub_125E70(ctx, base);
		return;
	}
	RunChaseCameraPerPlayer(ctx, base, __imp__sub_125E70, 0);
}

// orbit / look params
X86_FUNC_IMPL(__imp__sub_125E80);
X86_OVERRIDE_FUNC(sub_125E80)
{
	RunChaseCameraPerPlayer(ctx, base, __imp__sub_125E80, 1);
}

X86_FUNC_IMPL(__imp__sub_125EB0);
X86_OVERRIDE_FUNC(sub_125EB0)
{
	RunChaseCameraPerPlayer(ctx, base, __imp__sub_125EB0, 1);
}

X86_FUNC_IMPL(__imp__sub_125ED0);
X86_OVERRIDE_FUNC(sub_125ED0)
{
	RunChaseCameraPerPlayer(ctx, base, __imp__sub_125ED0, 1);
}

X86_FUNC_IMPL(__imp__sub_125F00);
X86_OVERRIDE_FUNC(sub_125F00)
{
	RunChaseCameraPerPlayer(ctx, base, __imp__sub_125F00, 1);
}

// Diablon pitch per followed player (fixes the target snap bs)
static void SetChaseCameraPitch(uint32_t wrapper, float pitch, float yaw)
{
	auto base = g_memory.base;
	const uint32_t impl = X86_LOAD_U32(wrapper);
	if (!impl)
	{
		return;
	}
	const uint32_t player = X86_LOAD_U32(impl + 0x10);
	const uint32_t ref = X86_LOAD_U32(impl + 0x14);
	if (player && ref && X86_LOAD_U32(ref + 0x4))
	{
		const float height = std::clamp(float((X86_LOAD_F32(player + 0x8) - X86_LOAD_F32(0x466570)) * X86_LOAD_F32(0x42A85C)), 0.0f, float(X86_LOAD_F32(0x46B438)));
		const float toRadians = X86_LOAD_F32(0x47E0A4) * X86_LOAD_F32(0x42A880); //bams moment
		pitch = (X86_LOAD_F32(0x44AD48) - height * X86_LOAD_F32(0x449B28)) * toRadians;
	}
	X86_STORE_F32(impl + 0x80, pitch);
	X86_STORE_F32(impl + 0x84, yaw);
}

X86_FUNC_IMPL(__imp__sub_125F20);
X86_OVERRIDE_FUNC(sub_125F20)
{
	const auto it = s_chaseCameras.find(ctx.ecx.u32);
	if (it == s_chaseCameras.end())
	{
		__imp__sub_125F20(ctx, base);
		return;
	}
	const float pitch = X86_LOAD_F32(ctx.esp.u32 + 0x4);
	const float yaw = X86_LOAD_F32(ctx.esp.u32 + 0x8);
	for (const uint32_t other : it->second)
	{
		if (other)
		{
			SetChaseCameraPitch(other, pitch, yaw);
		}
	}
	SetChaseCameraPitch(ctx.ecx.u32, pitch, yaw);
	ctx.esp.u32 += 0xC;
}

// apply camera mode
X86_OVERRIDE_FUNC(sub_126250)
{
	if (s_chaseCameraSolo)
	{
		__imp__sub_126250(ctx, base);
		return;
	}
	RunChaseCameraPerPlayer(ctx, base, __imp__sub_126250, 1, true);
}

// update
X86_FUNC_IMPL(__imp__sub_126AE0);
X86_OVERRIDE_FUNC(sub_126AE0)
{
	RunChaseCameraPerPlayer(ctx, base, __imp__sub_126AE0, 1, true);
}

// free the other wrappers
X86_FUNC_IMPL(__imp__sub_126AC0);
X86_OVERRIDE_FUNC(sub_126AC0)
{
	const auto it = s_chaseCameras.find(ctx.ecx.u32);
	if (it != s_chaseCameras.end())
	{
		const X86Context entry = ctx;
		for (const uint32_t other : it->second)
		{
			if (!other)
			{
				continue;
			}
			GuestToHostFunction<void, CallConvention::Thiscall>(ctx, __imp__sub_126AC0, other);
			GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_37E361, other);
		}
		s_chaseCameras.erase(it);
		ctx = entry;
	}
	__imp__sub_126AC0(ctx, base);
}

// Egg Dealer CameraTask per player
static bool IsChaseCameraApplied(uint32_t wrapper)
{
	auto base = g_memory.base;
	const uint32_t impl = X86_LOAD_U32(wrapper);
	const uint32_t ref = impl ? X86_LOAD_U32(impl + 0x4) : 0;
	return ref && X86_LOAD_U32(ref + 0x4);
}

X86_FUNC_IMPL(__imp__sub_112DE0);
X86_OVERRIDE_FUNC(sub_112DE0)
{
	const uint32_t task = ctx.ecx.u32;
	const uint32_t wrapper = X86_LOAD_U32(task + 0x4);
	const auto it = s_chaseCameras.find(wrapper);
	const bool event = X86_LOAD_U32(task + 0x1C) && !X86_LOAD_U8(task + 0x58);
	if (it == s_chaseCameras.end() || X86_LOAD_U8(task + 0x5A) || event)
	{
		__imp__sub_112DE0(ctx, base);
		return;
	}
	const uint32_t boss = X86_LOAD_U32(task);
	const uint32_t ownPlayer = X86_LOAD_U32(boss + 0xE0);
	const uint32_t ownRef = X86_LOAD_U32(boss + 0xE4);
	const uint8_t ownReleased = X86_LOAD_U8(task + 0x59);
	for (uint32_t i = 1; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t other = it->second[i];
		const uint32_t playerHandle = GetPlayerHandleByNumber(i);
		if (!other || !playerHandle)
		{
			continue;
		}
		const bool applied = IsChaseCameraApplied(other);
		X86_STORE_U32(boss + 0xE0, X86_LOAD_U32(playerHandle));
		X86_STORE_U32(boss + 0xE4, X86_LOAD_U32(playerHandle + 0x4));
		X86_STORE_U8(task + 0x59, applied ? 0 : 1);
		const bool center = GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_112B10, task) & 0xFF;
		if (center && applied)
		{
			GuestToHostFunction<void, CallConvention::Thiscall>(ctx, __imp__sub_125E70, other);
		}
		else if (!center && !applied)
		{
			// blend params to keep smooth
			if (!(X86_LOAD_U32(0x54BEC4) & 1))
			{
				X86_STORE_U32(0x54BEC4, X86_LOAD_U32(0x54BEC4) | 1);
				GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_283A0, 0x54BE98u, 0x3F800000u);
				GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_37D929, 0x3CF5E0u);
			}
			GuestToHostFunction<void, CallConvention::Thiscall>(ctx, __imp__sub_126250, other, 0x54BE98u);
		}
	}
	X86_STORE_U32(boss + 0xE0, ownPlayer);
	X86_STORE_U32(boss + 0xE4, ownRef);
	X86_STORE_U8(task + 0x59, ownReleased);
	// P1's camera needs to re-map post events, otherwise it'll cut
	const uint32_t mainHandle = GetPlayerHandleByNumber(0);
	if (mainHandle)
	{
		X86_STORE_U32(boss + 0xE0, X86_LOAD_U32(mainHandle));
		X86_STORE_U32(boss + 0xE4, X86_LOAD_U32(mainHandle + 0x4));
	}
	s_chaseCameraSolo = true;
	__imp__sub_112DE0(ctx, base);
	s_chaseCameraSolo = false;
	X86_STORE_U32(boss + 0xE0, ownPlayer);
	X86_STORE_U32(boss + 0xE4, ownRef);
}

// Egg Dealer Targets Closest Player
// closest living player, with a hold
static std::unordered_map<uint32_t, float> s_eggDealerRetargetHold;

static void AssignWeakHandle(const X86Context& ctx, uint32_t dst, uint32_t src)
{
	auto base = g_memory.base;
	const uint32_t ptr = X86_LOAD_U32(src);
	const uint32_t ref = X86_LOAD_U32(src + 0x4);
	const uint32_t oldRef = X86_LOAD_U32(dst + 0x4);
	if (ref)
	{
		X86_STORE_U32(ref + 0x8, X86_LOAD_U32(ref + 0x8) + 1);
	}
	X86_STORE_U32(dst, ptr);
	X86_STORE_U32(dst + 0x4, ref);
	if (oldRef)
	{
		GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_12070, oldRef);
	}
}

X86_FUNC_IMPL(__imp__sub_1105C0);
X86_OVERRIDE_FUNC(sub_1105C0)
{
	if (!Config::ExtendedMultiplayer)
	{
		__imp__sub_1105C0(ctx, base);
		return;
	}
	const uint32_t boss = ctx.ecx.u32;
	float& hold = s_eggDealerRetargetHold[boss];
	hold = std::max(hold - X86_LOAD_F32(ctx.esp.u32 + 0x4), 0.0f);

	const uint32_t current = X86_LOAD_U32(boss + 0xE0);
	const bool currentValid = IsLivePlayer(current);
	const float currentDistSq = currentValid ? GetDistanceSq(current + 0x4, boss + 0x4) : FLT_MAX;
	uint32_t best = 0;
	uint32_t bestHandle = 0;
	float bestDistSq = FLT_MAX;
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t handle = GetPlayerHandleByNumber(i);
		const uint32_t player = handle ? X86_LOAD_U32(handle) : 0;
		if (!IsLivePlayer(player))
		{
			continue;
		}
		const float distSq = GetDistanceSq(player + 0x4, boss + 0x4);
		if (distSq < bestDistSq)
		{
			best = player;
			bestHandle = handle;
			bestDistSq = distSq;
		}
	}
	if (best && best != current && (!currentValid || (hold <= 0.0f && bestDistSq < currentDistSq * 0.64f)))
	{
		AssignWeakHandle(ctx, boss + 0xE0, bestHandle);
		hold = 3.0f;
	}
	// sync the track zone target
	const uint32_t tracker = X86_LOAD_U32(boss + 0xF8);
	if (tracker && X86_LOAD_U32(tracker + 0x4) != X86_LOAD_U32(boss + 0xE0))
	{
		AssignWeakHandle(ctx, tracker + 0x4, boss + 0xE0);
	}
	__imp__sub_1105C0(ctx, base);
}

// Partner maps to camera of whoever activate them
void PartnerCameraIndexMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer || X86_LOAD_U32(ctx.esi.u32 + 0x5C) != 2)
	{
		return;
	}
	const uint32_t follow = X86_LOAD_U32(ctx.esi.u32 + 0x290);
	const uint32_t leader = follow ? X86_LOAD_U32(follow + 0x1C) : 0;
	const uint32_t ref = follow ? X86_LOAD_U32(follow + 0x20) : 0;
	if (ref && X86_LOAD_U32(ref + 0x4) && IsMultiplayerPlayer(leader))
	{
		const uint32_t number = X86_LOAD_U32(leader + 0x54);
		if (number < GetCameraCount())
		{
			ctx.edx.u32 = number;
		}
	}
}

// Partner Despawn Fix
void PartnerDespawnCameraMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!Config::ExtendedMultiplayer)
	{
		return;
	}
	const uint32_t pos = X86_LOAD_U32(ctx.esp.u32 + 0x3C);
	const uint32_t count = std::min(GetCameraCount(), g_multiplayerPlayerCount);
	float closestDistSq = GetDistanceSq(ctx.eax.u32, pos);
	for (uint32_t i = 1; i < count; i++)
	{
		const uint32_t cameraPos = GetCameraPos(ctx, i);
		const float distSq = cameraPos ? GetDistanceSq(cameraPos, pos) : FLT_MAX;
		if (distSq < closestDistSq)
		{
			ctx.eax.u32 = cameraPos;
			closestDistSq = distSq;
		}
	}
}

// Player Target Toggle
static uint32_t s_targetModePlayers[4] = {};
static bool s_targetModeHostile[4] = {};
static uint32_t GetPlayerTargetCollider(uint32_t player)
{
	auto base = g_memory.base;
	const uint32_t target = X86_LOAD_U32(player + 0x24C);
	const uint32_t ref = target ? X86_LOAD_U32(target + 0x10) : 0;
	return ref && X86_LOAD_U32(ref + 0x4) ? X86_LOAD_U32(target + 0xC) : 0;
}

static void UpdatePlayerTargetModes()
{
	auto base = g_memory.base;
	const bool playing = IsStagePlaying();
	bool anyHostile = false;

	// friendly flag
	for (auto i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		if (player != s_targetModePlayers[i])
		{
			s_targetModePlayers[i] = player;
			s_targetModeHostile[i] = false;
		}
		const uint32_t port = player ? GetPlayerPort(player) : UINT32_MAX;
		if (playing && port < 4 && (GetPadSlot(port)->justPressed & PJSPeripheral::BTN_BACK) != 0)
		{
			s_targetModeHostile[i] = !s_targetModeHostile[i];
		}
		anyHostile |= s_targetModeHostile[i];
	}

	// target flag
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		const uint32_t collider = player ? GetPlayerTargetCollider(player) : 0;
		if (!collider)
		{
			continue;
		}
		const uint32_t flags = X86_LOAD_U32(collider + 0x30);
		const uint32_t wanted = anyHostile ? flags | 0x2C00 : flags & ~0x2C00u;
		if (wanted != flags)
		{
			X86_STORE_U32(collider + 0x30, wanted);
		}
	}
}

// TargetSearch @ 0x8
// End @ 0x14
static void DropOtherPlayerTargets(uint32_t searcher, uint32_t number)
{
	auto base = g_memory.base;
	uint32_t colliders[4] = {};
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = i != number ? GetPlayerByNumber(i) : 0;
		colliders[i] = player ? GetPlayerTargetCollider(player) : 0;
	}
	const uint32_t begin = X86_LOAD_U32(searcher + 0x8);
	const uint32_t end = X86_LOAD_U32(searcher + 0x14);
	uint32_t out = begin;
	for (uint32_t it = begin; it < end; it += 0x4)
	{
		const uint32_t entry = X86_LOAD_U32(it);
		const uint32_t target = entry ? X86_LOAD_U32(entry) : 0;
		if (target && std::find(std::begin(colliders), std::end(colliders), target) != std::end(colliders))
		{
			continue;
		}
		X86_STORE_U32(out, entry);
		out += 0x4;
	}
	X86_STORE_U32(searcher + 0x14, out);
}

static void FilterFriendlySearch(uint32_t searcher, uint32_t number)
{
	if (number < g_multiplayerPlayerCount && !s_targetModeHostile[number])
	{
		DropOtherPlayerTargets(searcher, number);
	}
}

// Target Search Exclude
X86_FUNC_IMPL(__imp__sub_61300);
X86_OVERRIDE_FUNC(sub_61300)
{
	const uint32_t searcher = ctx.ecx.u32;
	const uint32_t handle = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const uint32_t target = handle ? X86_LOAD_U32(handle) : 0;
	__imp__sub_61300(ctx, base);
	if (!Config::ExtendedMultiplayer || !target)
	{
		return;
	}
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		if (player && GetPlayerTargetCollider(player) == target)
		{
			FilterFriendlySearch(searcher, i);
			return;
		}
	}
}

// Target Search Exclude (for pWeapon, like lock-on weapons)
X86_FUNC_IMPL(__imp__sub_61360);
X86_OVERRIDE_FUNC(sub_61360)
{
	const uint32_t searcher = ctx.ecx.u32;
	const uint32_t handle = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const uint32_t owner = handle ? X86_LOAD_U32(handle) : 0;
	__imp__sub_61360(ctx, base);
	if (!Config::ExtendedMultiplayer || !owner)
	{
		return;
	}
	for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
	{
		const uint32_t player = GetPlayerByNumber(i);
		const uint32_t collider = player ? GetPlayerTargetCollider(player) : 0;
		if (player && (owner == player || (collider && X86_LOAD_U32(collider + 0x8) == owner)))
		{
			FilterFriendlySearch(searcher, i);
			return;
		}
	}
}
// End Player Target Toggle
