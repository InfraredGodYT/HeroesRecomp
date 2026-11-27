#include "achievement_overlay.h"
#include <audio/embedded_player.h>
#include <locale/achievement_locale.h>
#include <locale/locale.h>
#include <ui/imgui_utils.h>
#include <user/achievement_data.h>
#include <user/config.h>
#include <user/achievement_manager.h>
#include <gpu/imgui/imgui_snapshot.h>
#include <res/images/achievements/overlay.png.h>
#include <decompressor.h>

constexpr double OVERLAY_DURATION = 5.0;

std::unique_ptr<GuestTexture> g_upTexWindow;

static bool g_isClosing{};
static double g_appearTime{};

static Achievement g_achievement{};
static ImFont* g_fntSeurat;

// Dequeue achievements only in the main thread. This is also extra thread safety.
static std::thread::id g_mainThreadId = std::this_thread::get_id();

static bool CanDequeueAchievement()
{
    return !AchievementOverlay::s_queue.empty();
}

static double DrawWindow(const ImVec2 min, const ImVec2 max, bool isAnimated, double time, bool isClosing)
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto motionTime = 1.0;

    auto _min = min;
    auto _max = max;

    if (isAnimated)
    {
        motionTime = ComputeLinearMotion(time, 0, 8, isClosing);

        auto centre = ImVec2{ min.x + ((max.x - min.x) / 2), min.y + ((max.y - min.y) / 2) };

        _min = Lerp(centre, min, motionTime);
        _max = Lerp(centre, max, motionTime);
    }

    constexpr auto containerTopLeftColour = IM_COL32(62, 14, 17, 225);
    constexpr auto containerTopRightColour = IM_COL32(34, 31, 33, 225);
    constexpr auto containerBottomRightColour = IM_COL32(10, 9, 10, 235);
    constexpr auto containerBottomLeftColour = IM_COL32(28, 7, 9, 235);
    constexpr auto accentColour = IM_COL32(178, 18, 24, 255);
    constexpr auto borderColour = IM_COL32(150, 16, 22, 200);

    drawList->AddRectFilledMultiColor(_min, _max, containerTopLeftColour, containerTopRightColour, containerBottomRightColour, containerBottomLeftColour);

    auto borderSize = std::max(1.0f, Scale(1));

    drawList->AddRectFilled(_min, { _max.x, _min.y + borderSize }, borderColour);
    drawList->AddRectFilled({ _min.x, _max.y - borderSize }, _max, borderColour);
    drawList->AddRectFilled(_min, { _min.x + Scale(4), _max.y }, accentColour);
    drawList->AddRectFilled({ _max.x - Scale(4), _min.y }, _max, accentColour);

    auto windowUV = PIXELS_TO_UV_COORDS(264, 64, 40, 5, 235, 60);



    // Window
    //drawList->AddImage(g_upTexWindow.get(), min, max, GET_UV_COORDS(windowUV));


    drawList->PushClipRect(_min, _max);

    return motionTime;
}

void AchievementOverlay::Draw(X86Context* ctx)
{
    if (!AchievementOverlay::s_isVisible && CanDequeueAchievement())
    {
        s_isVisible = true;
        g_isClosing = false;
        g_appearTime = ImGui::GetTime();
        g_achievement = GetAchievement(Config::Language.Value, s_queue.front());
        s_queue.pop();

        EmbeddedPlayer::Play("decide");
    }

    if (!s_isVisible)
        return;

    if (ImGui::GetTime() - g_appearTime >= OVERLAY_DURATION)
        AchievementOverlay::Close();

    auto drawList = ImGui::GetBackgroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    auto& strAchievementUnlocked = Localise("Achievements_Unlock");

    auto& strAchievementName = GetAchievementLocale(g_achievement.ID).Name;

    // Calculate text sizes.
    auto fontSize = Scale(Config::Language == ELanguage::Japanese ? 20 : 19);
    auto headerSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, strAchievementUnlocked.c_str());
    auto nameSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, strAchievementName.c_str());
    auto nameOffsetY = Scale(4);
    auto maxWidth = std::max(headerSize.x, nameSize.x) + Scale(4);
    auto maxHeight = headerSize.y + nameSize.y + nameOffsetY;

    // Calculate margins.
    auto imageMarginX = Scale(12);
    auto imageMarginY = Scale(8);
    auto imageSize = Scale(42);
    auto textMarginX = imageMarginX * 2 + imageSize - Scale(4);
    auto containerWidth = imageMarginX + textMarginX + maxWidth;
    auto containerHeight = Scale(72);

    ImVec2 min = { (res.x / 2) - (containerWidth / 2), Scale(40) };
    ImVec2 max = { min.x + containerWidth, min.y + containerHeight };

    // Calculate centred vertical text offset.
    auto textMarginY = (min.y + (containerHeight / 2)) - (maxHeight / 2);


    float windowMotion = DrawWindow(min, max, true, g_appearTime, g_isClosing);

    if (windowMotion >= 1.0)
    {
        // Draw achievement icon.
        drawList->AddImage
        (
            (GuestTexture*)g_achievement.pImageBuffer,
            { /* X */ min.x + imageMarginX, /* Y */ min.y + imageMarginY },
            { /* X */ min.x + imageMarginX + imageSize, /* Y */ min.y + imageMarginY + imageSize },
            { 0, 0 },
            { 1, 1 },
            IM_COL32(255, 255, 255, 255)
        );

        auto colourMotion = BREATHE_MOTION(1.0f, 0.0f, g_appearTime, 0.9f);
        auto headerColour = ColourLerp(IM_COL32(235, 230, 230, 255), IM_COL32(215, 30, 36, 255), colourMotion);

        // Draw header text.
        drawList->AddText
        (
            g_fntSeurat,
            fontSize,
            { /* X */ min.x + textMarginX + (maxWidth - headerSize.x) / 2, /* Y */ textMarginY },
            headerColour,
            strAchievementUnlocked.c_str()
        );

        // Draw achievement name.
        drawList->AddText
        (
            g_fntSeurat,
            fontSize,
            { /* X */ min.x + textMarginX + (maxWidth - nameSize.x) / 2, /* Y */ textMarginY + nameSize.y + nameOffsetY },
            IM_COL32(255, 255, 255, 255),
            strAchievementName.c_str()
        );
    }
    else if (windowMotion <= 0.0 && g_isClosing)
    {
        s_isVisible = false;
    }

    drawList->PopClipRect();
}

void AchievementOverlay::Open(int id)
{
    s_queue.push(id);
}

void AchievementOverlay::Close()
{
    if (!g_isClosing)
    {
        g_appearTime = ImGui::GetTime();
        g_isClosing = true;
    }

    if (CanDequeueAchievement())
        s_isVisible = false;
}

void AchievementOverlay::Init()
{

    g_fntSeurat = ImFontAtlasSnapshot::GetFont("FOT-SeuratPro-DB.otf");
    g_upTexWindow = LOAD_ZSTD_TEXTURE(g_overlayAchievements);
}