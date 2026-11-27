#include <kernel/function.h>
#include <cmath>
#include "global.h"

// Gravity Overhaul
// Fixes the awful implementation where state transitions apply wrong rotation or just straight up aren't done at all
// SpinDash and Slide were assuming NegY regardless of gravity mode
// Only applies (per player) if gravity is anything other than NegY

constexpr uint32_t PLAYER_TARGET_GRAVITY_UP = 0xE8;
constexpr uint32_t PLAYER_EULER_YAW = 0x14;
constexpr uint32_t PLAYER_VELOCITY = 0x1C;
constexpr uint32_t PLAYER_BASE_MATRIX = 0x1E4;

static uint32_t g_gravityPlayer = 0;

struct Quaternion
{
	float x, y, z, w;
};

// Helper to check if we should apply the fix
static bool GravityCorrectionNeeded(uint32_t player)
{
	auto base = g_memory.base;

	// Partner Structs can use this, so guard against them
	if (player == 0 || (player != g_playerStructAddrs[0] && player != g_playerStructAddrs[1] &&
		player != g_playerStructAddrs[2] && player != g_playerStructAddrs[3]))
	{
		return false;
	}

	const float x = X86_LOAD_F32(player + PLAYER_TARGET_GRAVITY_UP);
	const float y = X86_LOAD_F32(player + PLAYER_TARGET_GRAVITY_UP + 4);
	const float z = X86_LOAD_F32(player + PLAYER_TARGET_GRAVITY_UP + 8);

	return y < 0.99999f && (x * x + y * y + z * z) > 0.5f;
}

static Quaternion GravityMul(const Quaternion& a, const Quaternion& b)
{
	return {
		a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
		a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
		a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
		a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z };
}

static Quaternion GravityLoadQuat(uint32_t addr)
{
	auto base = g_memory.base;
	return { X86_LOAD_F32(addr), X86_LOAD_F32(addr + 4), X86_LOAD_F32(addr + 8), X86_LOAD_F32(addr + 12) };
}

// Original function assumed base gravity
// R = FromTo(up, player+0xE8) (aka sub_7FA10)
// Computes R * FromTo(up, R^-1 * n)
X86_FUNC_IMPL(__imp__sub_7FA10);
X86_OVERRIDE_FUNC(sub_7FA10)
{
	const uint32_t player = g_gravityPlayer;

	if (!GravityCorrectionNeeded(player))
	{
		__imp__sub_7FA10(ctx, base);
		return;
	}

	const uint32_t outQuat = X86_LOAD_U32(ctx.esp.u32 + 4);
	const uint32_t inVector = X86_LOAD_U32(ctx.esp.u32 + 8);

	const uint32_t scratch = (ctx.esp.u32 - 0x400) & ~0xF;
	const uint32_t gravityQuat = scratch + 0x00;
	const uint32_t localArc = scratch + 0x10;
	const uint32_t localNormal = scratch + 0x20;
	const uint32_t gravityMatrix = scratch + 0x30;
	const uint32_t gravityInverse = scratch + 0x60;

	// R = FromTo(up, gravity up)
	GuestToHostFunction<void, CallConvention::Cdecl>(ctx, __imp__sub_7FA10, gravityQuat, player + PLAYER_TARGET_GRAVITY_UP);

	// normal rotated into gravity space
	GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_30FDB0, gravityMatrix, gravityQuat);
	GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_3100F0, gravityMatrix, gravityInverse);
	GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_30F970, gravityInverse, localNormal, inVector);

	// apply with corrected arc and normal
	GuestToHostFunction<void, CallConvention::Cdecl>(ctx, __imp__sub_7FA10, localArc, localNormal);

	// out = R * arc
	const Quaternion q = GravityMul(GravityLoadQuat(gravityQuat), GravityLoadQuat(localArc));

	X86_STORE_F32(outQuat, q.x);
	X86_STORE_F32(outQuat + 4, q.y);
	X86_STORE_F32(outQuat + 8, q.z);
	X86_STORE_F32(outQuat + 12, q.w);

	ctx.eax.u32 = outQuat;
	ctx.esp.u32 += 4;
}

// Fix transitions
X86_FUNC_IMPL(__imp__sub_7FB30);
X86_OVERRIDE_FUNC(sub_7FB30)
{
	const uint32_t playerSlot = X86_LOAD_U32(ctx.esp.u32 + 4);
	const uint32_t previous = g_gravityPlayer;

	g_gravityPlayer = X86_LOAD_U32(playerSlot);
	__imp__sub_7FB30(ctx, base);
	g_gravityPlayer = previous;
}

// Per frame gravity update
X86_FUNC_IMPL(__imp__sub_908C0);
X86_OVERRIDE_FUNC(sub_908C0)
{
	const uint32_t executer = ctx.ecx.u32;
	const uint32_t previous = g_gravityPlayer;

	g_gravityPlayer = X86_LOAD_U32(executer + 0x28);
	__imp__sub_908C0(ctx, base);
	g_gravityPlayer = previous;
}

X86_FUNC_IMPL(__imp__sub_30F010);
static void GravityCarryVelocityBaseRelative(X86Context& ctx, uint8_t* base, uint32_t slot, uint32_t yawSlotOffset)
{
	const uint32_t player = X86_LOAD_U32(slot);
	if (!GravityCorrectionNeeded(player))
	{
		return;
	}

	const float oldYaw = X86_LOAD_F32(ctx.esp.u32 + yawSlotOffset);
	const float newYaw = X86_LOAD_F32(player + PLAYER_EULER_YAW);
	const float dyaw = newYaw - oldYaw;

	if (dyaw == 0.0f || !std::isfinite(dyaw))
	{
		return;
	}

	// need to allocate stack to do real calls
	const uint32_t originalStack = ctx.esp.u32;
	const uint32_t staging = (originalStack - 0x200) & ~0xF;
	const uint32_t inverse = (originalStack - 0x400) & ~0xF;
	const uint32_t local = inverse + 0x30;

	ctx.esp.u32 = staging;
	GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_3100F0, player + PLAYER_BASE_MATRIX, inverse);
	GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_30F970, inverse, local, player + PLAYER_VELOCITY);
	GuestToHostFunction<uint32_t, CallConvention::Cdecl>(ctx, __imp__sub_30F010, local, local, dyaw);
	GuestToHostFunction<void, CallConvention::Thiscall>(ctx, sub_30F970, player + PLAYER_BASE_MATRIX, player + PLAYER_VELOCITY, local);
	ctx.esp.u32 = originalStack;

	X86_STORE_F32(originalStack + yawSlotOffset, newYaw);
}

// Right after the steering call in sub_7A800 (Sliding::Exec)
// esi = player
// esp+0x18 = yaw
void SlideCarryMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	GravityCarryVelocityBaseRelative(ctx, base, ctx.esi.u32, 0x18);
}


// SpinDash Exec (spin/moving)
// esi = player
// esp+0x1C = yaw
void SpinCarryMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	GravityCarryVelocityBaseRelative(ctx, base, ctx.esi.u32, 0x1C);
}
