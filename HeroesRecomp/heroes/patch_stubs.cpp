// Heroes: definitions the core host needs from Shadow's patches/*.cpp, which are not built for Heroes.
//
// Most are Shadow features (pause text, multiplayer mod, bloom, codes, FPS fixes) and get neutral
// defaults here: do nothing / false / 0. Two are pure host math (screen layout) and are copied
// verbatim from patches/aspect_ratio_patches.cpp so the menus keep their layout.
// When a Heroes version of a patch exists, delete its stub here.

#include <stdafx.h>
#include <gpu/video.h>
#include <ui/imgui_utils.h>
#include <user/config.h>
#include <kernel/kernel.h>
#include <patches/global.h>
#include <patches/aspect_ratio_patches.h>
#include <patches/bloom_patches.h>
#include <patches/code_patches.h>
#include <patches/fps_patches.h>
#include <patches/multi_core_patches.h>
#include <patches/multiplayer_mod_patches.h>
#include <patches/pause_patches.h>
#include <patches/reloaded_mod_patches.h>
#include <patches/unix_patches.h>

// ---- variables (patches/global.h, patches/multiplayer_mod_patches.h)
thread_local bool g_isStartingEventAudio = false;
uint32_t g_playerStructAddrs[4] = {};
uint32_t g_multiplayerPlayerCount = 0;

// ---- Shadow feature hooks: no-ops for Heroes
void InitMultiCoreMutexesPatches() {}
void PatchUnixStrings() {}
void ReloadedValuePatches() {}
void CodeValuePatches() {}
void MultiplayerPatchesInit() {}
void PausePatchesInit() {}
void BloomConfigValueChangedCallback(IConfigDef*) {}
void FpsConfigValueChangedCallback(IConfigDef*) {}
void RequestQuitOptionForPause() {}
void DrawPauseText(X86Context*) {}

bool IsEventPlaying() { return false; }
bool IsExtraPlayerMode() { return false; }
bool GetMultiplayerHudCounters(uint32_t, MultiplayerHudCounters&) { return false; }
void GetMultiplayerHudShared(MultiplayerHudShared&) {}

// Only printed in a debug log (gpu/video.cpp). -1 = no loading thread known.
int GetLoadingThreadID() { return -1; }

// ---- Pure host math, copied from patches/aspect_ratio_patches.cpp (keep in sync with upstream)
#define widthMin 640.f
#define heightMin 480.f

static float ComputeScale(float aspectRatio)
{
	return ((aspectRatio * heightMin) / widthMin) / sqrt((aspectRatio * heightMin) / widthMin);
}

void AspectRatioPatches::ComputeOffsets()
{
	float width = Video::s_viewportWidth;
	float height = Video::s_viewportHeight;

	g_aspectRatio = width / height;
	g_aspectRatioGameplayScale = 1.0f;

	if (g_aspectRatio >= NARROW_ASPECT_RATIO)
	{
		g_aspectRatioOffsetX = (width - height * WIDE_ASPECT_RATIO) / 2.0f;
		g_aspectRatioOffsetY = 0.0f;
		g_aspectRatioScale = height / heightMin;

		// keep same scale above Steam Deck aspect ratio
		if (g_aspectRatio < WIDE_ASPECT_RATIO)
		{
			// interpolate to original 4:3 scale
			float steamDeckScale = g_aspectRatio / WIDE_ASPECT_RATIO;
			float narrowScale = ComputeScale(NARROW_ASPECT_RATIO);

			float lerpFactor = std::clamp((g_aspectRatio - NARROW_ASPECT_RATIO) / (STEAM_DECK_ASPECT_RATIO - NARROW_ASPECT_RATIO), 0.0f, 1.0f);
			g_aspectRatioGameplayScale = narrowScale + (steamDeckScale - narrowScale) * lerpFactor;
		}
	}
	else
	{
		// 4:3 crop
		g_aspectRatioOffsetX = (width - width * NARROW_ASPECT_RATIO) / 2.0f;
		g_aspectRatioOffsetY = (height - width / NARROW_ASPECT_RATIO) / 2.0f;
		g_aspectRatioScale = width / widthMin;
		g_aspectRatioGameplayScale = ComputeScale(NARROW_ASPECT_RATIO);
	}

	g_aspectRatioNarrowScale = std::clamp((g_aspectRatio - NARROW_ASPECT_RATIO) / (WIDE_ASPECT_RATIO - NARROW_ASPECT_RATIO), 0.0f, 1.0f);
	g_aspectRatioNarrowMargin = std::lerp(50.0f, 0.0f, g_aspectRatioNarrowScale);

	// the game draws all 2D in a fixed 640x480 space that we stretch over the viewport,
	// so one axis always ends up stretched. squeezing it back gives square pixels.
	g_hudUnstretchX = std::min(1.0f, NARROW_ASPECT_RATIO / g_aspectRatio);
	g_hudUnstretchY = std::min(1.0f, g_aspectRatio / NARROW_ASPECT_RATIO);

	g_horzCentre = g_aspectRatioOffsetX + 640.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;
	g_vertCentre = g_aspectRatioOffsetY + 360.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;
	UpdateFontSize();
}

int FindHudWidthPreset(float aspectRatio)
{
	for (size_t i = 0; i < std::size(g_hudWidthPresets); i++)
	{
		// mainly for float rounding bs, but also for approx manual config values
		if (std::abs(aspectRatio - g_hudWidthPresets[i].AspectRatio) < 0.01f)
		{
			return int(i);
		}
	}
	return -1;
}
