#pragma once

struct X86Context;

void MultiplayerPatchesInit();
uint32_t GetMultiplayerPausePort();
void MultiplayerPatchesUpdate(X86Context& ctx, uint8_t* base);
bool RouteMenuAnyPlayerClose();
bool IsExtraPlayerMode();

struct MultiplayerHudCounters
{
	uint32_t rings;
	uint32_t ammo;
	uint32_t weaponType; // 0 = hide ammo
};
bool GetMultiplayerHudCounters(uint32_t number, MultiplayerHudCounters& counters);

struct MultiplayerHudMission
{
	bool visible;
	int32_t current;
	int32_t target;
};

struct MultiplayerHudShared
{
	float time;
	float heroGauge;
	float darkGauge;
	MultiplayerHudMission dark;
	MultiplayerHudMission hero;
	bool bossVisible;
	float bossGauge;
};

void GetMultiplayerHudShared(MultiplayerHudShared& shared);
void NotifyMultiplayerHudBossGauge();
float GetMultiplayerViewScale();
bool AdjustMultiplayerItemIcon(const X86Context& ctx);
uint32_t PopAwakeGlowClone(uint32_t awake);
