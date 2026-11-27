#include <kernel/function.h>
#include <app.h>

static const uint32_t g_groundSlopeCos = 0x448F00;   // cos80 originally
static const uint32_t g_groundReachScale = 0x448F68; // 2.37f originally
static const uint32_t g_groundStickScale = 0x429F74; // 2.0f originally
static constexpr float g_seamRecastOffset = 0.05f;

static uint32_t g_rayFloorPlayer = 0;
static float g_rayFloorNormal[3];

// 0x84FEA runs once per pass on both branches
void CollisionToleranceResetMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	g_rayFloorPlayer = 0;
}

// ground ray at 0x85338 has no normal output, using it for side coli ray slot (esp+0x68)
void CollisionGroundRayNormalMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.esp.u32 + 0x8, ctx.esp.u32 + 0x44 + 0x68);
}

static float Length(const float v[3])
{
	return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static float Normalize(float v[3])
{
	float len = Length(v);
	if (len > 0.0f)
	{
		for (int i = 0; i < 3; i++)
		{
			v[i] /= len;
		}
	}
	return len;
}

static void Cross(const float a[3], const float b[3], float out[3])
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

static bool IsFloorNormal(const float normal[3], const float ground[3])
{
	auto base = g_memory.base;
	float len = Length(normal);
	if (len < 0.5f)
	{
		return false;
	}
	return (normal[0] * ground[0] + normal[1] * ground[1] + normal[2] * ground[2]) / len >= X86_LOAD_F32(g_groundSlopeCos);
}

// same call as 0x85338, shifted by offset
static float RecastGroundRay(const X86Context& ctx, uint8_t* base, uint32_t playerSlot, uint32_t filter, uint32_t stackTop, const float ray[6], const float offset[3], float hit[3], float normal[3])
{
	X86Context call = ctx;

	// ray +0, hit +0x18, normal +0x24 (below the live stack)
	uint32_t scratch = (stackTop - 0x40) & ~0xFu;
	for (int i = 0; i < 6; i++)
	{
		X86_STORE_F32(scratch + i * 4, ray[i] + offset[i % 3]);
	}

	call.esp.u32 = scratch;
	X86_PUSH(call, base, 2);
	X86_PUSH(call, base, 0x40000);
	X86_PUSH(call, base, filter);
	X86_PUSH(call, base, 0);
	X86_PUSH(call, base, scratch);
	X86_PUSH(call, base, 0);
	X86_PUSH(call, base, 0);
	X86_PUSH(call, base, 0);
	X86_PUSH(call, base, scratch + 0x24);
	X86_PUSH(call, base, scratch + 0x18);
	X86_PUSH(call, base, playerSlot);
	call.SubEsp(1);
	sub_82AD0(call, base);

	float dist = (float)call.fpu.st0(); // peak at float return (hack)
	for (int i = 0; i < 3; i++)
	{
		hit[i] = X86_LOAD_F32(scratch + 0x18 + i * 4) - offset[i];
		normal[i] = X86_LOAD_F32(scratch + 0x24 + i * 4);
	}
	return dist;
}

// sub_84E50 ground-stick tolerance for collision mesh seams/vertices
// Float imprecision at shared triangle edges/vertices causes the vert bumps
// Westopolis light pole before the goal ring is a good example of this, visible in HPP
// only helps over real floor, uses the ground normal so any gravity works
// after the ground ray, esp+0x14 is the distance (-1 = miss)
void CollisionToleranceMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	g_rayFloorPlayer = 0;

	uint32_t player = X86_LOAD_U32(ctx.esi.u32);

	// skip if not already grounded
	if ((X86_LOAD_U32(player + 0xa8) & 0x4) == 0)
	{
		return;
	}

	float radius = X86_LOAD_F32(player + 0x274);
	if (radius <= 0.0f)
	{
		return;
	}

	// stored ground normal (0x1C8 - 0x1D0)
	float ground[3] = { X86_LOAD_F32(player + 0x1c8), X86_LOAD_F32(player + 0x1cc), X86_LOAD_F32(player + 0x1d0) };
	if (Normalize(ground) < 0.7f)
	{
		return;
	}

	float reach = radius * X86_LOAD_F32(g_groundReachScale);
	float stick = radius * X86_LOAD_F32(g_groundStickScale);
	float dist = X86_LOAD_F32(ctx.esp.u32 + 0x14);
	float rayNormal[3];

	if (dist < 0.0f)
	{
		// ray start/end
		float ray[6];
		for (int i = 0; i < 6; i++)
		{
			ray[i] = X86_LOAD_F32(ctx.esp.u32 + 0x74 + i * 4);
		}

		// two tangents of the ground plane
		float axis[3] = { 0.0f, 1.0f, 0.0f };
		if (std::fabs(ground[1]) > 0.9f)
		{
			axis[0] = 1.0f;
			axis[1] = 0.0f;
		}
		float side1[3];
		Cross(ground, axis, side1);
		Normalize(side1);
		float side2[3];
		Cross(ground, side1, side2);

		// recast to both sides on a miss
		const float* sides[4] = { side1, side2, side1, side2 };
		for (int s = 0; s < 4 && dist < 0.0f; s++)
		{
			float shift = s < 2 ? g_seamRecastOffset : -g_seamRecastOffset;
			float offset[3] = { sides[s][0] * shift, sides[s][1] * shift, sides[s][2] * shift };
			float hit[3];

			float recastDist = RecastGroundRay(ctx, base, ctx.esi.u32, ctx.ebp.u32, ctx.esp.u32, ray, offset, hit, rayNormal);
			if (recastDist < 0.0f || recastDist > reach || !IsFloorNormal(rayNormal, ground))
			{
				continue;
			}

			dist = recastDist;
			X86_STORE_F32(ctx.esp.u32 + 0x8c, hit[0]);
			X86_STORE_F32(ctx.esp.u32 + 0x90, hit[1]);
			X86_STORE_F32(ctx.esp.u32 + 0x94, hit[2]);
			X86_STORE_F32(ctx.esp.u32 + 0x14, dist);
		}

		// real ledge
		if (dist < 0.0f)
		{
			return;
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			rayNormal[i] = X86_LOAD_F32(ctx.esp.u32 + 0x68 + i * 4);
		}

		// fallback to original behavior if our checks fail (walls)
		if (dist > reach || !IsFloorNormal(rayNormal, ground))
		{
			return;
		}
	}

	Normalize(rayNormal);
	for (int i = 0; i < 3; i++)
	{
		g_rayFloorNormal[i] = rayNormal[i];
	}

	g_rayFloorPlayer = player;

	// snap instead of the second ray
	if (dist > stick)
	{
		X86_STORE_F32(ctx.esp.u32 + 0x14, stick);
	}
}

static constexpr double g_snapFallbackTime = 0.75 / 60.0; // 1 frame at 60fps
static constexpr double g_snapStreakTime = 1.5 / 60.0;

struct SnapFallbackState
{
	double streakStart = 0.0;
	double missTime = 0.0;
	bool missing = false;
};

static std::unordered_map<uint32_t, SnapFallbackState> g_snapFallback;

// ray found floor but the sweep missed, stay grounded (with timer check to prevent wall exploits and fix ottotto)
bool CollisionToleranceSnapFallbackMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	uint32_t player = X86_LOAD_U32(ctx.esi.u32);
	auto& state = g_snapFallback[player];

	// sweep hit
	if (ctx.eax.u8_lo != 0)
	{
		if (state.missing)
		{
			state.missing = false;
			state.streakStart = App::s_time;
		}
		return false;
	}

	if (g_rayFloorPlayer != player)
	{
		return false;
	}

	// reset after a long enough hit streak
	if (!state.missing)
	{
		state.missing = true;
		if (App::s_time - state.streakStart >= g_snapStreakTime)
		{
			state.missTime = 0.0;
		}
	}

	// first frame is always covered (mainly for below 60fps)
	if (state.missTime >= g_snapFallbackTime)
	{
		return false;
	}
	state.missTime += App::s_deltaTime;

	// 0x856B1 (write ignoring state for coli and player)
	X86_STORE_F32(ctx.esp.u32 + 0x68, X86_LOAD_F32(ctx.esp.u32 + 0x18));
	X86_STORE_F32(ctx.esp.u32 + 0x6c, X86_LOAD_F32(ctx.esp.u32 + 0x1c));
	X86_STORE_F32(ctx.esp.u32 + 0x70, X86_LOAD_F32(ctx.esp.u32 + 0x20));
	X86_STORE_F32(ctx.esp.u32 + 0x4c, g_rayFloorNormal[0]);
	X86_STORE_F32(ctx.esp.u32 + 0x50, g_rayFloorNormal[1]);
	X86_STORE_F32(ctx.esp.u32 + 0x54, g_rayFloorNormal[2]);
	return true;
}

// 0x84E50 slope re-check @ 0x856AF
// sweep sometimes return an edge normal at a seam (Westopolis)
// now uses the ray floor normal as a fallback (shifted)
bool CollisionToleranceSlopeGateMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	uint32_t player = X86_LOAD_U32(ctx.esi.u32);
	if (g_rayFloorPlayer != player)
	{
		return false;
	}

	g_rayFloorPlayer = 0;
	float ground[3] = { X86_LOAD_F32(player + 0x1c8), X86_LOAD_F32(player + 0x1cc), X86_LOAD_F32(player + 0x1d0) };
	float sweep[3] = { X86_LOAD_F32(ctx.esp.u32 + 0x4c), X86_LOAD_F32(ctx.esp.u32 + 0x50), X86_LOAD_F32(ctx.esp.u32 + 0x54) };
	if (IsFloorNormal(sweep, ground))
	{
		return false;
	}

	X86_STORE_F32(ctx.esp.u32 + 0x4c, g_rayFloorNormal[0]);
	X86_STORE_F32(ctx.esp.u32 + 0x50, g_rayFloorNormal[1]);
	X86_STORE_F32(ctx.esp.u32 + 0x54, g_rayFloorNormal[2]);
	return true;
}

// Black Comet and Final Haunt have an issue with a specific collision (copy paste layout moment)
// A few polygons which should be treated as "floor" are actually a small wall, lying across the trench floor.
// It's not really an issue at 60 FPS since you will at worst get a tiny bump. Above however, it's enough to 
// be considered as a fence per the game and block the player until they have enough speed where 
// it eventually break.
// Fix: We swap the fence polygons for copies of the floor polygons next to them and GG.

struct ColiPolygonSwap
{
	uint16_t target;
	std::array<uint16_t, 4> targetValues;
	uint16_t source;
	std::array<uint16_t, 4> sourceValues;
};

struct PatchedColiFence
{
	const char* name;
	uint16_t numVertices;
	uint16_t numPolygons;
	uint64_t vertexHash; // hash the og vertex so the patch won't run for modded stages / cols
	std::vector<ColiPolygonSwap> poly;
};

// thanks Heroes Power Plant and Blender
static const PatchedColiFence coliFenceFixes[] =
{
	{ "STG0601_18_COLI_", 592, 764, 0x02A16541D0C519C4ull,
		{
			{ 1, { 2, 3, 4, 0 }, 3, { 6, 2, 4, 0 } },
			{ 2, { 3, 2, 5, 0 }, 4, { 2, 1, 4, 0 } },
			{ 5, { 2, 4, 5, 0 }, 3, { 6, 2, 4, 0 } },
			{ 6, { 3, 5, 4, 0 }, 4, { 2, 1, 4, 0 } },
			{ 8, { 4, 5, 8, 0 }, 7, { 7, 6, 4, 0 } },
			{ 9, { 5, 4, 8, 0 }, 12, { 9, 7, 4, 0 } },
			{ 10, { 7, 4, 8, 0 }, 7, { 7, 6, 4, 0 } },
			{ 11, { 4, 7, 8, 0 }, 12, { 9, 7, 4, 0 } },
		}
	},
	{ "STG0604_18_COLI_", 624, 790, 0xC896D1BFBB3E4CABull,
		{
			{ 693, { 522, 523, 524, 3 }, 696, { 523, 527, 524, 3 } },
			{ 694, { 523, 522, 524, 3 }, 698, { 529, 523, 524, 3 } },
		}
	},
};

// polygon field order don't match between the stream and the RpPolygon version, so sort them to compare them properly.
static bool PolygonMatches(uint32_t polygon, std::array<uint16_t, 4> expected)
{
	auto base = g_memory.base;
	std::array<uint16_t, 4> values;

	for (int i = 0; i < 4; i++)
	{
		values[i] = X86_LOAD_U16(polygon + i * 2);
	}

	std::sort(values.begin(), values.end());
	std::sort(expected.begin(), expected.end());
	return values == expected;
}

static void FixColiFences(uint32_t world)
{
	auto base = g_memory.base;

	if (world == NULL)
	{
		return;
	}

	// (RpWorld rootSector, rwSECTORATOMIC)
	uint32_t sector = X86_LOAD_U32(world + 0x1C);
	if (sector == NULL || (int32_t)X86_LOAD_U32(sector) != -1)
	{
		return;
	}

	uint32_t polygons = X86_LOAD_U32(sector + 0x4);
	uint32_t vertices = X86_LOAD_U32(sector + 0x8);

	if (polygons == NULL || vertices == NULL)
	{
		return;
	}

	// Memo: Xbox RpWorldSector is 0x88 bytes, counts are written by sub_2CDAF0
	uint16_t numVertices = X86_LOAD_U16(sector + 0x82);
	uint16_t numPolygons = X86_LOAD_U16(sector + 0x84);

	for (const auto& fixedCol : coliFenceFixes)
	{
		if (numVertices != fixedCol.numVertices || numPolygons != fixedCol.numPolygons || XXH3_64bits(base + vertices, fixedCol.numVertices * 12) != fixedCol.vertexHash)
		{
			continue;
		}

		for (const auto& swapPoly : fixedCol.poly)
		{
			if (!PolygonMatches(polygons + swapPoly.target * 8, swapPoly.targetValues) || !PolygonMatches(polygons + swapPoly.source * 8, swapPoly.sourceValues))
			{
				return;
			}
		}

		for (const auto& swapPoly : fixedCol.poly)
		{
			X86_STORE_U32(polygons + swapPoly.target * 8, X86_LOAD_U32(polygons + swapPoly.source * 8));
			X86_STORE_U32(polygons + swapPoly.target * 8 + 4, X86_LOAD_U32(polygons + swapPoly.source * 8 + 4));
		}

		return;
	}
}

// stage section COLI world load callback 
// (first param is the new RpWorld)
X86_FUNC_IMPL(__imp__sub_1B010);
X86_OVERRIDE_FUNC(sub_1B010)
{
	FixColiFences(X86_LOAD_U32(ctx.esp.u32 + 4));
	__imp__sub_1B010(ctx, base);
}

// Patch Coaster's bar collision to allow bullets
void CoasterHangbarCollisionMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.esp.u32, 0x40000);
}

// Lethal Highway stg0202_gr_001 bad last vert fix
// PATH.PTP load
void PathSplineFixMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	uint32_t path = ctx.eax.u32;
	if (path == NULL)
	{
		return;
	}

	uint32_t table = X86_LOAD_U32(path + 0x14);
	uint32_t spline;
	while (true)
	{
		spline = X86_LOAD_U32(table);
		if (spline == NULL)
		{
			return;
		}

		// decided to match by length and name rather than hashing since mods that tweak (e.g. grindable other types) should still get this fix
		uint32_t name = X86_LOAD_U32(spline + 0x2C);
		if (name != NULL && X86_LOAD_U32(spline) == 35 && strcmp(reinterpret_cast<const char*>(base + name), "stg0202_gr_001") == 0)
		{
			break;
		}
		table += 4;
	}

	uint32_t vertices = X86_LOAD_U32(spline + 0x8);
	uint32_t last = vertices + 33 * 0x20;
	X86_STORE_F32(spline + 0x4, X86_LOAD_F32(spline + 0x4) - X86_LOAD_F32(last + 0x18)); // update total length

	// fix the prior rotation
	for (int i = 0; i < 3; i++)
	{
		X86_STORE_F32(last + 0xC + i * 4, X86_LOAD_F32(last - 0x20 + 0xC + i * 4));
	}
	X86_STORE_F32(last + 0x18, 0.0f);
	X86_STORE_U32(last + 0x1C, 0);

	float minY = X86_LOAD_F32(vertices + 0x4);
	for (uint32_t v = 1; v < 34; v++)
	{
		float y = X86_LOAD_F32(vertices + v * 0x20 + 0x4);
		minY = std::min(minY, y);
	}
	X86_STORE_F32(spline + 0x24, minY);
	X86_STORE_U32(spline, 34);
}
