#include <kernel/function.h>
#include <user/config.h>
#include <patches/aspect_ratio_patches.h>
#include <gpu/video.h>

// The FMVs are mean to be played in 4:3 originally and thus, they stretch as the resolution increase.
// The game uses two rasters (640x480 and 1024x512) and then draws the FMV as a single quad covering 
// the whole camera raster.
// 
// This page rework this to implement different aspect ratio options to avoid stretching (fill and fit)


static constexpr uint32_t MovieManagerPtrAddr = 0x541808;

// RenderWare global
static constexpr uint32_t RwGlobalsAddr = 0x57DE00;
static constexpr uint32_t RwGlobalsCameraRaster = 0x60;
static constexpr uint32_t RwRasterWidth = 0xC;
static constexpr uint32_t RwRasterHeight = 0x10;

// The movie manager uses the regular combo of 4 vertex and a stride of 28 bytes.
static constexpr uint32_t MovieVerts = 0x30;
static constexpr uint32_t MovieVertStride = 0x1C;
static constexpr uint32_t MovieVertU = 0x14;
static constexpr uint32_t MovieVertV = 0x18;
static constexpr uint32_t MovieFrameWidth = 0xB8 + 0xC;
static constexpr uint32_t MovieFrameHeight = 0xB8 + 0x10;

// FMVs are made for a 4:3 screen, so never read the shape back from the frame size
static constexpr float MovieAspectRatio = 4.0f / 3.0f;

static uint32_t NextPowerOfTwo(uint32_t value)
{
    uint32_t result = 1;

    while (result < value)
        result <<= 1;

    return result;
}

static void SetMovieTexCoords(uint32_t manager, float u0, float v0, float u1, float v1)
{
    auto base = g_memory.base;

    const float u[4] = { u0, u1, u0, u1 };
    const float v[4] = { v0, v0, v1, v1 };

    for (uint32_t i = 0; i < 4; i++)
    {
        const uint32_t vertex = manager + MovieVerts + MovieVertStride * i;

        X86_STORE_F32(vertex + MovieVertU, u[i]);
        X86_STORE_F32(vertex + MovieVertV, v[i]);
    }
}

// TMovieManager::Draw(x, y, scaleX, scaleY)
X86_FUNC_IMPL(__imp__sub_24CF0);
X86_OVERRIDE_FUNC(sub_24CF0)
{
    const uint32_t args = ctx.esp.u32 + 4;
    const uint32_t manager = X86_LOAD_U32(MovieManagerPtrAddr);

    // The movie manager draws the same frame inside its own border
    // So stick to FS
    const bool isFullScreen = X86_LOAD_U32(args + 0x0) == 0 && X86_LOAD_U32(args + 0x4) == 0 &&
        X86_LOAD_U32(args + 0x8) == 0x3F800000 && X86_LOAD_U32(args + 0xC) == 0x3F800000;

    const uint32_t frameWidth = manager ? X86_LOAD_U32(manager + MovieFrameWidth) : 0;
    const uint32_t frameHeight = manager ? X86_LOAD_U32(manager + MovieFrameHeight) : 0;

    // save current frame's width and height for the clamp fix later done in video.cpp (see SetTextureInRenderThread)
    g_fmvFrameWidth = frameWidth;
    g_fmvFrameHeight = frameHeight;

    const uint32_t rwGlobals = X86_LOAD_U32(RwGlobalsAddr);
    const uint32_t cameraRaster = rwGlobals ? X86_LOAD_U32(rwGlobals + RwGlobalsCameraRaster) : 0;

    const int32_t rasterWidth = cameraRaster ? int32_t(X86_LOAD_U32(cameraRaster + RwRasterWidth)) : 0;
    const int32_t rasterHeight = cameraRaster ? int32_t(X86_LOAD_U32(cameraRaster + RwRasterHeight)) : 0;

    const float screenAspectRatio = g_aspectRatio;

    // if no proper info, just run original code
    if (!frameWidth || !frameHeight || rasterWidth <= 0 || rasterHeight <= 0 || screenAspectRatio <= 0.0f)
    {
        __imp__sub_24CF0(ctx, base);
        return;
    }

    const float maxU = float(frameWidth) / float(NextPowerOfTwo(frameWidth));
    const float maxV = float(frameHeight) / float(NextPowerOfTwo(frameHeight));

    const auto mode = Config::FMVAspectRatio.Value;

    if (mode == EFMVAspectRatio::Fill && isFullScreen)
    {
        // Keep the quad full screen and pull the texture coords in so the FMV is cropped properly.
        float cropU = 1.0f;
        float cropV = 1.0f;

        if (screenAspectRatio > MovieAspectRatio)
            cropV = MovieAspectRatio / screenAspectRatio;
        else
            cropU = screenAspectRatio / MovieAspectRatio;

        const float marginU = maxU * (1.0f - cropU) * 0.5f;
        const float marginV = maxV * (1.0f - cropV) * 0.5f;

        SetMovieTexCoords(manager, marginU, marginV, maxU - marginU, maxV - marginV);
    }
    else
    {
        // We still set the coords back (option can change while a movie is playing)
        SetMovieTexCoords(manager, 0.0f, 0.0f, maxU, maxV);
    }

    if (mode == EFMVAspectRatio::Fit && isFullScreen)
    {
        // Reduce the quad until it reaches 4:3 again and center it
        float scaleX = 1.0f;
        float scaleY = 1.0f;

        if (screenAspectRatio > MovieAspectRatio)
            scaleX = MovieAspectRatio / screenAspectRatio;
        else
            scaleY = screenAspectRatio / MovieAspectRatio;

        const int32_t x = int32_t(std::lround(rasterWidth * (1.0f - scaleX) * 0.5f));
        const int32_t y = int32_t(std::lround(rasterHeight * (1.0f - scaleY) * 0.5f));

        X86_STORE_U32(args + 0x0, uint32_t(x));
        X86_STORE_U32(args + 0x4, uint32_t(y));
        X86_STORE_F32(args + 0x8, scaleX);
        X86_STORE_F32(args + 0xC, scaleY);
    }

    __imp__sub_24CF0(ctx, base);
}
