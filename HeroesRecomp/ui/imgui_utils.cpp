#include "imgui_utils.h"
#include <map>
#include <decompressor.h>
#include <version.h>
#include <user/config.h>
#include <patches/aspect_ratio_patches.h>
#include <res/images/common/arrow.png.h>

float g_fntSeuratSize = 0.0f;

std::unique_ptr<GuestTexture> g_upTexArrow;

GuestTexture* GetTexArrow()
{
    return g_upTexArrow.get();
}

void UpdateFontSize()
{
    g_fntSeuratSize = Scale(Config::Language == ELanguage::Japanese ? 19 : 18);
}

void InitImGuiUtils()
{
    g_upTexArrow = LOAD_ZSTD_TEXTURE(g_arrow);

    UpdateFontSize();
}


void SetGradient(const ImVec2& min, const ImVec2& max, ImU32 top, ImU32 bottom)
{
    SetGradient(min, max, top, top, bottom, bottom);
}

void SetGradient(const ImVec2& min, const ImVec2& max, ImU32 topLeft, ImU32 topRight, ImU32 bottomRight, ImU32 bottomLeft)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetGradient);
    callbackData->setGradient.boundsMin[0] = min.x;
    callbackData->setGradient.boundsMin[1] = min.y;
    callbackData->setGradient.boundsMax[0] = max.x;
    callbackData->setGradient.boundsMax[1] = max.y;
    callbackData->setGradient.gradientTopLeft = topLeft;
    callbackData->setGradient.gradientTopRight = topRight;
    callbackData->setGradient.gradientBottomRight = bottomRight;
    callbackData->setGradient.gradientBottomLeft = bottomLeft;
}

void ResetGradient()
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetGradient);
    memset(&callbackData->setGradient, 0, sizeof(callbackData->setGradient));
}

void SetShaderModifier(uint32_t shaderModifier)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetShaderModifier);
    callbackData->setShaderModifier.shaderModifier = shaderModifier;
}

void SetOrigin(ImVec2 origin)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetOrigin);
    callbackData->setOrigin.origin[0] = origin.x;
    callbackData->setOrigin.origin[1] = origin.y;
}

void SetScale(ImVec2 scale)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetScale);
    callbackData->setScale.scale[0] = scale.x;
    callbackData->setScale.scale[1] = scale.y;
}

void SetTextSkew(float yCenter, float skewScale)
{
    SetShaderModifier(IMGUI_SHADER_MODIFIER_TEXT_SKEW);
    SetOrigin({ 0.0f, yCenter });
    SetScale({ skewScale, 1.0f });
}

void ResetTextSkew()
{
    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
    SetOrigin({ 0.0f, 0.0f });
    SetScale({ 1.0f, 1.0f });
}

void SetHorizontalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScaleLeft, float fadeScaleRight)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetMarqueeFade);
    callbackData->setMarqueeFade.boundsMin[0] = min.x;
    callbackData->setMarqueeFade.boundsMin[1] = min.y;
    callbackData->setMarqueeFade.boundsMax[0] = max.x;
    callbackData->setMarqueeFade.boundsMax[1] = max.y;

    SetShaderModifier(IMGUI_SHADER_MODIFIER_HORIZONTAL_MARQUEE_FADE);
    SetScale({ fadeScaleLeft, fadeScaleRight });
}

void SetHorizontalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScale)
{
    SetHorizontalMarqueeFade(min, max, fadeScale, fadeScale);
}

void SetVerticalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScaleTop, float fadeScaleBottom)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetMarqueeFade);
    callbackData->setMarqueeFade.boundsMin[0] = min.x;
    callbackData->setMarqueeFade.boundsMin[1] = min.y;
    callbackData->setMarqueeFade.boundsMax[0] = max.x;
    callbackData->setMarqueeFade.boundsMax[1] = max.y;

    SetShaderModifier(IMGUI_SHADER_MODIFIER_VERTICAL_MARQUEE_FADE);
    SetScale({ fadeScaleTop, fadeScaleBottom });
}

void SetVerticalMarqueeFade(ImVec2 min, ImVec2 max, float fadeScale)
{
    SetVerticalMarqueeFade(min, max, fadeScale, fadeScale);
}

void ResetMarqueeFade()
{
    ResetGradient();
    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
    SetScale({ 1.0f, 1.0f });
}

void SetOutline(float outline)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetOutline);
    callbackData->setOutline.outline = outline;
}

void ResetOutline()
{
    SetOutline(0.0f);
}

void SetProceduralOrigin(ImVec2 proceduralOrigin)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetProceduralOrigin);
    callbackData->setProceduralOrigin.proceduralOrigin[0] = proceduralOrigin.x;
    callbackData->setProceduralOrigin.proceduralOrigin[1] = proceduralOrigin.y;
}

void ResetProceduralOrigin()
{
    SetProceduralOrigin({ 0.0f, 0.0f });
}

void SetAdditive(bool enabled)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetAdditive);
    callbackData->setAdditive.enabled = enabled;
}

void ResetAdditive()
{
    SetAdditive(false);
}

float Scale(float size)
{
    return size * g_aspectRatioScale;
}


float GetMenuBoxLeft()
{
    return std::max(0.0f, g_aspectRatioOffsetX);
}

float GetMenuBoxWidth()
{
    return ImGui::GetIO().DisplaySize.x - GetMenuBoxLeft() * 2.0f;
}

double ComputeLoopMotion(double time, double offset, double total)
{
    return std::clamp(fmod((ImGui::GetTime() - time - (offset / 60.0)) / (total / 60.0), 1.0 + (total / 60.0)), 0.0, 1.0) / 1.0;
}

double ComputeLinearMotion(double time, double offset, double total, bool reverse)
{
    auto result = std::clamp((ImGui::GetTime() - time - offset / 60.0) / total * 60.0, 0.0, 1.0);

    return reverse ? 1.0 - result : result;
}

double ComputeMotion(double duration, double offset, double total, bool reverse)
{
    return sqrt(ComputeLinearMotion(duration, offset, total, reverse));
}

double ComputeEaseOut(double time, double duration, bool reverse)
{
    double t = std::clamp((ImGui::GetTime() - time) / duration, 0.0, 1.0);
    double result = 1.0 - std::pow(1.0 - t, 3.0); // cubic ease out
    return reverse ? 1.0 - result : result;
}

// process the pause zoom effect, set the min and max coordinates for the zoom area, as well as the alpha value for the fade effect.
PauseZoom ComputePauseZoom(double time, bool isClosing)
{
    auto& res = ImGui::GetIO().DisplaySize;
    auto frames = isClosing ? g_pause_zoom_out_frames : g_pause_zoom_in_frames;
    auto progress = (float)std::clamp((ImGui::GetTime() - time) * 60.0 / frames, 0.0, 1.0);
    auto t = isClosing ? 1.0f - progress : progress;
    auto scale = Lerp(g_pause_zoom_start_scale, 1.0f, t);

    ImVec2 centre = { res.x / 2.0f, res.y / 2.0f };
    ImVec2 halfSize = { centre.x * scale, centre.y * scale };

    PauseZoom pauseZoom;
	pauseZoom.Min = { centre.x - halfSize.x, centre.y - halfSize.y };
	pauseZoom.Max = { centre.x + halfSize.x, centre.y + halfSize.y };
	pauseZoom.Alpha = t;
	pauseZoom.IsDone = progress >= 1.0f;
    return pauseZoom;
}

// general keyframe reader for the looped fire and line animations, return the interpolated value 
// for the given frame.
// This is essentially what the game does with CSD, but in a simpler and smaller way.
float GetInterpolatedFrameValue(std::span<const LoopKey> keys, float frame, bool isLooping)
{
    auto lastFrame = keys.back().Frame;

    if (isLooping && lastFrame > 0.0f)
        frame = fmodf(frame, lastFrame);

    if (frame < keys.front().Frame)
        return keys.front().Value;

    for (size_t i = 1; i < keys.size(); i++)
    {
        if (frame < keys[i].Frame)
        {
            auto& prev = keys[i - 1];
            auto& next = keys[i]; // blend linearly
            return prev.Value + (next.Value - prev.Value) * (frame - prev.Frame) / (next.Frame - prev.Frame);
        }
    }

    return keys.back().Value;
}

// Helper function to compute the alpha value to create a flick effect for title / logo.
// Originally the game has an alpha flick effect for title like the "OPTIONS" text on the left and other menu stuff.
float ComputeTitleBlink(double time, bool isClosing)
{
    static constexpr LoopKey openKeys[] = { { 0, 0 }, { 6, 93 }, { 12, 38 }, { 18, 152 } };
    static constexpr LoopKey closeKeys[] = { { 0, 120 }, { 7, 39 }, { 13, 88 }, { 19, 0 } };

    auto frame = (float)((ImGui::GetTime() - time) * 60.0);
    auto value = GetInterpolatedFrameValue(isClosing ? closeKeys : openKeys, frame, false);

    return value / 152.0f;
}

void DrawTextBasic(ImFont* font, float fontSize, const ImVec2& pos, ImU32 colour, const char* text)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    drawList->AddText(font, fontSize, pos, colour, text, nullptr);
}

void DrawTextWithMarquee(ImFont* font, float fontSize, const ImVec2& position, const ImVec2& min, const ImVec2& max, ImU32 color, const char* text, double time, double delay, double speed)
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto rectWidth = max.x - min.x;
    auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, text);
    auto textX = position.x - fmodf(std::max(0.0, ImGui::GetTime() - (time + delay)) * speed, textSize.x + rectWidth);

    drawList->PushClipRect(min, max, true);

    if (textX <= position.x)
    {
        DrawRubyAnnotatedText
        (
            font,
            fontSize,
            FLT_MAX,
            { textX, position.y },
            0.0f,
            text,
            [=](const char* str, ImVec2 pos)
            {
                DrawTextBasic(font, fontSize, pos, color, str);
            },
            [=](const char* str, float size, ImVec2 pos)
            {
                DrawTextBasic(font, size, pos, color, str);
            }
        );
    }

    if (textX + textSize.x < position.x)
    {
        DrawRubyAnnotatedText
        (
            font,
            fontSize,
            FLT_MAX,
            { textX + textSize.x + rectWidth, position.y },
            0.0f,
            text,
            [=](const char* str, ImVec2 pos)
            {
                DrawTextBasic(font, fontSize, pos, color, str);
            },
            [=](const char* str, float size, ImVec2 pos)
            {
                DrawTextBasic(font, size, pos, color, str);
            }
        );
    }

    drawList->PopClipRect();
}

void DrawTextWithMarqueeShadow(ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 colour, const char* text, double time, double delay, double speed, float offset, float radius, ImU32 shadowColour)
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto rectWidth = max.x - min.x;
    auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, text);
    auto textX = pos.x - fmodf(std::max(0.0, ImGui::GetTime() - (time + delay)) * speed, textSize.x + rectWidth);

    drawList->PushClipRect(min, max, true);

    if (textX <= pos.x)
        DrawTextWithShadow(font, fontSize, { textX, pos.y }, colour, text, offset, radius, shadowColour);

    if (textX + textSize.x < pos.x)
        DrawTextWithShadow(font, fontSize, { textX + textSize.x + rectWidth, pos.y }, colour, text, offset, radius, shadowColour);

    drawList->PopClipRect();
}

void DrawTextWithOutline(ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, float outlineSize, ImU32 outlineColor, uint32_t shaderModifier)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    SetOutline(outlineSize);
    drawList->AddText(font, fontSize, pos, outlineColor, text);
    ResetOutline();

    if (shaderModifier != IMGUI_SHADER_MODIFIER_NONE)
        SetShaderModifier(shaderModifier);

    drawList->AddText(font, fontSize, pos, color, text);

    if (shaderModifier != IMGUI_SHADER_MODIFIER_NONE)
        SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
}

void DrawTextWithShadow(ImFont* font, float fontSize, const ImVec2& pos, ImU32 colour, const char* text, float offset, float radius, ImU32 shadowColour)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    offset = Scale(offset);

    SetOutline(radius);
    drawList->AddText(font, fontSize, { pos.x + offset, pos.y + offset }, shadowColour, text);
    ResetOutline();

    drawList->AddText(font, fontSize, pos, colour, text, nullptr);
}

float CalcWidestTextSize(ImFont* font, float fontSize, std::span<std::string> strs)
{
    auto result = 0.0f;

    for (auto& str : strs)
        result = std::max(result, font->CalcTextSizeA(fontSize, FLT_MAX, 0, str.c_str()).x);

    return result;
}

void DrawTextParagraph(ImFont* font, float fontSize, float maxWidth, const ImVec2& pos, float lineMargin, const char* text, std::function<void(const char*, ImVec2)> drawMethod, bool isCentred)
{
    auto lines = Split(text, font, fontSize, maxWidth);
    auto paragraphSize = MeasureCentredParagraph(font, fontSize, lineMargin, lines);
    auto offsetY = 0.0f;

    for (auto& line : lines)
    {
        auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, line.c_str());
        auto textX = pos.x;
        auto textY = pos.y + offsetY;

        if (isCentred)
        {
            textX = line.starts_with("- ")
                ? pos.x - paragraphSize.x / 2
                : pos.x - textSize.x / 2;

            textY = pos.y - paragraphSize.y / 2 + offsetY;
        }

        drawMethod(line.c_str(), { textX, textY });

        textX += textSize.x;
        offsetY += textSize.y + Scale(lineMargin);
    }
}
std::string Truncate(const std::string& input, size_t maxLength, bool useEllipsis, bool usePrefixEllipsis)
{
    const std::string ellipsis = "...";

    if (input.length() > maxLength)
    {
        if (useEllipsis && maxLength > ellipsis.length())
        {
            if (usePrefixEllipsis)
            {
                return ellipsis + input.substr(0, maxLength - ellipsis.length());
            }
            else
            {
                return input.substr(0, maxLength - ellipsis.length()) + ellipsis;
            }
        }
        else
        {
            return input.substr(0, maxLength);
        }
    }

    return input;
}

std::pair<std::string, std::map<std::string, std::string>> RemoveRubyAnnotations(const char* input)
{
    std::string output;
    std::map<std::string, std::string> rubyMap;
    std::string currentMain, currentRuby;
    size_t idx = 0;

    while (input[idx] != '\0')
    {
        if (input[idx] == '[')
        {
            idx++;
            currentMain.clear();
            currentRuby.clear();

            while (input[idx] != ':' && input[idx] != ']' && input[idx] != '\0')
            {
                currentMain += input[idx++];
            }
            if (input[idx] == ':')
            {
                idx++;
                while (input[idx] != ']' && input[idx] != '\0')
                {
                    currentRuby += input[idx++];
                }
            }
            if (input[idx] == ']') 
            {
                idx++;
            }

            if (!currentMain.empty() && !currentRuby.empty())
            {
                rubyMap[currentMain] = currentRuby;
            }

            output += currentMain;
        }
        else
        {
            output += input[idx++];
        }
    }

    return { output, rubyMap };
}

std::string ReAddRubyAnnotations(const std::string_view& wrappedText, const std::map<std::string, std::string>& rubyMap)
{
    std::string annotatedText;
    size_t idx = 0;
    size_t length = wrappedText.length();

    while (idx < length) {
        bool matched = false;
        for (const auto& [mainText, rubyText] : rubyMap)
        {
            if (wrappedText.substr(idx, mainText.length()) == mainText) 
            {
                annotatedText += "[";
                annotatedText += mainText;
                annotatedText += ":";
                annotatedText += rubyText;
                annotatedText += "]";

                idx += mainText.length();
                matched = true;
                break;
            }
        }
        if (!matched)
        {
            annotatedText += wrappedText[idx++];
        }
    }

    return annotatedText;
}

std::vector<std::string> Split(const char* strStart, const ImFont* font, float fontSize, float maxWidth)
{
    if (!strStart)
        return {};

    std::vector<std::string> result;
    float textWidth = 0.0f;
    float lineWidth = 0.0f;
    const float scale = fontSize / font->FontSize;
    const char *str = strStart;
    const char *strEnd = strStart + strlen(strStart);
    const char *lineStart = strStart;
    const bool wordWrapEnabled = (maxWidth > 0.0f);
    const char *wordWrapEOL = nullptr;

    auto IsKanji = [](const char* str, const char* strEnd)
    {
        const char* tempStr = str;
        unsigned int c = (unsigned int)*tempStr;
        if (c < 0x80)
            tempStr += 1;
        else
            tempStr += ImTextCharFromUtf8(&c, tempStr, strEnd);

        // Basic CJK and CJK Extension A
        return (c >= 0x4E00 && c <= 0x9FBF) || (c >= 0x3400 && c <= 0x4DBF);
    };

    while (*str != 0) 
    {
        if (wordWrapEnabled)
        {
            if (wordWrapEOL == nullptr)
            {
                wordWrapEOL = CalcWordWrapPositionA(font, scale, str, strEnd, maxWidth - lineWidth);
            }

            if (str >= wordWrapEOL)
            {
                if (IsKanji(str, strEnd))
                {
                    // If the current character is Kanji, move back to prevent splitting Kanji
                    while (str > lineStart && IsKanji(str - 3, strEnd))
                    {
                        str -= 3;
                    }
                }

                if (textWidth < lineWidth)
                    textWidth = lineWidth;

                result.emplace_back(lineStart, str);
                lineWidth = 0.0f;
                wordWrapEOL = nullptr;

                while (str < strEnd && ImCharIsBlankA(*str))
                    str++;

                if (*str == '\n')
                    str++;

                if (strncmp(str, "\u200B", 3) == 0)
                {
                    str += 3;
                }

                lineStart = str;
                continue;
            }
        }

        const char *prevStr = str;
        unsigned int c = (unsigned int)*str;
        if (c < 0x80)
            str += 1;
        else
            str += ImTextCharFromUtf8(&c, str, strEnd);

        if (c < 32)
        {
            if (c == '\n')
            {
                result.emplace_back(lineStart, str - 1);
                lineStart = str;
                textWidth = ImMax(textWidth, lineWidth);
                lineWidth = 0.0f;
                continue;
            }

            if (c == '\r')
            {
                lineStart = str;
                continue;
            }
        }

        const float charWidth = ((int)c < font->IndexAdvanceX.Size ? font->IndexAdvanceX.Data[c] : font->FallbackAdvanceX) * scale;
        if (lineWidth + charWidth >= maxWidth)
        {
            str = prevStr;
            break;
        }

        lineWidth += charWidth;
    }

    if (str != lineStart) 
    {
        result.emplace_back(lineStart, str);
    }

    return result;
}

Paragraph CalculateAnnotatedParagraph(const std::vector<std::string>& lines)
{
    Paragraph result;

    for (const auto& line : lines)
    {
        std::vector<TextSegment> segments;

        size_t pos = 0;
        size_t start = 0;

        while ((start = line.find('[', pos)) != std::string::npos)
        {
            size_t end = line.find(']', start);
            if (end == std::string::npos)
                break;

            size_t colon = line.find(':', start);
            if (colon != std::string::npos && colon < end)
            {
                if (start != pos)
                {
                    segments.push_back({ false, line.substr(pos, start - pos), "" });
                }

                segments.push_back({ true, line.substr(start + 1, colon - start - 1), line.substr(colon + 1, end - colon - 1) });

                result.annotated = true;
                pos = end + 1;
            }
            else
            {
                pos = start + 1;
            }
        }

        if (pos < line.size())
        {
            segments.push_back({ false, line.substr(pos), "" });
        }

        result.lines.push_back(segments);
    }

    return result;
}

std::vector<std::string> RemoveAnnotationFromParagraph(const std::vector<std::string>& lines)
{
    std::vector<std::string> result;
    const auto paragraph = CalculateAnnotatedParagraph(lines);

    for (auto& annotatedLine : paragraph.lines)
    {
        std::string annotationRemovedLine = "";

        for (const auto& segment : annotatedLine)
            annotationRemovedLine += segment.text;

        result.push_back(annotationRemovedLine);
    }

    return result;
}

std::string RemoveAnnotationFromParagraphLine(const std::vector<TextSegment>& annotatedLine)
{
    std::string result = "";

    for (auto& segment : annotatedLine)
        result += segment.text;

    return result;
}

ImVec2 MeasureCentredParagraph(ImFont* font, float fontSize, float lineMargin, const std::vector<std::string>& lines)
{
    auto x = 0.0f;
    auto y = 0.0f;

    const auto paragraph = CalculateAnnotatedParagraph(lines);
    
    std::vector<std::string> annotationRemovedLines;

    for (const auto& line : paragraph.lines)
        annotationRemovedLines.emplace_back(RemoveAnnotationFromParagraphLine(line));

    for (size_t i = 0; i < annotationRemovedLines.size(); i++)
    {
        auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, annotationRemovedLines[i].c_str());

        x = std::max(x, textSize.x);
        y += textSize.y + Scale(lineMargin);

        if (paragraph.annotated && i != (annotationRemovedLines.size() - 1))
            y += fontSize * ANNOTATION_FONT_SIZE_MODIFIER;
    }

    return { x, y };
}

ImVec2 MeasureCentredParagraph(ImFont* font, float fontSize, float maxWidth, float lineMargin, const char* text)
{
    const auto input = RemoveRubyAnnotations(text);
    auto lines = Split(input.first.c_str(), font, fontSize, maxWidth);

    for (auto& line : lines)
        line = ReAddRubyAnnotations(line, input.second);

    return MeasureCentredParagraph(font, fontSize, lineMargin, lines);
}

void DrawRubyAnnotatedText(ImFont* font, float fontSize, float maxWidth, const ImVec2& pos, float lineMargin, const char* text, std::function<void(const char*, ImVec2)> drawMethod, std::function<void(const char*, float, ImVec2)> annotationDrawMethod, bool isCentred, bool leadingSpace)
{
    auto annotationFontSize = fontSize * ANNOTATION_FONT_SIZE_MODIFIER;

    const auto input = RemoveRubyAnnotations(text);
    auto lines = Split(input.first.c_str(), font, fontSize, maxWidth);

    for (auto& line : lines)
    {
        line = ReAddRubyAnnotations(line, input.second);
        if (!line.empty() && line.substr(0, 3) != "「" && leadingSpace)
        {
            line.insert(0, " ");
        }
    }

    auto paragraphSize = MeasureCentredParagraph(font, fontSize, lineMargin, lines);
    auto offsetY = 0.0f;

    const auto paragraph = CalculateAnnotatedParagraph(lines);

    for (const auto& annotatedLine : paragraph.lines)
    {
        const auto annotationRemovedLine = RemoveAnnotationFromParagraphLine(annotatedLine);

        auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, annotationRemovedLine.c_str());
        auto annotationSize = font->CalcTextSizeA(annotationFontSize, FLT_MAX, 0, "");
        auto textX = pos.x;
        auto textY = pos.y + offsetY;

        if (isCentred)
        {
            textX = annotationRemovedLine.starts_with("- ")
                ? pos.x - paragraphSize.x / 2
                : pos.x - textSize.x / 2;

            textY = pos.y - paragraphSize.y / 2 + offsetY;
        }

        for (const auto& segment : annotatedLine)
        {
            textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, segment.text.c_str());

            if (segment.annotated)
            {
                annotationSize = font->CalcTextSizeA(annotationFontSize, FLT_MAX, 0, segment.annotation.c_str());
                float annotationX = textX + (textSize.x - annotationSize.x) / 2.0f;

                annotationDrawMethod(segment.annotation.c_str(), annotationFontSize, { annotationX, textY - annotationFontSize });
            }

            drawMethod(segment.text.c_str(), { textX, textY });
            
            textX += textSize.x;
        }

        offsetY += textSize.y + Scale(lineMargin);

        if (paragraph.annotated)
            offsetY += annotationSize.y;
    }
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float Cubic(float a, float b, float t)
{
    return a + (b - a) * (t * t * t);
}

float Hermite(float a, float b, float t)
{
    return a + (b - a) * (t * t * (3 - 2 * t));
}

ImVec2 Lerp(const ImVec2& a, const ImVec2& b, float t)
{
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

ImU32 ColourLerp(ImU32 c0, ImU32 c1, float t)
{
    auto a = ImGui::ColorConvertU32ToFloat4(c0);
    auto b = ImGui::ColorConvertU32ToFloat4(c1);

    ImVec4 result;
    result.x = a.x + (b.x - a.x) * t;
    result.y = a.y + (b.y - a.y) * t;
    result.z = a.z + (b.z - a.z) * t;
    result.w = a.w + (b.w - a.w) * t;

    return ImGui::ColorConvertFloat4ToU32(result);
}

// Fades everything drawn since vtxStart.
// (useful to fade multiple draws at once)
void ScaleVertexAlpha(ImDrawList* drawList, int vtxStart, float alpha)
{
    if (alpha >= 1.0f)
        return;

    alpha = std::max(alpha, 0.0f);

    for (int i = vtxStart; i < drawList->VtxBuffer.Size; i++)
    {
        auto& col = drawList->VtxBuffer[i].col;
        auto a = (col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT;

        col = (col & ~IM_COL32_A_MASK) | ((ImU32)(a * alpha) << IM_COL32_A_SHIFT);
    }
}

void DrawVersionString(ImFont* font, const ImU32 col)
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;
    auto fontSize = Scale(12);
    auto textMargin = Scale(2);
    auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, g_versionString);

    drawList->AddText(font, fontSize, { res.x - textSize.x - textMargin, res.y - textSize.y - textMargin }, col, g_versionString);
}


// Taken from ImGui because we need to modify to break for '\u200B\ too
// Simple word-wrapping for English, not full-featured. Please submit failing cases!
// This will return the next location to wrap from. If no wrapping if necessary, this will fast-forward to e.g. text_end.
// FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)
const char* CalcWordWrapPositionA(const ImFont* font, float scale, const char* text, const char* text_end, float wrap_width)
{
    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"
    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    IM_ASSERT(text_end != NULL);
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < font->IndexAdvanceX.Size ? font->IndexAdvanceX.Data[c] : font->FallbackAdvanceX);
        if (ImCharIsBlankW(c) || c == 0x200B)
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = (c != '.' && c != ',' && c != ';' && c != '!' && c != '?' && c != '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width > wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
    // +1 may not be a character start point in UTF-8 but it's ok because caller loops use (text >= word_wrap_eol).
    if (s == text && text < text_end)
        return s + 1;
    return s;
}

void SetVerticalGradient(const ImVec2& min, const ImVec2& max, ImU32 top, ImU32 bottom)
{
    SetGradient(min, max, top, top, bottom, bottom);
}

void AddImageFlipped(ImTextureID texture, const ImVec2& min, const ImVec2& max, const ImVec2& uvMin, const ImVec2& uvMax, ImU32 col, bool flipHorz, bool flipVert)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    ImVec2 uv0 = uvMin;
    ImVec2 uv1 = { uvMax.x, uvMin.y };
    ImVec2 uv2 = uvMax;
    ImVec2 uv3 = { uvMin.x, uvMax.y };

    if (flipHorz)
    {
        std::swap(uv0.x, uv1.x);
        std::swap(uv2.x, uv3.x);
    }

    if (flipVert)
    {
        std::swap(uv0.y, uv3.y);
        std::swap(uv1.y, uv2.y);
    }

    ImVec2 p0 = min;
    ImVec2 p1 = { max.x, min.y };
    ImVec2 p2 = max;
    ImVec2 p3 = { min.x, max.y };

    drawList->AddImageQuad(texture, p0, p1, p2, p3, uv0, uv1, uv2, uv3, col);
}

void DrawScrollArrows(ImVec2 min, ImVec2 max, float scale, ScrollArrows& arrows, bool top, bool bottom)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    auto step = ImGui::GetIO().DeltaTime * 60.0f / 8.0f;
    auto topMotion = ComputeLinearMotion(arrows.TopMotionTime, 0, 15);
    auto bottomMotion = ComputeLinearMotion(arrows.BottomMotionTime, 0, 15);

    // Fade out arrows.
    if (top)
    {
        arrows.TopAlpha = std::min(arrows.TopAlpha + step, 1.0f);
    }
    else if (topMotion >= 1.0)
    {
        arrows.TopAlpha = std::max(arrows.TopAlpha - step, 0.0f);
    }

    if (bottom)
    {
        arrows.BottomAlpha = std::min(arrows.BottomAlpha + step, 1.0f);
    }
    else if (bottomMotion >= 1.0)
    {
        arrows.BottomAlpha = std::max(arrows.BottomAlpha - step, 0.0f);
    }

    auto topOffsetY = Lerp(0.0f, Scale(9), sin(topMotion * M_PI));
    auto bottomOffsetY = Lerp(0.0f, Scale(9), sin(bottomMotion * M_PI));

    ImVec2 scrollArrowTopMin = { min.x, min.y - topOffsetY };
    ImVec2 scrollArrowTopMax = { scrollArrowTopMin.x + scale, scrollArrowTopMin.y + scale };
    ImVec2 scrollArrowBottomMin = { min.x, max.y - scale + bottomOffsetY };
    ImVec2 scrollArrowBottomMax = { scrollArrowBottomMin.x + scale, scrollArrowBottomMin.y + scale };

    auto scrollArrowUVs = PIXELS_TO_UV_COORDS(48, 48, 0, 24, 24, 24);
    drawList->AddImage(g_upTexArrow.get(), scrollArrowTopMin, scrollArrowTopMax, GET_UV_COORDS(scrollArrowUVs), IM_COL32(255, 255, 255, 255 * arrows.TopAlpha));

    scrollArrowUVs = PIXELS_TO_UV_COORDS(48, 48, 24, 24, 24, 24);
    drawList->AddImage(g_upTexArrow.get(), scrollArrowBottomMin, scrollArrowBottomMax, GET_UV_COORDS(scrollArrowUVs), IM_COL32(255, 255, 255, 255 * arrows.BottomAlpha));
}

void DrawArrowCursor(ImVec2 pos, double time, bool isIntroAnim, bool isBlinkingAnim, bool isReversed)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    auto cursorUVs = PIXELS_TO_UV_COORDS(48, 48, 0, 24, 24, 24);
    auto cursorScaleX = Scale(14);

    for (int i = 0; i < 3; i++)
    {
        auto cursorMotionTime = isIntroAnim ? ComputeLinearMotion(time, i, 5, isReversed) : 1.0;
        auto cursorScaleYMotion = Lerp(Scale(37.5), Scale(24), cursorMotionTime);
        auto cursorOffsetXMotion = Lerp(0, Scale(8.5), cursorMotionTime);
        auto cursorOffsetYMotion = Lerp(pos.y - cursorScaleYMotion / 6, pos.y, cursorMotionTime);
        auto cursorRightMotion = (cursorOffsetXMotion * 3) - (cursorOffsetXMotion * i);
        auto cursorAlphaMotionTime = isIntroAnim ? ComputeLinearMotion(time, i, 2, isReversed) : 1.0;
        auto cursorAlphaMotion = (int)Lerp(0, 255, cursorAlphaMotionTime);

        if (isBlinkingAnim)
        {
            auto cursorAlphaMotionInTime = ComputeLoopMotion(time, 3.0 * i, 12.0);
            auto cursorAlphaMotionOutTime = ComputeLoopMotion(time, 3.0 * (i + 1), 12.0);

            // horrible
            cursorAlphaMotionTime = cursorAlphaMotionInTime >= 1.0
                ? cursorAlphaMotionOutTime >= 1.0
                ? cursorAlphaMotionInTime
                : cursorAlphaMotionOutTime
                : cursorAlphaMotionInTime;

            cursorAlphaMotion = (int)Lerp(50 * (i + 1), 255, cursorAlphaMotionTime);
        }

        ImVec2 cursorMin = { pos.x + cursorRightMotion, cursorOffsetYMotion };
        ImVec2 cursorMax = { pos.x + cursorRightMotion + cursorScaleX, cursorMin.y + cursorScaleYMotion };

        drawList->AddImage(g_upTexArrow.get(), cursorMin, cursorMax, GET_UV_COORDS(cursorUVs), IM_COL32(255, 255, 255, cursorAlphaMotion));
    }
}


void AddTextWithOutline(ImDrawList* drawList, ImFont* font, float fontSize,
    ImVec2 pos, ImU32 textColor, ImU32 outlineColor,
    const char* text, float outlineSize, float outlineOffsetX, float outlineOffsetY)
{
    auto scaledSize = Scale(outlineSize);
    auto rings = std::max(1.0f, std::ceil(scaledSize));
    for (float ring = 1.0f; ring <= rings; ring += 1.0f)
    {
        auto thickness = scaledSize * ring / rings;
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0) 
                    continue;

                drawList->AddText(font, fontSize,
                    ImVec2(pos.x + dx * thickness + Scale(outlineOffsetX), pos.y + dy * thickness + Scale(outlineOffsetY)),
                    outlineColor, text);
            }
        }
    }
    drawList->AddText(font, fontSize, pos, textColor, text);
}

void AddTextCentered(ImDrawList* drawList, ImFont* font, float fontSize,
    float centerX, float y, ImU32 textColor, ImU32 outlineColor,
    const char* text)
{
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    ImVec2 pos = { centerX - textSize.x / 2.0f, y };
    AddTextWithOutline(drawList, font, fontSize, pos, textColor, outlineColor, text);
}

void DrawScaledRect(ImDrawList* drawList, ImVec2 winPos, float scale,
    float offsetX, float offsetY, float baseX, float baseY,
    float baseW, float baseH, ImU32 colour)
{
    ImVec2 topLeft = ImVec2(
        winPos.x + offsetX + (baseX * scale),
        winPos.y + offsetY + (baseY * scale)
    );
    ImVec2 bottomRight = ImVec2(
        topLeft.x + (baseW * scale),
        topLeft.y + (baseH * scale)
    );
    drawList->AddRectFilled(topLeft, bottomRight, colour);
};

// 0x44A80C seems to be the box anim duration (3.33) so we can re use that value here
constexpr float MSG_OPEN_SPEED = 10.0f / 3.0f;

float StepOpenProgress(float progress, bool isClosing)
{
    auto step = ImGui::GetIO().DeltaTime * MSG_OPEN_SPEED;
    return isClosing ? std::max(progress - step, 0.0f) : std::min(progress + step, 1.0f);
}

void DrawShadowWindow(ImDrawList* drawList, ImVec2 min, ImVec2 max, float scale, float openProgress)
{
    auto centreX = (min.x + max.x) * 0.5f;
    auto halfWidth = (max.x - min.x) * 0.5f * openProgress;
    min.x = centreX - halfWidth;
    max.x = centreX + halfWidth;

    // Original colors from the blue box (see sub_2A57E0).
    ImU32 bevCol = IM_COL32(16, 16, 128, 220); // Bevel Edge
    ImU32 shaCol = IM_COL32(0, 0, 16, 220); // 'Shadow' (top + left)
    ImU32 ligCol = IM_COL32(48, 48, 200, 220); // 'Light' (bottom + right)
    ImU32 winCol = IM_COL32(0, 0, 80, 220);  // Main Window

    // clamp insets so the open anim doesn't invert the inner rects while thin
    ImVec2 shaMin = { min.x + 4.0f * scale, min.y + 4.0f * scale };
    ImVec2 shaMax = { std::max(max.x - 4.0f * scale, shaMin.x), std::max(max.y - 4.0f * scale, shaMin.y) };
    ImVec2 winMin = { shaMin.x + scale, shaMin.y + scale };
    ImVec2 winMax = { std::max(shaMax.x - scale, winMin.x), std::max(shaMax.y - scale, winMin.y) };

    // Bevel at edges
    drawList->AddRectFilled(min, { shaMin.x, max.y }, bevCol);
    drawList->AddRectFilled({ shaMax.x, min.y }, max, bevCol);
    drawList->AddRectFilled({ shaMin.x, min.y }, { shaMax.x, shaMin.y }, bevCol);
    drawList->AddRectFilled({ shaMin.x, shaMax.y }, { shaMax.x, max.y }, bevCol);

    // Dark line (top + left)
    drawList->AddRectFilled(shaMin, { winMin.x, winMax.y }, shaCol);
    drawList->AddRectFilled({ winMin.x, shaMin.y }, { winMax.x, winMin.y }, shaCol);

    // Light line (bottom + right)
    drawList->AddRectFilled({ shaMin.x, winMax.y }, { winMax.x, shaMax.y }, ligCol);
    drawList->AddRectFilled({ winMax.x, shaMin.y }, { shaMax.x, winMax.y }, ligCol);

    // Main window background
    drawList->AddRectFilled(winMin, winMax, winCol);
}
