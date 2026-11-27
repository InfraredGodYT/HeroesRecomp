#pragma once

#include <gpu/video.h>

enum class EButtonIcon
{
    A,
    B,
    X,
    Y,
    LB,
    RB,
    LBRB,
    LT,
    RT,
    LTRT,
    Start,
    Back,
};

enum class EButtonAlignment
{
    Left,
    Right
};

class Button
{
public:
    std::string Name{};
    float MaxWidth{ FLT_MAX };
    EButtonIcon Icon{};
    EButtonAlignment Alignment{ EButtonAlignment::Right };
    bool* Visibility{ nullptr };

    Button(std::string name, float maxWidth, EButtonIcon icon, EButtonAlignment alignment, bool* visibility = nullptr)
        : Name(name), MaxWidth(maxWidth), Icon(icon), Alignment(alignment), Visibility(visibility) {
    }

    Button(std::string name, float maxWidth, EButtonIcon icon, bool* visibility = nullptr)
        : Name(name), MaxWidth(maxWidth), Icon(icon), Visibility(visibility) {
    }
};

class ButtonGuide
{
public:
    static inline bool s_isVisible = false;
    static inline float s_alpha = 1.0f;

    static void Init();
    static void Draw();
    static void Open(Button button);
    static void Open(std::span<Button> buttons);
    static void SetSideMargins(float width);
    static void SetLayout(float left, float right, float top);
    static void Close();
};
