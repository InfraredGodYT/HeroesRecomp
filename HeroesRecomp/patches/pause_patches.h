#pragma once

void PausePatchesInit();
void DrawPauseText(X86Context* ctx);

void RequestQuitOptionForPause();
void TogglePauseCSD(bool toggle);
void TogglePauseBossCSD(bool toggle);
void ToggleMessageTaskDraw(bool toggle);
const uint8_t GetAddedPauseItemsCount();

