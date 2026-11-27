#include "multiplayer_hud.h"
#include <gpu/video.h>
#include <decompressor.h>
#include <patches/global.h>
#include <patches/multiplayer_mod_patches.h>
#include <res/images/common/mp_hud_ring.png.h>
#include <res/images/common/mp_hud_ammo.png.h>
#include <res/images/common/mp_hud_ammo_types.png.h>
#include <res/images/common/mp_hud_digits.png.h>
#include <res/images/common/mp_hud_meter.png.h>
#include <res/images/common/mp_hud_mission.png.h>
#include <res/images/common/HPgauge.png.h>

// 3P / 4P HUD
static std::unique_ptr<GuestTexture> g_upTexRing;
static std::unique_ptr<GuestTexture> g_upTexAmmo;
static std::unique_ptr<GuestTexture> g_upTexAmmoTypes;
static std::unique_ptr<GuestTexture> g_upTexDigits;
static std::unique_ptr<GuestTexture> g_upTexMeter;
static std::unique_ptr<GuestTexture> g_upTexMission;
static std::unique_ptr<GuestTexture> g_upTexBossMeter;
struct HudRect
{
    float x, y, w, h;
};

static bool IsHudActive()
{
    auto base = g_memory.base;
    if (X86_LOAD_U8(0x54DE78) == 0)
    {
        return false;
    }
    const uint32_t manager = X86_LOAD_U32(0x5496E4);
    if (!manager)
    {
        return false;
    }
    switch (X86_LOAD_U32(manager + 0x8)) // hiding based on stageaction
    {
        case 2: // init
        case 3: // play
        case 7: // restart
        case 8: // continue
        case 9: // dead
            return true;
        default:
            return false;
    }
}

static float GetHudAlpha()
{
    auto base = g_memory.base;
    return 1.0f - std::clamp(float(X86_LOAD_F32(0x5394B8)), 0.0f, 1.0f);
}

static HudRect GetPlayerRect(const uint32_t number, const ImVec2& display)
{
    auto base = g_memory.base;
    const float screenW = float(int32_t(X86_LOAD_U32(0x4E5D88)));
    const float screenH = float(int32_t(X86_LOAD_U32(0x4E5D8C)));
    const uint32_t rect = 0x4E5D78 + (number + 1) * 0x30 + 0x8;
    const float x = float(int32_t(X86_LOAD_U32(rect)));
    const float y = float(int32_t(X86_LOAD_U32(rect + 0x4)));
    const float w = float(int32_t(X86_LOAD_U32(rect + 0x8)));
    const float h = float(int32_t(X86_LOAD_U32(rect + 0xC)));
    if (screenW > 0.0f && screenH > 0.0f && w > 0.0f && h > 0.0f)
    {
        return { x / screenW * display.x, y / screenH * display.y, w / screenW * display.x, h / screenH * display.y };
    }
    if (g_multiplayerPlayerCount == 4)
    {
        const float halfW = display.x / 2.0f;
        const float halfH = display.y / 2.0f;
        return { (number & 1) * halfW, (number >> 1) * halfH, halfW, halfH };
    }
    const float thirdW = display.x / 3.0f;
    return { number * thirdW, 0.0f, thirdW, display.y };
}

static void DrawIcon(ImDrawList* drawList, GuestTexture* texture, const ImVec2& min, const float size, const float alpha, const ImVec2& uvMin = { 0.0f, 0.0f }, const ImVec2& uvMax = { 1.0f, 1.0f })
{
    drawList->AddImage(texture, min, { min.x + size, min.y + size }, uvMin, uvMax, IM_COL32(255, 255, 255, int32_t(alpha * 255.0f)));
}

static void DrawAmmoIcon(ImDrawList* drawList, uint32_t weaponType, const ImVec2& min, const float size, const float alpha)
{
    static constexpr int32_t s_columns[] = { -1, 1, 2, 3, 4, 0, 5 };
    const int32_t column = weaponType < std::size(s_columns) ? s_columns[weaponType] : -1;
    if (column < 0)
    {
        DrawIcon(drawList, g_upTexAmmo.get(), min, size, alpha);
        return;
    }
    DrawIcon(drawList, g_upTexAmmoTypes.get(), min, size, alpha, { column / 6.0f, 0.0f }, { (column + 1) / 6.0f, 1.0f });
}

// matching the gradient the game applies to the numbers
static void DrawDigits(ImDrawList* drawList, float centerX, float y, float unit, const char* text, float alpha)
{
    const auto count = int32_t(strlen(text));
    const float glyphW = 16.0f * unit;
    const float glyphH = 18.0f * unit;
    float x = centerX - glyphW * count * 0.5f;
    const auto appliedAlpha = int32_t(alpha * 255.0f);
    const ImU32 light = IM_COL32(255, 255, 255, appliedAlpha);
    const ImU32 dark = IM_COL32(128, 128, 128, appliedAlpha);
    drawList->PushTextureID(g_upTexDigits.get());
    for (int32_t i = 0; i < count; i++, x += glyphW)
    {
        const int32_t digit = text[i] == ':' ? 10 : text[i] == '/' ? 11 : text[i] - '0';
        if (digit < 0 || digit > 11)
        {
            continue;
        }
        const ImVec2 uvMin = { ((digit % 4) * 16 + 0.5f) / 64.0f, ((digit / 4) * 18 + 0.5f) / 64.0f };
        const ImVec2 uvMax = { ((digit % 4 + 1) * 16 - 0.5f) / 64.0f, ((digit / 4 + 1) * 18 - 0.5f) / 64.0f };
        const ImDrawIdx index = ImDrawIdx(drawList->_VtxCurrentIdx);
        drawList->PrimReserve(6, 4);
        drawList->PrimWriteIdx(index);
        drawList->PrimWriteIdx(index + 1);
        drawList->PrimWriteIdx(index + 3);
        drawList->PrimWriteIdx(index + 1);
        drawList->PrimWriteIdx(index + 2);
        drawList->PrimWriteIdx(index + 3);
        drawList->PrimWriteVtx({ x, y }, uvMin, light);
        drawList->PrimWriteVtx({ x + glyphW, y }, { uvMax.x, uvMin.y }, dark);
        drawList->PrimWriteVtx({ x + glyphW, y + glyphH }, uvMax, light);
        drawList->PrimWriteVtx({ x, y + glyphH }, { uvMin.x, uvMax.y }, dark);
    }
    drawList->PopTextureID();
}

static void DrawCounter(ImDrawList* drawList, float centerX, float y, float unit, uint32_t value, float alpha)
{
    char text[8];
    snprintf(text, sizeof(text), "%03u", std::min(value, 999u));
    DrawDigits(drawList, centerX, y, unit, text, alpha);
}

static ImU32 ScaleColor(const ImVec4& color, float brightness, float alpha)
{
    return ImGui::ColorConvertFloat4ToU32({ std::min(color.x * brightness, 1.0f), std::min(color.y * brightness, 1.0f), std::min(color.z * brightness, 1.0f), color.w * alpha });
}

// build gauge
static void DrawMeterPart(ImDrawList* drawList, GuestTexture* texture, const ImVec2& origin, float scale, float x, float y, float w, float h, const ImVec4& uv, ImU32 color)
{
    drawList->AddImage(texture, { origin.x + x * scale, origin.y + y * scale }, { origin.x + (x + w) * scale, origin.y + (y + h) * scale },
        { (uv.x + 0.5f) / 128.0f, (uv.y + 0.5f) / 128.0f }, { (uv.z - 0.5f) / 128.0f, (uv.w - 0.5f) / 128.0f }, color);
}

// HeroGauge.xncp / DarkGauge.xncp
static constexpr float CHAOS_GAUGE_WIDTH = 188.5f;

static void DrawChaosGauge(ImDrawList* drawList, const ImVec2& origin, float scale, float value, bool hero, float alpha)
{
    static constexpr ImVec4 UV_FRAME = { 0.0f, 1.0f, 45.0f, 22.0f };
    static constexpr ImVec4 UV_FILL = { 0.0f, 23.0f, 83.0f, 36.0f };
    const ImU32 white = IM_COL32(255, 255, 255, int32_t(alpha * 255.0f));
    const ImU32 backing = IM_COL32(0, 0, 0, int32_t(alpha * 80.0f));
    const ImU32 fill = hero ? IM_COL32(0, 149, 198, int32_t(alpha * 255.0f)) : IM_COL32(255, 0, 0, int32_t(alpha * 255.0f));
    if (hero)
    {
        const float fillW = 156.5f * value;
        drawList->AddRectFilled({ origin.x + 20.0f * scale, origin.y + 1.4f * scale }, { origin.x + 182.0f * scale, origin.y + 20.6f * scale }, backing);
        for (int32_t i = 0; i < 3; i++)
        {
            DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, 44.0f + 45.0f * i, 0.0f, 45.0f, 21.0f, UV_FRAME, white);
        }
        if (fillW > 0.0f)
        {
            DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, 22.0f, 4.0f, fillW, 13.0f, UV_FILL, fill);
        }
        DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, 178.5f, 0.0f, 10.0f, 21.0f, { 56.0f, 1.0f, 65.0f, 22.0f }, white);
        DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, 0.0f, 0.0f, 44.0f, 50.0f, { 84.0f, 50.0f, 128.0f, 99.0f }, white);
    }
    else
    {
        const float shift = CHAOS_GAUGE_WIDTH - 188.0f;
        const float fillW = 155.0f * value;
        drawList->AddRectFilled({ origin.x + (shift + 5.0f) * scale, origin.y + 2.0f * scale }, { origin.x + (shift + 170.6f) * scale, origin.y + 19.0f * scale }, backing);
        for (int32_t i = 0; i < 3; i++)
        {
            DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, shift + 9.0f + 45.0f * i, 0.0f, 45.0f, 21.0f, UV_FRAME, white);
        }
        if (fillW > 0.0f)
        {
            DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, shift + 164.0f - fillW, 4.0f, fillW, 13.0f, UV_FILL, fill);
        }
        DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, shift, 0.0f, 9.0f, 21.0f, { 46.0f, 1.0f, 55.0f, 22.0f }, white);
        DrawMeterPart(drawList, g_upTexMeter.get(), origin, scale, shift + 144.0f, 0.0f, 44.0f, 49.0f, { 84.0f, 0.0f, 128.0f, 49.0f }, white);
    }
}

// BossGauge.xncp
static void DrawBossGauge(ImDrawList* drawList, const ImVec2& origin, float scale, float value, float alpha)
{
    const auto texture = g_upTexBossMeter.get();
    const ImU32 white = IM_COL32(255, 255, 255, int32_t(alpha * 255.0f));
    drawList->AddRectFilled({ origin.x + 9.0f * scale, origin.y + 2.0f * scale }, { origin.x + 288.0f * scale, origin.y + 14.0f * scale }, IM_COL32(0, 0, 0, int32_t(alpha * 150.0f)));
    DrawMeterPart(drawList, texture, origin, scale, 10.0f, 0.0f, 113.0f, 33.0f, { 15.0f, 22.0f, 128.0f, 55.0f }, white);
    DrawMeterPart(drawList, texture, origin, scale, 123.0f, 0.0f, 128.0f, 20.0f, { 0.0f, 1.0f, 128.0f, 21.0f }, white);
    DrawMeterPart(drawList, texture, origin, scale, 250.0f, 0.0f, 38.0f, 33.0f, { 79.0f, 56.0f, 117.0f, 89.0f }, white);
    if (value > 0.0f)
    {
        DrawMeterPart(drawList, texture, origin, scale, 9.0f, 4.0f, 279.0f * value, 10.0f, { 0.0f, 98.0f, 128.0f, 108.0f }, white);
    }
    DrawMeterPart(drawList, texture, origin, scale, 0.0f, 0.0f, 10.0f, 33.0f, { 5.0f, 22.0f, 15.0f, 55.0f }, white);
    DrawMeterPart(drawList, texture, origin, scale, 288.0f, 0.0f, 11.0f, 33.0f, { 117.0f, 56.0f, 128.0f, 89.0f }, white);
}

static void DrawMission(ImDrawList* drawList, float innerX, float y, float unit, const MultiplayerHudMission& mission, const ImVec4& color, bool tagRight, float alpha)
{
    if (!mission.visible)
    {
        return;
    }
    char text[16];
    snprintf(text, sizeof(text), "%d/%d", std::clamp(mission.current, 0, 999), std::clamp(mission.target, 0, 999));
    const float textW = 16.0f * unit * float(strlen(text));
    const float tag = 30.0f * unit;
    const float gap = 5.0f * unit;
    const float left = tagRight ? innerX - (tag + gap + textW) : innerX;
    const float tagX = tagRight ? left + textW + gap : left;
    const float textX = tagRight ? left : left + tag + gap;
    const float tagY = y + 9.0f * unit - tag * 0.5f;
    const float rounding = tag * (5.0f / 42.0f);
    const float border = std::max(1.0f, tag * (1.5f / 42.0f));
    drawList->AddRectFilled({ tagX, tagY }, { tagX + tag, tagY + tag }, IM_COL32(110, 110, 110, int32_t(alpha * 255.0f)), rounding);
    drawList->AddRect({ tagX, tagY }, { tagX + tag, tagY + tag }, IM_COL32(0, 0, 0, int32_t(alpha * 255.0f)), rounding, 0, border);
    const float logoH = tag * 0.8f;
    const float logoW = logoH * (109.0f / 135.0f); // quad
    const ImVec2 logoMin = { tagX + (tag - logoW) * 0.5f, tagY + (tag - logoH) * 0.5f };
    drawList->AddImage(g_upTexMission.get(), logoMin, { logoMin.x + logoW, logoMin.y + logoH }, { 0.5f / 64.0f, 0.5f / 128.0f }, { 63.5f / 64.0f, 127.5f / 128.0f }, ScaleColor(color, 1.0f, alpha));
    DrawDigits(drawList, textX + textW * 0.5f, y, unit, text, alpha);
}

static void DrawSharedHud(ImDrawList* drawList, const ImVec2& display, const float alpha)
{
    MultiplayerHudShared shared{};
    GetMultiplayerHudShared(shared);

    const bool quad = g_multiplayerPlayerCount == 4;
    const float unit = quad
        ? std::min(display.x / 1280.0f, display.y / 960.0f) * 1.25f
        : std::min(display.x / 1920.0f, display.y / 480.0f) * 1.5f;
    const float centerX = display.x * 0.5f;
    const float glyphH = 18.0f * unit;
    const float gaugeScale = 0.9f * unit;
    const float bossScale = 0.9f * unit;
    const float gaugeW = CHAOS_GAUGE_WIDTH * gaugeScale;
    const float gaugeGap = (8.0f * 16.0f + 2.0f * 10.0f) * unit;
    const float timerY = 20.0f * unit;
    const float gaugeY = timerY + (glyphH - 21.0f * gaugeScale) * 0.5f;
    const auto seconds = int32_t(std::max(shared.time, 0.0f));
    char text[16];
    if (seconds / 60 < 100)
    {
        snprintf(text, sizeof(text), "%02d:%02d:%02d", seconds / 60, seconds % 60, int32_t((std::max(shared.time, 0.0f) - float(seconds)) * 100.0f));
    }
    else
    {
        snprintf(text, sizeof(text), "99:59:99");
    }
    DrawDigits(drawList, centerX, timerY, unit, text, alpha);
    DrawChaosGauge(drawList, { centerX - gaugeGap * 0.5f - gaugeW, gaugeY }, gaugeScale, shared.darkGauge, false, alpha);
    DrawChaosGauge(drawList, { centerX + gaugeGap * 0.5f, gaugeY }, gaugeScale, shared.heroGauge, true, alpha);
    if (shared.bossVisible)
    {
        const float bossY = gaugeY + 50.0f * gaugeScale + 6.0f * unit;
        DrawBossGauge(drawList, { centerX - 299.0f * bossScale * 0.5f, bossY }, bossScale, shared.bossGauge, alpha);
    }
    const float missionOffset = gaugeGap * 0.5f + gaugeW + 28.0f * unit;
    DrawMission(drawList, centerX - missionOffset, timerY, unit, shared.dark, { 0.95f, 0.15f, 0.15f, 1.0f }, true, alpha);
    DrawMission(drawList, centerX + missionOffset, timerY, unit, shared.hero, { 0.20f, 0.55f, 1.00f, 1.0f }, false, alpha);
}

static void DrawPlayerCounters(ImDrawList* drawList, const HudRect& rect, const MultiplayerHudCounters& counters, const float alpha)
{
    const float unit = std::min(rect.w / 640.0f, rect.h / 480.0f) * (g_multiplayerPlayerCount == 3 ? 1.5f : 1.0f);
    const float iconSize = 36.0f * unit;
    const float margin = 20.0f * unit;

    // draw on top slightly
    const float textY = rect.y + rect.h - margin - 18.0f * unit;
    const float iconY = textY + 7.0f * unit - iconSize;

    const float ringX = rect.x + margin;
    DrawIcon(drawList, g_upTexRing.get(), { ringX, iconY }, iconSize, alpha);
    DrawCounter(drawList, ringX + iconSize * 0.5f, textY, unit, counters.rings, alpha);
    if (counters.weaponType == 0)
    {
        return;
    }
    const float ammoX = rect.x + rect.w - margin - iconSize;
    DrawAmmoIcon(drawList, counters.weaponType, { ammoX, iconY }, iconSize, alpha);
    DrawCounter(drawList, ammoX + iconSize * 0.5f, textY, unit, counters.ammo, alpha);
}

void MultiplayerHud::Init()
{
    g_upTexRing = LOAD_ZSTD_TEXTURE(g_mpHudRing);
    g_upTexAmmo = LOAD_ZSTD_TEXTURE(g_mpHudAmmo);
    g_upTexAmmoTypes = LOAD_ZSTD_TEXTURE(g_mpHudAmmoTypes);
    g_upTexDigits = LOAD_ZSTD_TEXTURE(g_mpHudDigits);
    g_upTexMeter = LOAD_ZSTD_TEXTURE(g_mpHudMeter);
    g_upTexMission = LOAD_ZSTD_TEXTURE(g_mpHudMission);
    g_upTexBossMeter = LOAD_ZSTD_TEXTURE(g_hpGauge);
}

void MultiplayerHud::Draw()
{
    if (!IsExtraPlayerMode() || !IsHudActive())
    {
        return;
    }
    const float alpha = GetHudAlpha();
    if (alpha <= 0.0f)
    {
        return;
    }
    const auto drawList = ImGui::GetBackgroundDrawList();
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    DrawSharedHud(drawList, display, alpha);
    for (uint32_t i = 0; i < g_multiplayerPlayerCount; i++)
    {
        MultiplayerHudCounters counters{};
        if (GetMultiplayerHudCounters(i, counters))
        {
            DrawPlayerCounters(drawList, GetPlayerRect(i, display), counters, alpha);
        }
    }
}
