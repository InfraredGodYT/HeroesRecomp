#include <cstring>
#include <kernel/function.h>
#include <user/config.h>
#include <xbox.h>
#include <patches/global.h>

//HintRingExec aka the capsule with "?" 
X86_FUNC_IMPL(__imp__sub_1A1E60);
X86_OVERRIDE_FUNC(sub_1A1E60)
{
	uint32_t this_ = ctx.ecx.u32;

	// mode 2 = the hint message is playing, let it finish to be safe.
	if (Config::Hints.Value || X86_LOAD_U32(this_ + 0x64) == 2)
	{
		__imp__sub_1A1E60(ctx, base);
		return;
	}

	uint32_t hintFlags = X86_LOAD_U32(this_ + 0x6C);
	uint32_t gadget = X86_LOAD_U32(this_ + 0x38);

	// the game has a flag to hide the hint so we can just re use that too
	if ((hintFlags & 4) == 0 && gadget != 0)
	{
		X86Context call = ctx;
		call.ecx.u32 = gadget;
		call.SubEsp(1);
		sub_17AC60(call, base);
	}

	X86_STORE_U32(this_ + 0x6C, (hintFlags | 4) & ~1u);
	ctx.AddEsp(2);
}

// sub_17E290 (ObjectUpdateVisibility (gadgets)) calls sub_17C130 (SetFadeAlpha)
// fading moves clumps into world 10 (_AL)
// most gadgets are supposed to use _A not _AL (hence the white flash, applying light when it shouldn't)
// _AL has backface culling while _A doesn't, so we only remove the light and keep the other properties of _AL
static std::unordered_map<uint32_t, uint32_t> g_fadeOriginalWorld;

// RenderWare name, I call these layers personally but 'world' is accurate
static uint32_t GetClumpWorld(uint8_t* base, uint32_t clump)
{
	return X86_LOAD_U32(clump + X86_LOAD_U32(0x55F4D8));
}

// render layers (world) table @ 0x4E50F0 (_O, _OS, _A, _AL etc...)
// Gadgets that have the lit flag should actually use AL, so we skip patching them
static bool IsUnlitLayer(uint8_t* base, uint32_t world)
{
	for (uint32_t layer = 0x4E50F0; layer < 0x4E50F0 + 15 * 0x18; layer += 0x18)
	{
		if (X86_LOAD_U32(layer + 0x8) == world)
		{
			return (X86_LOAD_U32(layer) & 0x1) == 0;
		}
	}
	return false;
}

// When an object switches world (e.g. _O to _AL) RW invokes LTM sync (both rwFRAMEPRIVATEHIERARCHYSYNCLTM and rwFRAMEPRIVATESUBTREESYNCLTM, thanks donut!)
// When the world switches the object is does the sync and reset to origin (0,0,0) which was what I thought was a 'dropped frame'
// If we track the flags we can just persist them across world moves
struct FadeFrame
{
	uint32_t frame;
	uint8_t privateFlags;
	uint8_t rootPrivateFlags;
};

static void SaveFrameFlags(uint8_t* base, uint32_t clump, std::vector<FadeFrame>& frames)
{
	const auto save = [&](uint32_t frame)
	{
		if (frame)
		{
			const uint32_t root = X86_LOAD_U32(frame + 0xA0);
			frames.push_back({ frame, X86_LOAD_U8(frame + 0x3), X86_LOAD_U8(root + 0x3) });
		}
	};

	save(X86_LOAD_U32(clump + 0x4));
	const uint32_t atomicEnd = clump + 0x8;
	for (uint32_t link = X86_LOAD_U32(atomicEnd); link != atomicEnd; link = X86_LOAD_U32(link))
	{
		const uint32_t atomic = link - 0x40;
		save(X86_LOAD_U32(atomic + 0x4));
	}
}

static void ClearNewLtmSync(uint8_t* base, const std::vector<FadeFrame>& frames)
{
	for (const auto& saved : frames)
	{
		const uint32_t root = X86_LOAD_U32(saved.frame + 0xA0);
		X86_STORE_U8(root + 0x3, X86_LOAD_U8(root + 0x3) & (saved.rootPrivateFlags | ~0x1));
		X86_STORE_U8(saved.frame + 0x3, X86_LOAD_U8(saved.frame + 0x3) & (saved.privateFlags | ~0x4));
	}
}

// SetFadeAlpha
X86_FUNC_IMPL(__imp__sub_17C130);
X86_OVERRIDE_FUNC(sub_17C130)
{
	const uint32_t gadget = ctx.ecx.u32;
	std::vector<FadeFrame> frames;
	if ((X86_LOAD_U32(gadget + 0x3C) & 0x40) == 0)
	{
		const uint32_t listEnd = X86_LOAD_U32(gadget + 0x50);
		for (uint32_t node = X86_LOAD_U32(listEnd); node != listEnd; node = X86_LOAD_U32(node))
		{
			const uint32_t clump = X86_LOAD_U32(node + 0x8);
			const uint32_t world = GetClumpWorld(base, clump);
			// only patch if the gadget is supposed to be in _A
			if (world && IsUnlitLayer(base, world))
			{
				g_fadeOriginalWorld[clump] = world;
			}
			else
			{
				g_fadeOriginalWorld.erase(clump);
			}
			SaveFrameFlags(base, clump, frames);
		}
	}
	__imp__sub_17C130(ctx, base);
	ClearNewLtmSync(base, frames);
}

// Fade end
X86_FUNC_IMPL(__imp__sub_17C1C0);
X86_OVERRIDE_FUNC(sub_17C1C0)
{
	const uint32_t gadget = ctx.ecx.u32;
	std::vector<FadeFrame> frames;
	if (X86_LOAD_U32(gadget + 0x3C) & 0x40)
	{
		const uint32_t listEnd = X86_LOAD_U32(gadget + 0x50);
		for (uint32_t node = X86_LOAD_U32(listEnd); node != listEnd; node = X86_LOAD_U32(node))
		{
			const uint32_t clump = X86_LOAD_U32(node + 0x8);
			g_fadeOriginalWorld.erase(clump);
			SaveFrameFlags(base, clump, frames);
		}
	}
	__imp__sub_17C1C0(ctx, base);
	ClearNewLtmSync(base, frames);
}

// Gadget atomic render
X86_FUNC_IMPL(__imp__sub_17A5C0);
X86_OVERRIDE_FUNC(sub_17A5C0)
{
	const uint32_t atomic = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const uint32_t clump = X86_LOAD_U32(atomic + 0x3C);
	const auto originalWorld = g_fadeOriginalWorld.find(clump);
	if (originalWorld == g_fadeOriginalWorld.end() || GetClumpWorld(base, clump) != X86_LOAD_U32(0x4E51E8))
	{
		__imp__sub_17A5C0(ctx, base);
		return;
	}
	// restore the original mode
	const uint32_t curWorld = X86_LOAD_U32(0x57DE04);
	X86_STORE_U32(0x57DE04, originalWorld->second);
	__imp__sub_17A5C0(ctx, base);
	X86_STORE_U32(0x57DE04, curWorld);
}

// Shared AnimTask func (used by a lot of objects)
X86_FUNC_IMPL(__imp__sub_17E290);
X86_OVERRIDE_FUNC(sub_17E290)
{
	// deltatime patch (used by a lot of objects for fade and anim transitions)
	const float dt = X86_LOAD_F32(ctx.esp.u32 + 4);
	float frames = dt * 60.0f;
	if (!(frames > 0.0f))
	{
		frames = 1.0f;
	}
	frames = std::min(frames, 1.0f);
	const uint32_t rateAddr = ctx.ecx.u32 + 0x48;
	const float restoreRate = X86_LOAD_F32(rateAddr);
	if (std::fabs(frames - 1.0f) >= 0.0001f)
	{
		X86_STORE_F32(rateAddr, restoreRate * frames);
	}

	__imp__sub_17E290(ctx, base);

	// deltatime patch
	X86_STORE_F32(rateAddr, restoreRate);
}

// BombingWall filter patch for CB
void BombingWallBreakMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (ctx.ecx.u32 & 0x10)
	{
		ctx.ecx.u32 |= 0x200;
	}
}

// Switch attacker filter callback (Ball switch)
// Extend to include melee swing (0x80) + melee generic (0x2)
bool BallSwitchTriggerMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return ctx.eax.u32 != 0 && (X86_LOAD_U32(ctx.eax.u32 + 0x40) & 0x82) != 0;
}

// BKTank only responds to damage once in StopAction
// Ignore it for StopAction (passing to damage callback)
bool BKTankStopActionDamageFixMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return X86_LOAD_F32(ctx.esi.u32 + 0x28) > 0.0f;
}
