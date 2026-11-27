#pragma once

#include <map>
#include <gpu/imgui/imgui_common.h>
#include <gpu/video.h>

#define PIXELS_TO_UV_COORDS(textureWidth, textureHeight, x, y, width, height) \
    std::make_tuple(ImVec2((float)(x) / (float)(textureWidth), (float)(y) / (float)(textureHeight)), \
                    ImVec2(((float)(x) + (float)(width)) / (float)(textureWidth), ((float)(y) + (float)(height)) / (float)(textureHeight)))

#define GET_UV_COORDS(tuple) std::get<0>(tuple), std::get<1>(tuple)

#define CENTRE_TEXT_HORZ(min, max, textSize) min.x + ((max.x - min.x) - textSize.x) / 2
#define CENTRE_TEXT_VERT(min, max, textSize) min.y + ((max.y - min.y) - textSize.y) / 2

// this will be used to control speed of the pause zoom in/out effect and the logo blink effect
inline constexpr double g_pause_zoom_in_frames = 20.0;
inline constexpr double g_pause_zoom_out_frames = 20.0;
inline constexpr float g_pause_zoom_start_scale = 0.2f;
inline constexpr double g_logo_blink_close_frames = 19.0;

struct LoopKey
{
    float Frame;
    float Value;
};

struct PauseZoom
{
    ImVec2 Min;
    ImVec2 Max;
    float Alpha;
    bool IsDone;
};

#define BREATHE_MOTION(start, end, time, rate)Lerp(start, end, (sin((ImGui::GetTime() - time) * (2.0f * M_PI / rate)) + 1.0f) / 2.0f)

constexpr float ANNOTATION_FONT_SIZE_MODIFIER = 0.6f;
#define IM_COL32_WHITE_TRANS IM_COL32(255, 255, 255, 0)

extern float g_fntSeuratSize;


struct TextSegment {
    bool annotated;
    std::string text;
    std::string annotation;
};

struct Paragraph {
    bool annotated = false;
    std::vector<std::vector<TextSegment>> lines;
};

void InitImGuiUtils();
void UpdateFontSize();

void SetGradient(const ImVec2& min, const ImVec2& max, ImU32 top, ImU32 bottom);
void SetGradient(const ImVec2& min, const ImVec2& max, ImU32 topLeft, ImU32 topRight, ImU32 bottomRight, ImU32 bottomLeft);
void ResetGradient();
void SetShaderModifier(uint32_t shaderModifier);
void SetOrigin(ImVec2 origin);
void SetScale(ImVec2 scale);
void SetTextSkew(float yCenter, float skewScale);
void ResetTextSkew();
void SetHorizontalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScaleLeft, float fadeScaleRight);
void SetHorizontalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScale);
void SetVerticalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScaleTop, float fadeScaleBottom);
void SetVerticalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScale);
void ResetMarqueeFade();
void SetOutline(float outline);
void ResetOutline();
void SetProceduralOrigin(ImVec2 proceduralOrigin);
void ResetProceduralOrigin();
void SetAdditive(bool enabled);
void ResetAdditive();
float Scale(float size);
float GetMenuBoxLeft();
float GetMenuBoxWidth();
double ComputeLinearMotion(double time, double offset, double total, bool reverse = false);
double ComputeMotion(double duration, double offset, double total, bool reverse);
void DrawTextBasic(ImFont* font, float fontSize, const ImVec2& pos, ImU32 colour, const char* text);
void DrawTextWithMarquee(ImFont* font, float fontSize, const ImVec2& position, const ImVec2& min, const ImVec2& max, ImU32 color, const char* text, double time, double delay, double speed);
void DrawTextWithMarqueeShadow(ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 colour, const char* text, double time, double delay, double speed, float offset = 2.0f, float radius = 1.0f, ImU32 shadowColour = IM_COL32(0, 0, 0, 255));
void DrawTextWithOutline(ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, float outlineSize, ImU32 outlineColor, uint32_t shaderModifier = IMGUI_SHADER_MODIFIER_NONE);
void DrawTextWithShadow(ImFont* font, float fontSize, const ImVec2& pos, ImU32 colour, const char* text, float offset = 2.0f, float radius = 1.0f, ImU32 shadowColour = IM_COL32(0, 0, 0, 255));
float CalcWidestTextSize(ImFont* font, float fontSize, std::span<std::string> strs);
std::string Truncate(const std::string& input, size_t maxLength, bool useEllipsis = true, bool usePrefixEllipsis = false);
std::pair<std::string, std::map<std::string, std::string>> RemoveRubyAnnotations(const char* input);
std::string ReAddRubyAnnotations(const std::string_view& wrappedText, const std::map<std::string, std::string>& rubyMap);
std::vector<std::string> Split(const char* strStart, const ImFont* font, float fontSize, float maxWidth);
Paragraph CalculateAnnotatedParagraph(const std::vector<std::string>& lines);
std::vector<std::string> RemoveAnnotationFromParagraph(const std::vector<std::string>& lines);
std::string RemoveAnnotationFromParagraphLine(const std::vector<TextSegment>& annotatedLine);
ImVec2 MeasureCentredParagraph(ImFont* font, float fontSize, float lineMargin, const std::vector<std::string>& lines);
ImVec2 MeasureCentredParagraph(ImFont* font, float fontSize, float maxWidth, float lineMargin, const char* text);
void DrawRubyAnnotatedText(ImFont* font, float fontSize, float maxWidth, const ImVec2& pos, float lineMargin, const char* text, std::function<void(const char*, ImVec2)> drawMethod, std::function<void(const char*, float, ImVec2)> annotationDrawMethod, bool isCentred = false, bool leadingSpace = false);
float Lerp(float a, float b, float t);
float Cubic(float a, float b, float t);
float Hermite(float a, float b, float t);
ImVec2 Lerp(const ImVec2& a, const ImVec2& b, float t);
ImU32 ColourLerp(ImU32 c0, ImU32 c1, float t);
void ScaleVertexAlpha(ImDrawList* drawList, int vtxStart, float alpha);
void DrawVersionString(ImFont* font, const ImU32 col = IM_COL32(255, 255, 255, 70));

const char* CalcWordWrapPositionA(const ImFont* font, float scale, const char* text, const char* text_end, float wrap_width);
double ComputeEaseOut(double time, double duration, bool reverse);
PauseZoom ComputePauseZoom(double time, bool isClosing);
float GetInterpolatedFrameValue(std::span<const LoopKey> keys, float frame, bool isLooping = true);
float ComputeTitleBlink(double time, bool isClosing);
void SetVerticalGradient(const ImVec2& min, const ImVec2& max, ImU32 top, ImU32 bottom);

struct ScrollArrows
{
    double TopMotionTime{};
    double BottomMotionTime{};
    float TopAlpha{};
    float BottomAlpha{};
};

void DrawScrollArrows(ImVec2 min, ImVec2 max, float scale, ScrollArrows& arrows, bool top, bool bottom);
double ComputeLoopMotion(double time, double offset, double total);
void DrawArrowCursor(ImVec2 pos, double time, bool isIntroAnim, bool isBlinkingAnim, bool isReversed);
GuestTexture* GetTexArrow();
void DrawTextParagraph(ImFont* font, float fontSize, float maxWidth, const ImVec2& pos, float lineMargin, const char* text, std::function<void(const char*, ImVec2)> drawMethod, bool isCentred = false);
void AddTextWithOutline(ImDrawList* drawList, ImFont* font, float fontSize,
    ImVec2 pos, ImU32 textColor, ImU32 outlineColor,
    const char* text, float outlineSize = 2.0f, float outlineOffsetX = 0.0f, float outlineOffsetY = 0.0f);

void AddTextCentered(ImDrawList* drawList, ImFont* font, float fontSize,
    float centerX, float y, ImU32 textColor, ImU32 outlineColor,
    const char* text);

void DrawScaledRect(ImDrawList* drawList, ImVec2 winPos, float scale, float offsetX, float offsetY, float baseX, float baseY, float baseW, float baseH, ImU32 colour);

// advances the message box open (0 -> 1) or close (1 -> 0) anim
float StepOpenProgress(float progress, bool isClosing);

// installer style blue window, min/max is the outer bevel, scale is one pixel at shadow's 640x480
// openProgress grows it out from the centre
void DrawShadowWindow(ImDrawList* drawList, ImVec2 min, ImVec2 max, float scale, float openProgress = 1.0f);

