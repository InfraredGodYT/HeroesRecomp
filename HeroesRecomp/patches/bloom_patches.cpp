#include <kernel/function.h>
#include <user/config.h>
#include "aspect_ratio_patches.h"

static const uint32_t bloomAddr = 0x52E5C4;

// change bloom intensity based on user's config.

void BloomConfigValueChangedCallback(IConfigDef* config)
{
	if (config == &Config::BloomIntensity)
	{
		float rounded = std::roundf(Config::BloomIntensity.Value * 100.0f) / 100.0f;

		Config::BloomIntensity.Value = rounded;

		if (Config::BloomIntensity.Value < 0.01f)
			Config::BloomIntensity.Value = 0.0f;

		if (Config::BloomIntensity.Value > 1.f)
			Config::BloomIntensity.Value = 1.0f;

		auto base = g_memory.base;
		X86_STORE_F32(bloomAddr, Config::BloomIntensity.Value);
	}
}

struct BloomData
{
	float offsetX;
	float offsetY;
	float width;
	float height;
	float u0, v0;
	float u1, v1;
};

// This function builds a 4 vertex quad which is used for every glare.
// This is originally buggy, some pixels come out dark, creating a blurry band along the top and left of the screen.
// This seems to be because they tried to prevent the quad's texture coordinates sampling outside the source 
// texture when a tap shifts by a fraction of a pixel. Thus, they lowered the destination quad, which ends being too small.
// We can patch this by keeping the quad full size and shift the UV window by the same amount

// bloom spreads narrower or wider you get, so ultrawide or ultranarrow looks awful (esp for jet shoes)
// scale to spread like 4:3
static float UnstretchBloomTap(float offset, float unstretch)
{
	// 0 is the composite quad, not a tap
	if (unstretch >= 1.0f || offset == 0.0f)
	{
		return offset;
	}
	return (offset + 0.5f) * unstretch - 0.5f;
}

X86_FUNC_IMPL(__imp__sub_3258A0);
X86_OVERRIDE_FUNC(sub_3258A0)
{
	const uint32_t args = ctx.esp.u32 + 4;

	BloomData bwk;
	memset(&bwk, 0, sizeof(BloomData));
	bwk.offsetX = UnstretchBloomTap(GetHostParameterFloat(ctx, 2, CallConvention::Thiscall), g_hudUnstretchX);
	bwk.offsetY = UnstretchBloomTap(GetHostParameterFloat(ctx, 3, CallConvention::Thiscall), g_hudUnstretchY);
	bwk.width = GetHostParameterFloat(ctx, 4, CallConvention::Thiscall);
	bwk.height = GetHostParameterFloat(ctx, 5, CallConvention::Thiscall);
	bwk.u0 = GetHostParameterFloat(ctx, 6, CallConvention::Thiscall);
	bwk.v0 = GetHostParameterFloat(ctx, 7, CallConvention::Thiscall);
	bwk.u1 = GetHostParameterFloat(ctx, 8, CallConvention::Thiscall);
	bwk.v1 = GetHostParameterFloat(ctx, 9, CallConvention::Thiscall);


	if (bwk.width != 0.0f)
	{
		const float offset = (bwk.u1 - bwk.u0) * (bwk.offsetX / bwk.width);

		X86_STORE_F32(args + 0x00, 0.0f); // offset X 
		X86_STORE_F32(args + 0x10, bwk.u0 - offset); // tex coordinate (u0)
		X86_STORE_F32(args + 0x18, bwk.u1 - offset); //tex coordinate (u1)
	}

	if (bwk.height != 0.0f)
	{
		const float offset = (bwk.v1 - bwk.v0) * (bwk.offsetY / bwk.height);

		X86_STORE_F32(args + 0x4, 0.0f); // offset Y
		X86_STORE_F32(args + 0x14, bwk.v0 - offset); // tex coordinate (V0)
		X86_STORE_F32(args + 0x1C, bwk.v1 - offset);  // tex coordinate (V1)

	}

	__imp__sub_3258A0(ctx, base);
}
