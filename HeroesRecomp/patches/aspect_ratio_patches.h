#pragma once

#include <xxHashMap.h>

#define MAKE_BITFLAG32(bit) 1U << bit
#define MAKE_BITFLAG64(bit) 1ULL << bit

inline constexpr float NARROW_ASPECT_RATIO = 4.0f / 3.0f;
inline constexpr float WIDE_ASPECT_RATIO = 16.0f / 9.0f;
inline constexpr float STEAM_DECK_ASPECT_RATIO = 16.0f / 10.0f;

inline float g_aspectRatio;
inline float g_aspectRatioOffsetX;
inline float g_aspectRatioOffsetY;
inline float g_aspectRatioScale;
inline float g_aspectRatioGameplayScale;
inline float g_aspectRatioNarrowScale;
inline float g_aspectRatioNarrowMargin;
inline float g_horzCentre;
inline float g_vertCentre;

// 1.0 at 4:3 and below, 0.75 at 16:9, ~0.5625 at 21:9.
inline float g_hudUnstretchX = 1.0f;

// Same for the Y axis: only goes under 1.0 on screens narrower than 4:3.
inline float g_hudUnstretchY = 1.0f;

struct HudWidthPreset
{
    const char* Name;
    float AspectRatio;
};

inline constexpr HudWidthPreset g_hudWidthPresets[] =
{
    { nullptr, 0.0f }, // MAX
    { "4:3", NARROW_ASPECT_RATIO },
    { "16:9", WIDE_ASPECT_RATIO },
    { "16:10", STEAM_DECK_ASPECT_RATIO },
    { "21:9", 21.0f / 9.0f },
};

// -1 for a custom value
int FindHudWidthPreset(float aspectRatio);

struct AspectRatioPatches
{
    static void ComputeOffsets();
};
void UpdateUserConfigConstant(bool toggle);
ImVec2 GuestTextToViewport(float x, float y);

// -------------- CSD MODIFIERS --------------- //

// All 2D is drawn in a fixed 640x480 space that gets stretched over the whole viewport.
// On wide screens the HUD looks too wide, so we squeeze it back around an anchor point.

enum CsdFlags : uint64_t
{
    CSD_NONE = 0,

    // Squeeze the element back to square pixels
    CSD_UNSTRETCH = MAKE_BITFLAG64(0),

    // Where the element is pinned in the 640x480 layout, one flag per axis.
    // Centre is the default, it can never break a layout since every quad moves to
    // the same point. Give every quad of a widget the same flag.

    CSD_ALIGN_LEFT = MAKE_BITFLAG64(1),
    CSD_ALIGN_CENTRE_H = MAKE_BITFLAG64(2),
    CSD_ALIGN_RIGHT = MAKE_BITFLAG64(3),

    CSD_ALIGN_TOP = MAKE_BITFLAG64(4),
    CSD_ALIGN_CENTRE_V = MAKE_BITFLAG64(5),
    CSD_ALIGN_BOTTOM = MAKE_BITFLAG64(6),


    CSD_ALIGN_AUTO_H = MAKE_BITFLAG64(7),
    CSD_ALIGN_AUTO_V = MAKE_BITFLAG64(8),

    CSD_ALIGN_CENTRE = CSD_ALIGN_CENTRE_H | CSD_ALIGN_CENTRE_V,
    CSD_ALIGN_AUTO = CSD_ALIGN_AUTO_H | CSD_ALIGN_AUTO_V,

    CSD_ALIGN_TOP_LEFT = CSD_ALIGN_TOP | CSD_ALIGN_LEFT,
    CSD_ALIGN_TOP_CENTRE = CSD_ALIGN_TOP | CSD_ALIGN_CENTRE_H,
    CSD_ALIGN_TOP_RIGHT = CSD_ALIGN_TOP | CSD_ALIGN_RIGHT,
    CSD_ALIGN_BOTTOM_LEFT = CSD_ALIGN_BOTTOM | CSD_ALIGN_LEFT,
    CSD_ALIGN_BOTTOM_CENTRE = CSD_ALIGN_BOTTOM | CSD_ALIGN_CENTRE_H,
    CSD_ALIGN_BOTTOM_RIGHT = CSD_ALIGN_BOTTOM | CSD_ALIGN_RIGHT,



    // Unstretch even when the primitive covers the whole layout, for the rare full
    // width element that is a widget and not a background.
    CSD_FORCE_UNSTRETCH = MAKE_BITFLAG64(9),

    // Skip the draw call.
    CSD_SKIP = MAKE_BITFLAG64(10),

    // The other flags only apply on a screen wider / narrower than 16:9.
    CSD_ULTRAWIDE_ONLY = MAKE_BITFLAG64(11),
    CSD_NARROW_ONLY = MAKE_BITFLAG64(12),

    // Pause menu stuff, see pause_patches. We add options to the pause menu,
    // so its background grows and everything below it moves down with it.
    CSD_HIDE_PAUSE = MAKE_BITFLAG64(13),
    CSD_PAUSE_BOSS = MAKE_BITFLAG64(14),
    CSD_EXTEND_BOTTOM = MAKE_BITFLAG64(15),
    CSD_MOVE_BOTTOM = MAKE_BITFLAG64(16),

    // Unstretch the quad around its own center
    CSD_ALIGN_SELF_H = MAKE_BITFLAG64(17),

    // Edge case exclusively used for the black box with low opacity on the main menu
	// It needs to extend to the edges of the screen like on 4:3.
    CSD_EXTEND_EDGE_H = MAKE_BITFLAG64(18),

    // Not limited by Max HUD Width
    CSD_IGNORE_MAX_WIDTH = MAKE_BITFLAG64(19),

    // black_cover NCP bug
    CSD_PAUSE_DIM = MAKE_BITFLAG64(20),

    CSD_CENTER_UNSTRETCH = CSD_UNSTRETCH | CSD_ALIGN_CENTRE,

    // Every flag that moves a primitive, one of them is enough to opt in.
    CSD_POSITIONING =
    CSD_UNSTRETCH | CSD_FORCE_UNSTRETCH |
    CSD_ALIGN_LEFT | CSD_ALIGN_CENTRE_H | CSD_ALIGN_RIGHT | CSD_ALIGN_AUTO_H | CSD_ALIGN_SELF_H |
    CSD_ALIGN_TOP | CSD_ALIGN_CENTRE_V | CSD_ALIGN_BOTTOM | CSD_ALIGN_AUTO_V,
};

struct CsdModifier
{
    uint64_t Flags{};
};

std::optional<CsdModifier> FindCsdModifier(uint32_t data);
