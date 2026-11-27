#pragma once

#define MSG_OPEN   (false)
#define MSG_CLOSED (true)

class MessageWindow
{
public:
    static inline bool s_isVisible = false;

    static void Init();
    static void Draw(X86Context* ctx);
    static bool Open(std::string text, int* result, std::span<std::string> buttons = {}, int defaultButtonIndex = 0, int cancelButtonIndex = 1, bool hideFrame = false, bool guestHasControl = false, uint32_t padIndex = 0);
    static void SetSelection(int index);
    static void SetLayout(ImVec2 textAnchor, std::span<ImVec2> buttonAnchors);
    static void Close(bool immediate = false);
};
