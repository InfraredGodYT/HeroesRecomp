#pragma once

#include <filesystem>

struct InstallerWizard
{
    static inline bool s_isVisible = false;

    static void Init();
    static void Draw(X86Context* ctx);
    static void Shutdown();
    static bool Run(std::filesystem::path installPath);
};
