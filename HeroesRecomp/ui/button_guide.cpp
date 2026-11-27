#include "button_guide.h"
#include "imgui_utils.h"
#include <gpu/imgui/imgui_snapshot.h>
#include <gpu/video.h>
#include <input/input.h>
#include <patches/aspect_ratio_patches.h>
#include <user/config.h>
#include <decompressor.h>

#include <res/images/common/controllerBtn.png.h>

constexpr float DEFAULT_SIDE_MARGINS = 379;
float g_sideMargins = DEFAULT_SIDE_MARGINS;

constexpr float DEFAULT_TOP = 480.f - 65.0f;
float g_top = DEFAULT_TOP;
bool g_isCenterAnchored = false;
float g_leftAnchor = 0.0f;
float g_rightAnchor = 0.0f;

ImFont* g_fntDFPZong;

std::unique_ptr<GuestTexture> g_upControllerIcons;
std::unique_ptr<GuestTexture> g_upKBMIcons;

static std::vector<Button> g_buttons;

#define widthC 35
#define heightC 30
std::unordered_map<EButtonIcon, float> g_iconWidths =
{
    { EButtonIcon::A, widthC },
    { EButtonIcon::B, widthC },
    { EButtonIcon::X, widthC },
    { EButtonIcon::Y, widthC },
};

std::unordered_map<EButtonIcon, float> g_iconHeights =
{
    { EButtonIcon::A, heightC },
    { EButtonIcon::B, heightC },
    { EButtonIcon::X, heightC },
    { EButtonIcon::Y, heightC },
};

std::tuple<std::tuple<ImVec2, ImVec2>, GuestTexture*> GetButtonIcon(EButtonIcon icon)
{
    std::tuple<ImVec2, ImVec2> btn;
    GuestTexture* texture;

    const bool isAuto = Config::ControllerIcons.Value == EControllerIcons::Auto;
    const bool isGC = Config::ControllerIcons.Value == EControllerIcons::Gamecube;
    bool isXbox = Config::ControllerIcons.Value == EControllerIcons::Xbox || isAuto && input::g_inputDeviceController == input::EInputDevice::Xbox;

    if (isAuto && input::g_inputDeviceController == input::EInputDevice::Unknown)
    {
        // Default to Xbox icons if no controller is detected, as they are more universally recognised.
        isXbox = true;
	}

    auto xOffsetCmn = 0;
    auto yOffsetBtnY = 88;

    if (isGC)
    {
        xOffsetCmn = 40;
        yOffsetBtnY = 85;

    }

    if (isXbox)
    {
        xOffsetCmn = 80;
    }

    switch (icon)
    {
    case EButtonIcon::X:
        btn = PIXELS_TO_UV_COORDS(114, 120, xOffsetCmn, 0, 35, 30);
        texture = g_upControllerIcons.get();
        break;
    case EButtonIcon::A:
        btn = PIXELS_TO_UV_COORDS(114, 120, xOffsetCmn, 0 + 30, 35, 30);
        texture = g_upControllerIcons.get();
        break;
    case EButtonIcon::B:
        btn = PIXELS_TO_UV_COORDS(114, 120, xOffsetCmn, 0 + 60, 35, 30);
        texture = g_upControllerIcons.get();
        break;
    case EButtonIcon::Y:
        btn = PIXELS_TO_UV_COORDS(114, 120, xOffsetCmn, yOffsetBtnY, 35, 30);
        texture = g_upControllerIcons.get();
        break;
    default: ;
    }

    return std::make_tuple(btn, texture);
}

static void DrawGuide(float* offset, ImVec2 regionMin, ImVec2 regionMax, EButtonIcon icon,
    EButtonAlignment alignment, ImVec2 iconMin, ImVec2 iconMax,
    float textWidth, float maxTextWidth, float textScale, float fontSize, const char* text)
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto iconWidth = Scale(g_iconWidths[icon]);
    auto textMarginY = regionMin.y + Scale(9.0f);

    ImVec2 textPos;

    if (icon == EButtonIcon::LBRB || icon == EButtonIcon::LTRT)
    {
        auto iconMarginX = Scale(16);

        iconMin.x = alignment == EButtonAlignment::Left
            ? regionMin.x + *offset - maxTextWidth + (iconWidth / 2)
            : regionMax.x - *offset - maxTextWidth - (iconWidth / 2) - iconMarginX;

        iconMax.x = iconMin.x + iconWidth;

        // Left button.
        auto btnIcon = GetButtonIcon(icon == EButtonIcon::LBRB ? EButtonIcon::LB : EButtonIcon::LT);
        drawList->AddImage(std::get<1>(btnIcon), iconMin, iconMax, GET_UV_COORDS(std::get<0>(btnIcon)));

        auto dualIconMin = alignment == EButtonAlignment::Left
            ? ImVec2(iconMax.x + maxTextWidth + iconMarginX, iconMin.y)
            : ImVec2(regionMax.x - *offset + maxTextWidth - iconWidth, iconMin.y);

        auto dualIconMax = ImVec2(dualIconMin.x + iconWidth, iconMax.y);

        // Right button.
        btnIcon = GetButtonIcon(icon == EButtonIcon::LBRB ? EButtonIcon::RB : EButtonIcon::RT);
        drawList->AddImage(std::get<1>(btnIcon), dualIconMin, dualIconMax, GET_UV_COORDS(std::get<0>(btnIcon)));

        textPos = { (iconMax.x + ((dualIconMin.x - iconMax.x) - maxTextWidth + std::max(0.0f, maxTextWidth - textWidth)) / 2) + Scale(2), textMarginY };

        *offset += iconWidth;
    }
    else
    {
        auto btnIcon = GetButtonIcon(icon);

        drawList->AddImage(std::get<1>(btnIcon), iconMin, iconMax, GET_UV_COORDS(std::get<0>(btnIcon)));

        auto textMarginX = alignment == EButtonAlignment::Left
            ? regionMin.x + *offset
            : regionMax.x - *offset;

        textPos = { textMarginX, textMarginY };
    }

	textPos.y -= Scale(6.0f);
    SetScale({ textScale, 1.0f });
    SetOrigin(textPos);

    DrawTextWithOutline(g_fntDFPZong, fontSize, textPos, IM_COL32(204, 204, 204, 255), text, 4, IM_COL32_BLACK);

    SetScale({ 1.0f, 1.0f });
    SetOrigin({ 0.0f, 0.0f });
}

void ButtonGuide::Draw()
{
    if (!s_isVisible || s_alpha <= 0.0f)
        return;

    auto drawList = ImGui::GetBackgroundDrawList();
    auto vtxStart = drawList->VtxBuffer.Size;

    ImVec2 regionMin = { GetMenuBoxLeft() + GetMenuBoxWidth() * 0.1f, g_aspectRatioOffsetY * 2.0f + Scale(g_top)};
    ImVec2 regionMax = { g_aspectRatioOffsetX + Scale(640.f - g_sideMargins), g_aspectRatioOffsetY * 2.0f + Scale(480.f) };

    auto centreX = ImGui::GetIO().DisplaySize.x / 2.0f;

    if (g_isCenterAnchored)
        regionMin.x = centreX + Scale(g_leftAnchor);

    auto textMarginX = Scale(21.25f);
    auto iconMarginX = Scale(10.5f);
    auto fontSize = Scale(18.8f);

    auto offsetLeft = 0.0f;
    auto offsetRight = 0.0f;

    // Draw left aligned icons.
    for (int i = 0; i < g_buttons.size(); i++)
    {
        auto& btn = g_buttons[i];

        if (btn.Alignment != EButtonAlignment::Left)
            continue;

        if (btn.Visibility && !*btn.Visibility)
            continue;

        auto str = Localise(btn.Name).c_str();
        auto iconWidth = Scale(g_iconWidths[btn.Icon]);
        auto iconHeight = Scale(g_iconHeights[btn.Icon]);
        auto textWidth = g_fntDFPZong->CalcTextSizeA(fontSize, FLT_MAX, 0, str).x;
        auto maxWidth = btn.MaxWidth == FLT_MAX ? textWidth : Scale(btn.MaxWidth);
        auto textScale = std::min(1.0f, maxWidth / textWidth);

        if (i > 0)
            offsetLeft += maxWidth + iconWidth + textMarginX;

        ImVec2 iconMin = { regionMin.x + offsetLeft - iconWidth - iconMarginX, regionMin.y };
        ImVec2 iconMax = { regionMin.x + offsetLeft - iconMarginX, regionMin.y + iconHeight };

        DrawGuide(&offsetLeft, regionMin, regionMax, btn.Icon, btn.Alignment, iconMin, iconMax, textWidth, maxWidth, textScale, fontSize, str);
    }

    regionMin = { GetMenuBoxLeft() + GetMenuBoxWidth() * 0.6f, g_aspectRatioOffsetY * 2.0f + Scale(g_top) };
    regionMax = { GetMenuBoxLeft() + GetMenuBoxWidth() * 0.88f, g_aspectRatioOffsetY * 2.0f + Scale(480.f) };

    if (g_isCenterAnchored)
        regionMax.x = centreX + Scale(g_rightAnchor);
    
    // Draw right aligned icons.
    for (int i = g_buttons.size() - 1; i >= 0; i--)
    {
        auto& btn = g_buttons[i];

        if (btn.Alignment != EButtonAlignment::Right)
            continue;

        if (btn.Visibility && !*btn.Visibility)
            continue;

        auto str = Localise(btn.Name).c_str();
        auto iconWidth = Scale(g_iconWidths[btn.Icon]);
        auto iconHeight = Scale(g_iconHeights[btn.Icon]);
        auto textWidth = g_fntDFPZong->CalcTextSizeA(fontSize, FLT_MAX, 0, str).x + Scale(15.f);
        auto maxWidth = btn.MaxWidth == FLT_MAX ? textWidth : Scale(btn.MaxWidth);
        auto textScale = std::min(1.0f, maxWidth / textWidth);

  
        if (i < g_buttons.size() - 1)
            offsetRight += maxWidth + iconWidth + textMarginX;
      

        ImVec2 iconMin = { regionMax.x - offsetRight - iconWidth - iconMarginX, regionMin.y };
        ImVec2 iconMax = { regionMax.x - offsetRight - iconMarginX, regionMin.y + iconHeight };

        DrawGuide(&offsetRight, regionMin, regionMax, btn.Icon, btn.Alignment, iconMin, iconMax, textWidth, maxWidth, textScale, fontSize, str);
    }

    ScaleVertexAlpha(drawList, vtxStart, s_alpha);
}

void ButtonGuide::Init()
{
    auto& io = ImGui::GetIO();
    g_fntDFPZong = ImFontAtlasSnapshot::GetFont("DFPZongYiW5-B5.ttc");
    g_upControllerIcons = LOAD_ZSTD_TEXTURE(g_controller);
}

void ButtonGuide::Open(Button button)
{
    s_isVisible = true;
    g_sideMargins = DEFAULT_SIDE_MARGINS;
    g_top = DEFAULT_TOP;
    g_isCenterAnchored = false;

    g_buttons = {};
    g_buttons.push_back(button);
}

void ButtonGuide::Open(const std::span<Button> buttons)
{
    s_isVisible = true;
    g_sideMargins = DEFAULT_SIDE_MARGINS;
    g_top = DEFAULT_TOP;
    g_isCenterAnchored = false;
    g_buttons = std::vector(buttons.begin(), buttons.end());
}

void ButtonGuide::SetSideMargins(float width = DEFAULT_SIDE_MARGINS)
{
    g_sideMargins = width;
}

void ButtonGuide::SetLayout(float left, float right, float top)
{
    g_isCenterAnchored = true;
    g_leftAnchor = left;
    g_rightAnchor = right;
    g_top = top;
}

void ButtonGuide::Close()
{
    s_isVisible = false;
}
