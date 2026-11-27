#include <cmath>
#include <kernel/function.h>
#include <patches/aspect_ratio_patches.h>
#include <gpu/video.h>
#include <ui/imgui_utils.h>
#include <user/config.h>
#include <ui/game_window.h>
#include <api/CSD/csd.h>
#include <os/logger.h>
#include <app.h>
#include <ui/options_menu.h>
#include "pause_patches.h"
#include "multiplayer_mod_patches.h"
#include "prompt_patches.h"
#include "fps_patches.h"
#include "global.h"
#include <mod/mod_loader.h>

static const uint8_t CSDVertexCount = 4u; // CSD always use 4 vertex count in this game
bool readyToDebugConstant = false;
#define widthMin 640.f
#define heightMin 480.f
static Mutex g_pathMutex;
static Mutex g_bufferMutex;
static std::map<const void*, XXH64_hash_t> g_paths;
static std::unordered_map<const uint8_t*, std::vector<const uint8_t*>> g_bufferRegistry;
static uint8_t* g_currentBuffer = nullptr;
static std::optional<CsdModifier> g_sceneModifier{};
static std::optional<CsdModifier> g_castNodeModifier{};
static std::optional<CsdModifier> g_castModifier{};
static uint32_t hudFlagPtr = 0;

static XXH64_hash_t HashStr(const std::string_view& value)
{
	return XXH3_64bits(value.data(), value.size());
}

static float ComputeScale(float aspectRatio)
{
	return ((aspectRatio * heightMin) / widthMin) / sqrt((aspectRatio * heightMin) / widthMin);
}

void AspectRatioPatches::ComputeOffsets()
{
	float width = Video::s_viewportWidth;
	float height = Video::s_viewportHeight;

	g_aspectRatio = width / height;
	g_aspectRatioGameplayScale = 1.0f;

	if (g_aspectRatio >= NARROW_ASPECT_RATIO)
	{
		g_aspectRatioOffsetX = (width - height * WIDE_ASPECT_RATIO) / 2.0f;
		g_aspectRatioOffsetY = 0.0f;
		g_aspectRatioScale = height / heightMin;

		// keep same scale above Steam Deck aspect ratio
		if (g_aspectRatio < WIDE_ASPECT_RATIO)
		{
			// interpolate to original 4:3 scale
			float steamDeckScale = g_aspectRatio / WIDE_ASPECT_RATIO;
			float narrowScale = ComputeScale(NARROW_ASPECT_RATIO);

			float lerpFactor = std::clamp((g_aspectRatio - NARROW_ASPECT_RATIO) / (STEAM_DECK_ASPECT_RATIO - NARROW_ASPECT_RATIO), 0.0f, 1.0f);
			g_aspectRatioGameplayScale = narrowScale + (steamDeckScale - narrowScale) * lerpFactor;
		}
	}
	else
	{
		// 4:3 crop
		g_aspectRatioOffsetX = (width - width * NARROW_ASPECT_RATIO) / 2.0f;
		g_aspectRatioOffsetY = (height - width / NARROW_ASPECT_RATIO) / 2.0f;
		g_aspectRatioScale = width / widthMin;
		g_aspectRatioGameplayScale = ComputeScale(NARROW_ASPECT_RATIO);
	}

	g_aspectRatioNarrowScale = std::clamp((g_aspectRatio - NARROW_ASPECT_RATIO) / (WIDE_ASPECT_RATIO - NARROW_ASPECT_RATIO), 0.0f, 1.0f);
	g_aspectRatioNarrowMargin = std::lerp(50.0f, 0.0f, g_aspectRatioNarrowScale);

	// the game draws all 2D in a fixed 640x480 space that we stretch over the viewport,
	// so one axis always ends up stretched. squeezing it back gives square pixels.
	g_hudUnstretchX = std::min(1.0f, NARROW_ASPECT_RATIO / g_aspectRatio);
	g_hudUnstretchY = std::min(1.0f, g_aspectRatio / NARROW_ASPECT_RATIO);

	g_horzCentre = g_aspectRatioOffsetX + 640.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;
	g_vertCentre = g_aspectRatioOffsetY + 360.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;
	UpdateFontSize();
}

std::string NormalizeCSDPath(std::string& path)
{
	std::replace(path.begin(), path.end(), '\\', '/');

	auto pos = path.find("csdFiles");

	if (pos != std::string::npos)
		return path.substr(pos);

	return path;
}

static void EmplacePath(const void* key, const std::string_view& value)
{
	std::lock_guard lock(g_pathMutex);

	g_paths.emplace(key, HashStr(value));

	if (g_currentBuffer)
	{
		g_bufferRegistry[g_currentBuffer].push_back((const uint8_t*)key);

		//printf("Emplace key %x, %s\n", key, value.data());
	}
}

// Some csd have node name using Shift-JIS encoded Japanese characters being misinterpreted as Latin-1/UTF-8. 
// This can crash Recomp when storing those, so we fix that by replacing them with _
std::string SanitizeName(const char* str)
{
	if (!str)
		return "";

	std::string result(str);
	for (char& c : result)
	{
		if ((unsigned char)c > 127)
			c = '_';
	}
	return result;
}

void UnhideSynopsisSkipPrompt(uint32_t cast, const std::string& path);
void TraverseCast(CSDScene* scene, uint32_t castNodeIndex, CSDCastNode* castNode, uint32_t castIndex, const std::string& parentPath)
{

	if (castIndex == ~0)
		return;

	auto& pCastLink = castNode->pCastLinks[castIndex];

	TraverseCast(scene, castNodeIndex, castNode, pCastLink.SiblingCastIndex, parentPath);

	std::string path = SanitizeName(parentPath.c_str());
	path = NormalizeCSDPath(path);

	for (size_t i = 0; i < scene->CastCount; i++)
	{
		auto& index = scene->pCastIndices[i];
		if (index.CastNodeIndex == castNodeIndex && index.CastIndex == castIndex)
		{
			path += index.pCastName;
			break;
		}
	}

	CSDCast* pCast = castNode->pCasts[castIndex].get();
	path = SanitizeName(path.c_str());
	EmplacePath(pCast, path);

	UnhideSynopsisSkipPrompt(g_memory.MapVirtual(pCast), path);

	if (castNode->RootCastIndex == castIndex)
		EmplacePath(castNode, path);

	path += "/";


	std::string fixedpath = SanitizeName(path.c_str());

	TraverseCast(scene, castNodeIndex, castNode, castNode->pCastLinks[castIndex].ChildCastIndex, fixedpath);

	//LOGFN_UTILITY("CSD hierarchy: {}", fixedpath);
}

void TraverseScene(CSDScene* scene, std::string path)
{
	std::string pathFixed = SanitizeName(path.c_str());
	pathFixed = NormalizeCSDPath(pathFixed);
	EmplacePath(scene, pathFixed);

	pathFixed += "/";

	for (size_t i = 0; i < scene->CastNodeCount; i++)
	{
		auto& castNode = scene->pCastNodes[i];
		TraverseCast(scene, i, &castNode, castNode.RootCastIndex, pathFixed);
	}
}

void TraverseSceneNode(CSDSceneNode* node, const std::string& path)
{
	std::string pathFixed = SanitizeName(path.c_str());
	pathFixed = NormalizeCSDPath(pathFixed);

	EmplacePath(node, pathFixed);
	std::string basePath = pathFixed + "/";

	for (uint32_t i = 0; i < node->SceneCount; ++i)
	{
		auto& idx = node->pSceneIndices[i];
		void* scene = (void*)node->pScenes[idx.SceneIndex];

		std::string scenePath = basePath + idx.pSceneName.get();
		TraverseScene(node->pScenes[idx.SceneIndex], scenePath);
	}


	for (uint32_t j = 0; j < node->SceneNodeCount; ++j)
	{
		auto& idx = node->pSceneNodeIndices[j];
		CSDSceneNode* child = &node->pSceneNodes[idx.SceneNodeIndex];
		std::string childPath = basePath + idx.pSceneNodeName.get();
		TraverseSceneNode(child, childPath);
	}
}

static void RegisterCsdFile(uint8_t* base, void* thisPtr, const char* path);

// CSD_LOADER
X86_FUNC_IMPL(__imp__sub_2B47C0);
X86_OVERRIDE_FUNC(sub_2B47C0)
{
	std::lock_guard lock2(g_bufferMutex);

	void* thisPtr = (void*)GetHostParameter(ctx, 1, CallConvention::Thiscall, ParameterType::Pointer);
	char* path = (char*)GetHostParameter(ctx, 2, CallConvention::Thiscall, ParameterType::Pointer);

	__imp__sub_2B47C0(ctx, base);

	if (!thisPtr || !path)
		return;

	std::string str = path;
	RegisterCsdFile(base, thisPtr, path);
}

// Some CSD uses a different function to be loaded (such as partner intro names or mission result).
// The file is read into a buffer first and then parsed (sub_2B4AC0), which never sees the path, so it get lost.
// As a workaround, we can use the buffer as a key and copy the path in a map.
static Mutex g_asyncCsdMutex;
static std::unordered_map<uint32_t, std::string> g_asyncCsdPaths;

// TBGRead ctor (this, request, file handle)
X86_FUNC_IMPL(__imp__sub_1A6D0);
X86_OVERRIDE_FUNC(sub_1A6D0)
{
	auto request = X86_LOAD_U32(ctx.esp.u32 + 4);
	auto pathAddr = request ? X86_LOAD_U32(request) : 0;
	auto key = request ? X86_LOAD_U32(request + 4) : 0;

	if (pathAddr && key)
	{
		std::string path = (const char*)g_memory.Translate(pathAddr);
		std::string lower = path;
		std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });

		// some CSD path get their slash flip, ensure accuracy.
		std::replace(path.begin(), path.end(), '\\', '/');

		auto rootPos = lower.find("csdfiles");

		if (rootPos != std::string::npos)
			path.erase(0, rootPos);

		std::lock_guard lock(g_asyncCsdMutex);

		if (lower.find(".xncp") != std::string::npos)
			g_asyncCsdPaths[key] = std::move(path);
		else
			g_asyncCsdPaths.erase(key);
	}

	__imp__sub_1A6D0(ctx, base);
}

// Stages .one archives can have CSD files too (like checkpoint warp menu).
// sub_29FF30 read the entry into a buffer then parse it with sub_2B4AC0, so we can intercept the data safely here.
// There is no path at all, so we just store the csd filename.
// Memo: eax = buffer from TOneFileAsync::Create, ebp = archive entry name
void OneFileCsdBufferMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!ctx.eax.u32 || !ctx.ebp.u32)
		return;

	std::string path = (const char*)g_memory.Translate(ctx.ebp.u32);
	std::string lower = path;
	std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });

	if (lower.find(".xncp") == std::string::npos)
		return;

	std::lock_guard lock(g_asyncCsdMutex);
	g_asyncCsdPaths[ctx.eax.u32] = std::move(path);
}

// CSD loader from memory
X86_FUNC_IMPL(__imp__sub_2B4AC0);
X86_OVERRIDE_FUNC(sub_2B4AC0)
{
	std::lock_guard lock2(g_bufferMutex);

	void* thisPtr = (void*)GetHostParameter(ctx, 1, CallConvention::Thiscall, ParameterType::Pointer);
	auto buffer = X86_LOAD_U32(ctx.esp.u32 + 4);
	std::string path;

	{
		// the buffer is freed right after parsing
		std::lock_guard lock(g_asyncCsdMutex);

		auto findResult = g_asyncCsdPaths.find(buffer);

		if (findResult != g_asyncCsdPaths.end())
		{
			path = std::move(findResult->second);
			g_asyncCsdPaths.erase(findResult);
		}
	}

	__imp__sub_2B4AC0(ctx, base);

	if (!thisPtr || path.empty())
		return;

	RegisterCsdFile(base, thisPtr, path.c_str());
}

static void RegisterCsdFile(uint8_t* base, void* thisPtr, const char* path)
{
	auto csdFileAddrPtr = X86_LOAD_U32(g_memory.MapVirtual((const uint8_t*)thisPtr + 4));

	if (!csdFileAddrPtr)
		return;

	// since we can't blindly free g_paths member since no size are stored, we will store the CSD file pointer as a key instead
	// each child (cast, node etc.) will be able to get identified properly this way
	auto csdFilePtr = X86_LOAD_U32(csdFileAddrPtr + 4);
	g_currentBuffer = (uint8_t*)g_memory.Translate(csdFilePtr);

	auto addrNodePtr = X86_LOAD_U32(g_memory.MapVirtual((const uint8_t*)thisPtr + 8));

	if (addrNodePtr)
	{
		auto resPtr = X86_LOAD_U32(addrNodePtr);
		CSDSceneNode* nodePtr = (CSDSceneNode*)(base + resPtr);

		if (nodePtr)
		{
			TraverseSceneNode(nodePtr, path);
		}
	}

	g_currentBuffer = nullptr;
}

static bool s_hudDisabled = false;

void ToggleHud(bool toggle)
{
	auto base = g_memory.base;
	X86_STORE_U16(0x54DE78, toggle ? hudFlagPtr : 0);
	X86_STORE_U8(0x4E4B74, toggle ? 1 : 0);
	s_hudDisabled = toggle == false;
}

void PatchDisableHudMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (s_hudDisabled)
	{
		X86_STORE_U16(0x54DE78, 0x0);
	}
}

static std::optional<XXH64_hash_t> FindCsdPath(uint32_t data);
static bool IsMultiplayerHudHiddenScene(uint32_t scene)
{
	static const XXH64_hash_t s_hidden[] =
	{
		HashStr("csdFiles/2P_Ring.xncp/Ring1P"),
		HashStr("csdFiles/2P_Ring.xncp/Ring2P"),
		HashStr("csdFiles/2P_CommandWindow.xncp/CarMeter"),
		HashStr("csdFiles/2P_CommandWindow.xncp/Car_BCom"),
		HashStr("csdFiles/2P_CommandWindow.xncp/Weapon"),
		HashStr("csdFiles/2P_CommandWindow.xncp/Window"),
		HashStr("csdFiles/2P_Star.xncp/Star"),
		HashStr("csdFiles/HeroGauge.xncp/Scene_0000"),
		HashStr("csdFiles/DarkGauge.xncp/Scene_0000"),
		HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetDark"),
		HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetHero"),
		HashStr("csdFiles/Hissatu_Hero.xncp/HissatuHero"),
		HashStr("csdFiles/Hissatu_Dark.xncp/HissatuDark"),
	};
	const auto path = FindCsdPath(scene);
	if (!path.has_value())
	{
		return false;
	}
	if (path.value() == HashStr("csdFiles/BossGauge.xncp/Scene_0000"))
	{
		NotifyMultiplayerHudBossGauge();
		return true;
	}
	return std::find(std::begin(s_hidden), std::end(s_hidden), path.value()) != std::end(s_hidden);
}

// Chao::CSD::Scene::Render
X86_FUNC_IMPL(__imp__sub_38B5E0);
X86_OVERRIDE_FUNC(sub_38B5E0)
{
	auto res = X86_LOAD_U32(0x54DE78);
	if (res)
		hudFlagPtr = res;

	auto pSceneAddr = X86_LOAD_U32(ctx.esp.u32 + 4);
	auto pScene = (CSDScene*)(base + pSceneAddr);

	// Hide the HUD in 3P/4P
	if (IsExtraPlayerMode() && IsMultiplayerHudHiddenScene(pSceneAddr))
	{
		ctx.esp.u32 += 4;
		return;
	}
	g_sceneModifier = FindCsdModifier(pSceneAddr);


	__imp__sub_38B5E0(ctx, base);
}

//TCsdSystem::FreeMemory
X86_FUNC_IMPL(__imp__sub_184E0);
X86_OVERRIDE_FUNC(sub_184E0)
{
	auto ptr = X86_LOAD_U32(ctx.esp.u32 + 4);

	if (ptr != 0)
	{
		uint32_t magic1 = X86_LOAD_U32(ptr);
		uint32_t magic2 = X86_LOAD_U32(ptr + 0x20);


		if (magic1 == 0x4649584E && magic2 == 0x4A50436E) // NXIF, nCPJ
		{

			const uint8_t* base_key = (const uint8_t*)(base + ptr);

			std::lock_guard lock(g_pathMutex);

			auto it = g_bufferRegistry.find(base_key);
			if (it != g_bufferRegistry.end())
			{
				for (auto* child : it->second)
					g_paths.erase(child);

				g_bufferRegistry.erase(it);
				LOGFN_UTILITY("CSD freed: 0x{:08X}", (uint64_t)base_key);
			}
		}
	}
	__imp__sub_184E0(ctx, base);
}

void RenderCsdCastNodeMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	g_castNodeModifier = FindCsdModifier(ctx.eax.u32);
}

void RenderCsdCastMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	g_castModifier = FindCsdModifier(ctx.esi.u32);
}

int FindHudWidthPreset(float aspectRatio)
{
	for (size_t i = 0; i < std::size(g_hudWidthPresets); i++)
	{
		// mainly for float rounding bs, but also for approx manual config values
		if (std::abs(aspectRatio - g_hudWidthPresets[i].AspectRatio) < 0.01f)
		{
			return int(i);
		}
	}
	return -1;
}

struct CsdFrame
{
	float Left;
	float Right;
	float Top;
	float Bottom;
};

static CsdFrame GetCsdFrame(uint64_t flags)
{
	const int preset = FindHudWidthPreset(Config::MaxHUDWidth.Value);
	const float maxAspectRatio = preset >= 0 ? g_hudWidthPresets[preset].AspectRatio : Config::MaxHUDWidth.Value;
	float width = widthMin;

	if (maxAspectRatio > 0.0f && g_aspectRatio > 0.0f && (flags & CSD_IGNORE_MAX_WIDTH) == 0)
	{
		width = widthMin * std::clamp(maxAspectRatio / g_aspectRatio, g_hudUnstretchX, 1.0f);
	}

	const float left = (widthMin - width) * 0.5f;

	return { left, left + width, 0.0f, heightMin };
}

// A point of the 640x480 layout and where that point should end up on screen.
// Each vertex then becomes anchor.Screen + (vertex - anchor.Layout) * unstretch.
struct CsdAnchor
{
	float Layout;
	float Screen;
};

static CsdAnchor ResolveAnchorX(uint64_t flags, float minX, float maxX)
{
	const CsdFrame frame = GetCsdFrame(flags);

	// freeze don't moveeee
	if ((flags & CSD_ALIGN_SELF_H) != 0)
	{
		const float centre = (minX + maxX) * 0.5f;
		return { centre, centre };
	}

	// centre is tested first on purpose. the corner combos carry an edge bit, so
	// adding CSD_ALIGN_CENTRE_H to a CSD_ALIGN_TOP_RIGHT entry leaves CSD_ALIGN_RIGHT
	// set too, and whichever we test first wins. asking for centre should mean centre.
	if ((flags & CSD_ALIGN_CENTRE_H) != 0)
		return { widthMin * 0.5f, widthMin * 0.5f };

	if ((flags & CSD_ALIGN_LEFT) != 0)
		return { 0.0f, frame.Left };

	if ((flags & CSD_ALIGN_RIGHT) != 0)
		return { widthMin, frame.Right };


	if ((flags & CSD_ALIGN_AUTO_H) != 0)
	{
		const float centre = (minX + maxX) * 0.5f;

		if (centre < widthMin / 3.0f)
			return { 0.0f, frame.Left };

		if (centre > widthMin * 2.0f / 3.0f)
			return { widthMin, frame.Right };
	}

	// centre keeps a layout whole, every quad moves to the same point
	return { widthMin * 0.5f, widthMin * 0.5f };
}

static CsdAnchor ResolveAnchorY(uint64_t flags, float minY, float maxY)
{
	const CsdFrame frame = GetCsdFrame(flags);

	if ((flags & CSD_ALIGN_CENTRE_V) != 0)
		return { heightMin * 0.5f, heightMin * 0.5f };

	if ((flags & CSD_ALIGN_TOP) != 0)
		return { 0.0f, frame.Top };

	if ((flags & CSD_ALIGN_BOTTOM) != 0)
		return { heightMin, frame.Bottom };

	if ((flags & CSD_ALIGN_AUTO_V) != 0)
	{
		const float centre = (minY + maxY) * 0.5f;

		if (centre < heightMin / 3.0f)
			return { 0.0f, frame.Top };

		if (centre > heightMin * 2.0f / 3.0f)
			return { heightMin, frame.Bottom };
	}

	return { heightMin * 0.5f, heightMin * 0.5f };
}

struct CsdSavedVertices
{
	bool Active{};
	float X[CSDVertexCount]{};
	float Y[CSDVertexCount]{};
};


static bool UnstretchPrimitive(void* vertexData, uint32_t count, uint64_t flags, CsdSavedVertices& saved,
	const std::optional<CsdAnchor>& anchorOverrideX = std::nullopt, const std::optional<CsdAnchor>& anchorOverrideY = std::nullopt)
{
	auto* verts = (RwD3D8Vertex*)vertexData;

	if (verts == nullptr || count == 0 || count > CSDVertexCount)
		return false;

	const bool forced = (flags & CSD_FORCE_UNSTRETCH) != 0;

	// we only touch a primitive its entry asked for. naming an alignment counts as asking,
	// so CSD_ALIGN_BOTTOM_LEFT on its own is enough.
	if ((flags & CSD_POSITIONING) == 0)
		return false;

	const bool unstretchX = g_hudUnstretchX < 1.0f;
	const bool unstretchY = g_hudUnstretchY < 1.0f;

	if (!unstretchX && !unstretchY)
		return false;

	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minY = FLT_MAX, maxY = -FLT_MAX;

	for (uint32_t i = 0; i < count; i++)
	{
		minX = std::min(minX, verts[i].x);
		maxX = std::max(maxX, verts[i].x);
		minY = std::min(minY, verts[i].y);
		maxY = std::max(maxY, verts[i].y);
	}

	// primitives covering the whole layout are backgrounds, fades and letterbox bars.
	// they are meant to cover the screen, so leave them stretched.
	if (!forced && minX <= 1.0f && maxX >= widthMin - 1.0f)
		return false;

	const CsdAnchor anchorX = anchorOverrideX.value_or(ResolveAnchorX(flags, minX, maxX));
	const CsdAnchor anchorY = anchorOverrideY.value_or(ResolveAnchorY(flags, minY, maxY));

	const float scaleX = unstretchX ? g_hudUnstretchX : 1.0f;
	const float scaleY = unstretchY ? g_hudUnstretchY : 1.0f;

	auto mapX = [&](float x) { return anchorX.Screen + (x - anchorX.Layout) * scaleX; };
	auto mapY = [&](float y) { return anchorY.Screen + (y - anchorY.Layout) * scaleY; };

	// the game hides elements by parking them off screen and letting the viewport clip them.
	// squeezing the UI can pull one of those back into view, so don't touch it.
	const bool wasHidden = maxX <= 0.0f || minX >= widthMin || maxY <= 0.0f || minY >= heightMin;

	if (wasHidden)
	{
		const bool nowVisible = mapX(maxX) > 0.0f && mapX(minX) < widthMin &&
			mapY(maxY) > 0.0f && mapY(minY) < heightMin;

		if (nowVisible)
			return false;
	}

	// Some UI elements persist to draw on ultrawide when they shoudn't (the game normally let them out of bound so we can't see them)
	// We can detect that and simply restore the original position to hide them again as a workaround.
	const float centerX = (minX + maxX) * 0.5f;
	const float centerY = (minY + maxY) * 0.5f;
	const bool shouldBeOutOfBound = !forced && ((centerX < 0.0f && mapX(0.0f) > 0.0f) ||
		(centerX > widthMin && mapX(widthMin) < widthMin) ||
		(centerY < 0.0f && mapY(0.0f) > 0.0f) || (centerY > heightMin && mapY(heightMin) < heightMin));

	for (uint32_t i = 0; i < count; i++)
	{
		saved.X[i] = verts[i].x;
		saved.Y[i] = verts[i].y;

		verts[i].x = mapX(shouldBeOutOfBound ? saved.X[0] : verts[i].x);
		verts[i].y = mapY(shouldBeOutOfBound ? saved.Y[0] : verts[i].y);

		// Edge case for black box on main menu where it stops around half of the screen at high resolution 
		// when it should normally take the whole background.
		const bool isOnEdge = saved.X[i] <= 1.0f || saved.X[i] >= widthMin - 1.0f;
		if ((flags & CSD_EXTEND_EDGE_H) != 0 && !shouldBeOutOfBound && isOnEdge)
			verts[i].x = saved.X[i];
	}

	saved.Active = true;

	return true;
}

static void RestorePrimitive(void* vertexData, uint32_t count, const CsdSavedVertices& saved)
{
	auto* verts = (RwD3D8Vertex*)vertexData;

	for (uint32_t i = 0; i < count; i++)
	{
		verts[i].x = saved.X[i];
		verts[i].y = saved.Y[i];
	}
}

void Draw(X86Context& ctx, uint8_t* base, X86Func* original, void* vertexData, uint32_t stride)
{
	CsdModifier modifier{};

	if (g_castModifier.has_value())
	{
		modifier = g_castModifier.value();
	}
	else if (g_castNodeModifier.has_value())
	{
		modifier = g_castNodeModifier.value();
	}
	else if (g_sceneModifier.has_value())
	{
		modifier = g_sceneModifier.value();
	}
	// Remove all flags if the aspect ratio is above 16:9.
	if ((modifier.Flags & CSD_ULTRAWIDE_ONLY) != 0 && g_aspectRatio <= WIDE_ASPECT_RATIO)
		modifier.Flags = CSD_NONE;

	if ((modifier.Flags & CSD_NARROW_ONLY) != 0 && g_aspectRatio >= WIDE_ASPECT_RATIO)
		modifier.Flags = CSD_NONE;

	// This is an OG Xbox bug workaround
	if ((modifier.Flags & CSD_PAUSE_DIM) != 0)
	{
		SetPauseDimAlpha(((RwD3D8Vertex*)vertexData)->emissiveColor >> 24);
	}

	if ((modifier.Flags & CSD_SKIP) != 0 || ((modifier.Flags & CSD_HIDE_PAUSE) != 0 && OptionsMenu::s_isVisible))
	{
		TogglePauseCSD((modifier.Flags & CSD_HIDE_PAUSE) != 0);
		TogglePauseBossCSD((modifier.Flags & CSD_PAUSE_BOSS) != 0);
		// Skip drawing this cast.
		return;
	}

	// we add entries to the pause menu, so its background has to grow
	// and everything under it moves down by the same amount.
	const bool isPauseBossWithExtendBottom = ((modifier.Flags & CSD_PAUSE_BOSS) != 0 && ((modifier.Flags & CSD_EXTEND_BOTTOM) != 0));

	uint8_t pauseItemCount = GetAddedPauseItemsCount();

	if (isPauseBossWithExtendBottom && pauseItemCount >= 2) // pause boss csd already have a big enough background to support 4 options
		pauseItemCount -= 1;

	const float pauseYOffset = round(15.f * (float)(pauseItemCount));
	const bool movesPause = (modifier.Flags & CSD_HIDE_PAUSE) != 0 &&
		(modifier.Flags & (CSD_EXTEND_BOTTOM | CSD_MOVE_BOTTOM)) != 0;

	// EXTEND_BOTTOM only pushes the bottom edge down, MOVE_BOTTOM moves the whole quad.
	auto offsetPause = [&](float sign)
		{
			auto* verts = (RwD3D8Vertex*)vertexData;

			for (int i = 0; i < CSDVertexCount; i++)
			{
				float delta = 0.0f;

				if ((modifier.Flags & CSD_EXTEND_BOTTOM) != 0 && (i == 1 || i == 3))
					delta += pauseYOffset;

				if ((modifier.Flags & CSD_MOVE_BOTTOM) != 0)
					delta += pauseYOffset;

				verts[i].y += delta * sign;
			}
		};

	if (movesPause)
		offsetPause(1.0f);

	CsdSavedVertices saved{};

	UnstretchPrimitive(vertexData, CSDVertexCount, modifier.Flags, saved);

	ctx.SubEsp(1);
	original(ctx, base);

	if (saved.Active)
		RestorePrimitive(vertexData, CSDVertexCount, saved);

	if (movesPause)
		offsetPause(-1.0f);
}

X86_FUNC_IMPL(__imp__sub_2ECC80);
void CPlatformRWDrawPrimitive2DMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	// ecx = vertex stride
	Draw(ctx, base, __imp__sub_2ECC80, g_memory.Translate(ctx.ecx.u32), 28);
}

void CPlatformRWDrawPrimitive2DMidAsmHook2(X86Context& __restrict ctx, uint8_t* base)
{
	// ecx = vertex stride
	Draw(ctx, base, __imp__sub_2ECC80, g_memory.Translate(ctx.esi.u32), 28);
}

// DrawNumberHud is also used on the stage select to show current record.
// With high res, they don't align properly so we have to manually patch them
// Each digit is a quad, so they end with a huge gap which we don't want.

static constexpr uint32_t hudNumberGlyphWidth = 0x598; // scaled
static constexpr uint32_t hudNumberGlyphScaleX = 0x08;
static constexpr uint32_t hudNumberGlyphHeight = 0x59C; // scaled
static constexpr uint32_t hudNumberGlyphScaleY = 0x0C;
static constexpr uint32_t hudNumberSpacing = 0x14;

enum HudNumberAlign : uint32_t
{
	HudNumberAlign_Left = 0,
	HudNumberAlign_Centre = 1,
	HudNumberAlign_Right = 2,
};

// DrawNumberHud is shared by everything that prints numbers: rings, timer, ammo etc. 
// One alignment can't fit them all, so each call site gets its own entry.

// Align is what the call passes: 0 puts x at the run's left edge, 1 at its centre, 2 at its
// right edge. It is part of the key because x alone does not place the digits.
struct HudNumberSite
{
	float X;
	float Y;
	uint32_t Align;
	uint64_t Flags;
};

static constexpr HudNumberSite g_hudNumberSites[] =
{
	// sub_27DE70, weapon ammo "%03d", one per screen half at the bottom
	{ 120.0f, 397.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_LEFT },
	{ 520.0f, 397.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_RIGHT },

	// DrawHudRingLives. Ring1P/Ring2P at the top, the ring/lives/key stack at the bottom left.
	// the stack shares one anchor so it can't split apart.
	{  84.0f,  68.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_TOP_LEFT },    // ring, Ring1P
	{ 560.0f,  68.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_TOP_RIGHT },   // ring, Ring2P
	{  78.0f, 395.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_LEFT }, // ring
	{  78.0f, 332.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_LEFT }, // lives / zanki
	{  78.0f, 266.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_LEFT }, // key count

	// TimerLogic. timer, main score and the two side scores, all pinned to the centre.
	{ 320.0f,  41.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE },  // timer, 1P and 2P
	{ 320.0f,  61.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE },  // score "%08d"
	{ 477.0f,  61.0f, HudNumberAlign_Left,   CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE },  // score "%07d", right slot
	{  54.0f,  61.0f, HudNumberAlign_Left,   CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE },  // score "%07d", left slot
	{ 320.0f, 100.0f, HudNumberAlign_Centre, CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE },  // timer, 2nd pass

};

static constexpr uint64_t HUD_NUMBER_DEFAULT_FLAGS = CSD_UNSTRETCH | CSD_ALIGN_AUTO_H;

static uint64_t GetHudNumberFlags(float x, float y, uint32_t align)
{
	for (const auto& site : g_hudNumberSites)
	{
		if (site.Align == align && site.X == x && site.Y == y)
			return site.Flags;
	}

	return HUD_NUMBER_DEFAULT_FLAGS;
}

struct HudNumberState
{
	uint64_t Flags{};
	std::optional<CsdAnchor> Anchor{};
	std::optional<CsdAnchor> AnchorY{};
};

static HudNumberState g_hudNumber{};
static bool g_isInStageMode = false;
static bool g_isDrawingSynopsisText = false;
static bool g_isDrawingLibraryText = false;
static bool g_isDrawingStaffRoll = false;

// The stage select CSD is squeezed towards the center, so the player's best time
// which drawn on top of it have to be moved to the same spot
static float StageSelectUnstretchX(float x)
{
	if (g_hudUnstretchX >= 1.0f)
		return x;

	return widthMin * 0.5f + (x - widthMin * 0.5f) * g_hudUnstretchX;
}

static float StageSelectUnstretchY(float y)
{
	if (g_hudUnstretchY >= 1.0f)
		return y;

	return heightMin * 0.5f + (y - heightMin * 0.5f) * g_hudUnstretchY;
}

static void BeginHudNumber(uint32_t font, float x, float y, uint32_t text, uint32_t align)
{
	g_hudNumber = {};

	if (font == 0 || text == 0 || (g_hudUnstretchX >= 1.0f && g_hudUnstretchY >= 1.0f))
		return;

	const uint64_t flags = GetHudNumberFlags(x, y, align);

	if ((flags & CSD_POSITIONING) == 0)
		return;

	auto base = g_memory.base;

	const auto* str = (const char*)g_memory.Translate(text);
	const float length = float(strlen(str));

	if (length <= 0.0f)
		return;

	// Same layout as DrawNumberHud: one glyph per char, plus a gap between them.
	const float glyphWidth = X86_LOAD_F32(font + hudNumberGlyphWidth) * X86_LOAD_F32(font + hudNumberGlyphScaleX);
	const float width = (length - 1.0f) * X86_LOAD_F32(font + hudNumberSpacing) + length * glyphWidth;

	const float left = x + (align == HudNumberAlign_Centre ? width * -0.5f : align == HudNumberAlign_Right ? -width : 0.0f);

	g_hudNumber.Flags = flags;

	// Stage select PBs text already draw in a centered 4:3 format
	// Patch it to match the CSD layout so it doesn't get misaligned
	if (g_isInStageMode)
	{
		const float glyphHeight = X86_LOAD_F32(font + hudNumberGlyphHeight) * X86_LOAD_F32(font + hudNumberGlyphScaleY);
		const float bottom = y + glyphHeight;

		g_hudNumber.Anchor = CsdAnchor{ x, StageSelectUnstretchX(x) };
		g_hudNumber.AnchorY = CsdAnchor{ bottom, StageSelectUnstretchY(bottom) };
		return;
	}

	// one anchor for the whole string so the digits keep their spacing
	g_hudNumber.Anchor = ResolveAnchorX(flags, left, left + width);
}

// DrawNumberHud (this, x, y, text, align)
X86_FUNC_IMPL(__imp__sub_281550);
X86_OVERRIDE_FUNC(sub_281550)
{
	BeginHudNumber(ctx.ecx.u32, X86_LOAD_F32(ctx.esp.u32 + 4), X86_LOAD_F32(ctx.esp.u32 + 8),
		X86_LOAD_U32(ctx.esp.u32 + 0xC), X86_LOAD_U32(ctx.esp.u32 + 0x10));

	__imp__sub_281550(ctx, base);

	g_hudNumber = {};
}

void DrawHudMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (!ctx.eax.u32 || App::isInMainMenu() && !g_isInStageMode) // this affect text on stage select mode badly
		return;

	// the digits are drawn right away, so there is nothing to restore after
	CsdSavedVertices saved{};
	UnstretchPrimitive(g_memory.Translate(ctx.eax.u32), CSDVertexCount, g_hudNumber.Flags, saved, g_hudNumber.Anchor, g_hudNumber.AnchorY);
}


// the enemy HP bar isn't like regular HUD elements: every frame the game projects the enemy's world position
// to the screen (see sub_285930 and sub_2D5D0) and places the bar wherever the enemy is.
// The projection uses the camera's own fov, which we never widen. thus, we cannot patch it at vertex levels,
// so we will do it the old way.

// squeeze a projected screen position (x, y floats) around the center of the screen rect (x, y, w, h ints).
static void UnstretchProjectedPoint(uint32_t screenRect, uint32_t screenPos)
{
	auto base = g_memory.base;

	if (g_hudUnstretchX < 1.0f)
	{
		float rectX = float(int32_t(X86_LOAD_U32(screenRect)));
		float rectW = float(int32_t(X86_LOAD_U32(screenRect + 0x8)));
		float centreX = rectX + rectW * 0.5f;

		X86_STORE_F32(screenPos, centreX + (X86_LOAD_F32(screenPos) - centreX) * g_hudUnstretchX);
	}

	if (g_hudUnstretchY < 1.0f)
	{
		float rectY = float(int32_t(X86_LOAD_U32(screenRect + 0x4)));
		float rectH = float(int32_t(X86_LOAD_U32(screenRect + 0xC)));
		float centreY = rectY + rectH * 0.5f;

		X86_STORE_F32(screenPos + 0x4, centreY + (X86_LOAD_F32(screenPos + 0x4) - centreY) * g_hudUnstretchY);
	}
}

void HpBarProjectionMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	// rEax = SonicteamUSA::System::Camera::GetScreenRectangle(screenType)
	// esi + 0x2C = projected screen X / Y, still to be culled against the screen rect below
	UnstretchProjectedPoint(ctx.eax.u32, ctx.esi.u32 + 0x2C);
}

// the game scales the bar against a fixed 640x480 space (rect.w * scale / 640, rect.h * scale / 480),
// and the 2D layer then stretches that space over the whole viewport, so like everything else here
// the bar comes out too wide on wide screens (too tall on narrow ones). squeeze it back to square pixels.
void HpBarScaleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	// esi + 0x38 = scale X, esi + 0x3C = scale Y, consumed by the CSD SetScale call further down.
	if (g_hudUnstretchX < 1.0f)
		X86_STORE_F32(ctx.esi.u32 + 0x38, X86_LOAD_F32(ctx.esi.u32 + 0x38) * g_hudUnstretchX);

	if (g_hudUnstretchY < 1.0f)
		X86_STORE_F32(ctx.esi.u32 + 0x3C, X86_LOAD_F32(ctx.esi.u32 + 0x3C) * g_hudUnstretchY);
}

// Item reward icon from CP
// sub_289970 builds a quad which gets stretched as resolution increase.
// the quad is rebuilt every frame, so we can unstretch it right before the draw
static constexpr uint32_t g_itemIconVertices = 0x559688;

void ItemIconDrawMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	CsdSavedVertices saved{};
	auto* verts = (RwD3D8Vertex*)g_memory.Translate(g_itemIconVertices);

	// handle P3/P4 centers
	if (AdjustMultiplayerItemIcon(ctx))
	{
		float minY = FLT_MAX, maxY = -FLT_MAX;
		for (uint32_t i = 0; i < CSDVertexCount; i++)
		{
			minY = std::min(minY, verts[i].y);
			maxY = std::max(maxY, verts[i].y);
		}
		const float centerY = (minY + maxY) * 0.5f;
		UnstretchPrimitive(verts, CSDVertexCount, CSD_UNSTRETCH | CSD_ALIGN_SELF_H, saved, std::nullopt, CsdAnchor{ centerY, centerY });
		return;
	}
	UnstretchPrimitive(verts, CSDVertexCount, CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE, saved);
}


// Weapon lock-on reticle (This includes birbo targets in the ark)
// Those have the same problem than enemy hp bar above
void LockOnProjectionMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	//eax is camera's screen rectangle
	// esp + 8 is the project screen X, esp + 0xC the Y
	UnstretchProjectedPoint(ctx.eax.u32, ctx.esp.u32 + 0x8);
}

// sub_268FD0 creates one reticle (quad) which isn't called, so it's safe to patch the corners here
static constexpr uint32_t LockOnStride = 28u;
static constexpr uint32_t LockOnVertexCount = 4u;
X86_FUNC_IMPL(__imp__sub_268FD0);
X86_OVERRIDE_FUNC(sub_268FD0)
{
	uint32_t vertices = X86_LOAD_U32(ctx.esp.u32 + 4);
	uint32_t centre = X86_LOAD_U32(ctx.esp.u32 + 8); //projected center the corners are offset from

	// 3P / 4P view scale
	X86_STORE_F32(ctx.esp.u32 + 0xC, X86_LOAD_F32(ctx.esp.u32 + 0xC) * GetMultiplayerViewScale());

	__imp__sub_268FD0(ctx, base);

	if (!vertices || !centre)
		return;

	if (g_hudUnstretchX < 1.0f)
	{
		float centreX = X86_LOAD_F32(centre);

		for (uint32_t i = 0; i < LockOnVertexCount; i++)
		{
			uint32_t vertexX = vertices + i * LockOnStride;
			X86_STORE_F32(vertexX, centreX + (X86_LOAD_F32(vertexX) - centreX) * g_hudUnstretchX);
		}
	}

	if (g_hudUnstretchY < 1.0f)
	{
		float centreY = X86_LOAD_F32(centre + 0x4);

		for (uint32_t i = 0; i < LockOnVertexCount; i++)
		{
			uint32_t vertexY = vertices + i * LockOnStride + 0x4;
			X86_STORE_F32(vertexY, centreY + (X86_LOAD_F32(vertexY) - centreY) * g_hudUnstretchY);
		}
	}
}

// Logo shown in the bottom right corner during fade out.
// It's stretched at high res, so scale it back properly.
static constexpr uint32_t FadeLogoVertices = 0x484;
X86_FUNC_IMPL(__imp__sub_19730);
X86_OVERRIDE_FUNC(sub_19730)
{
	void* vertices = g_memory.Translate(ctx.ecx.u32 + FadeLogoVertices);

	CsdSavedVertices saved{};
	UnstretchPrimitive(vertices, CSDVertexCount, CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_RIGHT | CSD_IGNORE_MAX_WIDTH, saved);

	__imp__sub_19730(ctx, base);

	if (saved.Active)
		RestorePrimitive(vertices, CSDVertexCount, saved);
}

// Button prompt above air saucers, checkpoints etc.
// This is originally a simple CSD, but the game places it like the enemy HP bar: it projects as if the screen was 4:3,
// so it looks wrong at high resolution. The idea is to fix the position before the cull and squeeze 
// the scale X / Y value directly instead of the vertex.

// Memo: eax = GetScreenRectangle(0), esp + 0x10 = projected screen X, esp + 0x14 = Y
void VHArrowProjectionMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	UnstretchProjectedPoint(ctx.eax.u32, ctx.esp.u32 + 0x10);
}

// split screen variant for MP
// Memo: esp+0xC = projected screen X, esp+0x10 = Y, eax = GetScreenRectangle(screen).
void VHArrowProjectionSplitMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	UnstretchProjectedPoint(ctx.eax.u32, ctx.esp.u32 + 0xC);
}

// Memo: esp = scale X pushed for sub_2BBB40, esp+4 = scale Y
void VHArrowScaleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (g_hudUnstretchX < 1.0f)
		X86_STORE_F32(ctx.esp.u32, X86_LOAD_F32(ctx.esp.u32) * g_hudUnstretchX);

	if (g_hudUnstretchY < 1.0f)
		X86_STORE_F32(ctx.esp.u32 + 0x4, X86_LOAD_F32(ctx.esp.u32 + 0x4) * g_hudUnstretchY);
}


// the game often use a quad model to draw a picture on the screen, usually to show Shadow memories or space background, 
// but the quad model is not stretched to fit the screen, thus any high resolution won't cover the whole screen
// we can fix that by stretching the vertex needed.
// note: this is inconsistent, some cutscenes have the model stretched enough to cover width but not height, some the opposite, and sometimes both are wrong.

struct QuadStretchCalibration
{
	float calibrationX = 1.0f; // horizontal multiplier at 16:9
	float calibrationY = 1.0f; // aspect independent
};

struct PictureDff
{
	const char* name;
	uint64_t fileHash;
	QuadStretchCalibration calibration;
};

thread_local const QuadStretchCalibration* g_currentQuadCallib = nullptr;

static const PictureDff g_pictureDff[] =
{
	{ "ARK!PICTURE!O!.DFF", 0x8A9AA0AC9ADA2165ull, { 2.2f, 1.2f } }, // ev 6301 cosmic fall intro (quad lies in XZ, height is verts.z)
	{ "ARK!PICTURE!LOCATOR.DFF", 0x8C3EC1BA538F07C2ull, { 2.2f, 1.2f } }, // ev 6301
	{ "FLASH!BACK!PIC01!O!.DFF", 0xF8980C50AA4AFEE6ull, { 1.6f, 1.1f } }, // ev 6201 (lava shelter intro)
	{ "FLASH!BACK!PIC01!O!.DFF", 0x41D1756A26AD0E6Cull, { 1.6f, 1.1f } }, // ev 6202
	{ "FLASH!BACK!PIC01!O!.DFF", 0x59679B7EFC7F6BEBull, { 1.6f, 1.1f } }, // ev 6203
	{ "FLASH!BACK!PIC01!O!.DFF", 0xA62A2381EB93B972ull, { 1.6f, 1.1f } }, // ev 4111
	{ "FLASH!BACK!PIC01!O!.DFF", 0xB135381ACF0E7DE1ull, { 1.6f, 1.1f } }, // ev 6101, 6103
	{ "FLASH!BACK!PIC01!O!.DFF", 0xA56875E819163AE5ull, { 1.6f, 1.1f } }, // ev 6104
	{ "FLASH!BACK!PIC02!O!.DFF", 0xC8FD545ED49E7F5Bull, { 1.6f, 1.1f } }, // ev 6201
	{ "FLASH!BACK!PIC02!O!.DFF", 0x9E8CC36B927DD50Bull, { 1.6f, 1.1f } }, // ev 6202
	{ "FLASH!BACK!PIC02!O!.DFF", 0x3BE8029CA9923AF6ull, { 1.6f, 1.1f } }, // ev 6203
	{ "FLASH!BACK!PIC03!O!.DFF", 0xB7C87812CC4E3F43ull, { 1.6f, 1.1f } }, // ev 6201
	{ "FLASH!BACK!PIC03!O!.DFF", 0x0AA5C34F51028FB3ull, { 1.6f, 1.1f } }, // ev 6202
	{ "FLASH!BACK!PIC03!O!.DFF", 0xC4936BC822FDCB50ull, { 1.6f, 1.1f } }, // ev 6203
	{ "FLASH!BACK!PIC04!O!.DFF", 0x5CBBFEF84D845D91ull, { 1.6f, 1.1f } }, // ev 6201
	{ "FLASH!BACK!PIC04!O!.DFF", 0x31BFE6FC220B1A15ull, { 1.6f, 1.1f } }, // ev 6202
	{ "FLASH!BACK!PIC04!O!.DFF", 0xC11A58D21E2D84C2ull, { 1.6f, 1.1f } }, // ev 6203
	{ "ADV2!FLASH!BACK!PIC01!O!.DFF", 0x5F3EEEEED08941D7ull, { 1.3f, 1.1f } }, // ev 5302
	{ "ADV2!FLASH!BACK!PIC02!O!.DFF", 0x5ED6F088DF43A2EFull, { 1.3f, 1.1f } }, // ev 5302
	{ "ADV2!FLASH!BACK!PIC03!O!.DFF", 0xB2F0E6B57EFE21ADull, { 1.3f, 1.1f } }, // ev 5302
	{ "ADV2!FLASH!BACK!PIC04!O!.DFF", 0x04AEF24F2506F58Dull, { 1.3f, 1.1f } }, // ev 5302
	{ "HEROS!FLASH!BACK!PICA!O!.DFF", 0xB75EE7F97968549Cull, { 1.2f, 1.1f } }, // ev 3101 prison island intro
	{ "HEROS!FLASH!BACK!PICB!O!.DFF", 0x4020BD6E58EDBBA0ull, { 1.2f, 1.1f } }, // ev 3101 prison island intro
	{ "BD!SHIP!PIC01!O!.DFF", 0x36AA13F84C89A7E8ull, { 1.5f, 1.1f } }, // ev 4202 sky troops intro
	{ "EGG!SHIP!PIC01!O!.DFF", 0x80B5E4F74190CB01ull, { 1.5f, 1.1f } }, // ev 4202 sky troops intro
	{ "EGG!SHIP!PIC02!O!.DFF", 0x1627CF293E62388Eull, { 1.5f, 1.1f } }, // ev 4202 sky troops intro
	{ "SPACE!PIC!O!.DFF", 0x1F6FEF680246A77Dull, { 1.3f, 1.2f } }, // ev 8204 epilogue
};


// Helper function to stretch picture vertex in cutscenes that need it
// We can use the value tuned at 16:9 and re solve it for the current aspect ratio. (at 4:3 the stretch off to 0.75, at 16:10 it goes to 0.9375 etc.)

static float PictureStretchX(float calibrationX)
{
	if (g_aspectRatio <= 0.0f) // ComputeOffsets didn't run yet
		return calibrationX;

	// make sure to never go below 1.0f, the quads only need growing.
	return std::max(1.0f, calibrationX * (g_aspectRatio / WIDE_ASPECT_RATIO));
}

void FindPictureBackgroundModel(X86Context* ctx, uint8_t* base)
{
	g_currentQuadCallib = nullptr;
	auto pOneFileAsync = (TOneFileAsync*)g_memory.Translate(ctx->ecx.u32);
	if (!pOneFileAsync)
	{
		return;
	}

	auto pOneFile = pOneFileAsync->pOneFile.get();
	if (!pOneFile)
	{
		return;
	}

	auto assetName = TOneFileCheckFileName(pOneFile, pOneFileAsync->fid);
	auto data = pOneFileAsync->dest.get();
	uint32_t size = pOneFileAsync->_mem.length;
	if (!assetName || !data || size == 0)
	{
		return;
	}

	std::optional<XXH64_hash_t> fileHash;
	for (const auto& pic : g_pictureDff)
	{
		if (strncmp(assetName, pic.name, 0x2C) != 0)
		{
			continue;
		}

		if (!fileHash)
		{
			fileHash = XXH3_64bits(data, size);
		}

		if (*fileHash == pic.fileHash)
		{
			g_currentQuadCallib = &pic.calibration;
			return;
		}
	}
}


void StretchPictureVertex(uint32_t atomicAddr, uint8_t* base)
{
	if (!g_currentQuadCallib)
		return;

	auto geometryPtr = X86_LOAD_U32(atomicAddr + 0x18);
	RpGeometry* geometry = (RpGeometry*)g_memory.Translate(geometryPtr);

	// pic data is always 4 vertex
	if (!geometry || geometry->numVertices != 4 || geometry->numMorphTargets != 1)
		return;

	auto morphTargetPtr = geometry->morphTarget.get();

	if (morphTargetPtr)
	{
		morphTargetPtr = morphTargetPtr;

		if (morphTargetPtr)
		{
			RwV3d* verts = morphTargetPtr->verts.get();

			// We can't blindly stretch the same vertex to stretch the models,
			// the in-plane vertical axis is whichever of Y/Z actually spans, the other one is 
			// the plane normal and is constant with all four corners.
			float minY = verts[0].y;
			float maxY = verts[0].y;
			float minZ = verts[0].z;
			float maxZ = verts[0].z;

			for (int i = 1; i < geometry->numVertices; i++)
			{
				minY = std::min(minY, verts[i].y);
				maxY = std::max(maxY, verts[i].y);
				minZ = std::min(minZ, verts[i].z);
				maxZ = std::max(maxZ, verts[i].z);
			}

			const bool verticalIsZ = (maxZ - minZ) > (maxY - minY);
			const float stretchX = PictureStretchX(g_currentQuadCallib->calibrationX);
			const float stretchY = g_currentQuadCallib->calibrationY;

			for (int i = 0; i < geometry->numVertices; i++)
			{
				verts[i].x *= stretchX;
				(verticalIsZ ? verts[i].z : verts[i].y) *= stretchY;
			}

			const float radiusScale = std::max(stretchX, stretchY);

			if (radiusScale > 1.0f)
				morphTargetPtr->boundingSphere.radius *= radiusScale;
		}
	}
}

// callbacks used by RpClumpForAllAtomics
// we can use them to stretch the vertex of the picture model
X86_FUNC_IMPL(__imp__sub_29EB20);
X86_OVERRIDE_FUNC(sub_29EB20)
{
	if (ctx.esp.u32 + 4)
	{
		StretchPictureVertex(X86_LOAD_U32(ctx.esp.u32 + 4), base);
	}
	__imp__sub_29EB20(ctx, base);
}

X86_FUNC_IMPL(__imp__sub_261A0);
X86_OVERRIDE_FUNC(sub_261A0)
{
	if (ctx.esp.u32 + 4)
	{
		StretchPictureVertex(X86_LOAD_U32(ctx.esp.u32 + 4), base);
	}
	__imp__sub_261A0(ctx, base);
}

X86_FUNC_IMPL(__imp__sub_25DF0);
X86_OVERRIDE_FUNC(sub_25DF0)
{
	if (ctx.esp.u32 + 4)
	{
		StretchPictureVertex(X86_LOAD_U32(ctx.esp.u32 + 4), base);
	}
	__imp__sub_25DF0(ctx, base);
}

// TOneFileAsyncRpClump::Read
X86_FUNC_IMPL(__imp__sub_1FE00);
X86_OVERRIDE_FUNC(sub_1FE00)
{
	auto mode = X86_LOAD_U32(ctx.ecx.u32 + 0x28);
	if (mode == 4)
	{
		FindPictureBackgroundModel(&ctx, base);
	}

	__imp__sub_1FE00(ctx, base);

	if (mode >= 4)
	{
		g_currentQuadCallib = nullptr;
	}
}

// -------------- CSD MODIFIERS --------------- //

const xxHashMap<CsdModifier> g_csdModifiers =
{
	// Loading
	{ HashStr("csdFiles/Loading_EN.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT | CSD_IGNORE_MAX_WIDTH } },
	{ HashStr("csdFiles/Loading_FR.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT | CSD_IGNORE_MAX_WIDTH } },
	{ HashStr("csdFiles/Loading_GE.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT | CSD_IGNORE_MAX_WIDTH } },
	{ HashStr("csdFiles/Loading_IT.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT | CSD_IGNORE_MAX_WIDTH } },
	{ HashStr("csdFiles/Loading_JP.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT | CSD_IGNORE_MAX_WIDTH } },
	{ HashStr("csdFiles/Loading_SP.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT | CSD_IGNORE_MAX_WIDTH } },

	// intro logo
	{ HashStr("csdFiles/Logo.xncp/XBox/sega"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Logo.xncp/XBox/sonicteam"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Logo.xncp/XBox/dolby"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	// Title Screen
	{ HashStr("csdFiles/honban.xncp/Title"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Loop/ultimate"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT | CSD_IGNORE_MAX_WIDTH } },

	// the logo animation stretch and take the whole screen which would treat it as "background", so we force unstretch mode to keep the scaled size.
	{ HashStr("csdFiles/honban.xncp/Title/title2"), { CSD_CENTER_UNSTRETCH | CSD_FORCE_UNSTRETCH } },
	{ HashStr("csdFiles/honban.xncp/Title/title3"), { CSD_CENTER_UNSTRETCH | CSD_FORCE_UNSTRETCH } },
	{ HashStr("csdFiles/honban.xncp/Title/title4"), { CSD_CENTER_UNSTRETCH | CSD_FORCE_UNSTRETCH } },
	{ HashStr("csdFiles/honban.xncp/Title/title5"), { CSD_CENTER_UNSTRETCH | CSD_FORCE_UNSTRETCH } },
	{ HashStr("csdFiles/honban.xncp/Title/title6"), { CSD_CENTER_UNSTRETCH | CSD_FORCE_UNSTRETCH } },

	{ HashStr("csdFiles/Synopsis.xncp/Button"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_RIGHT } },

	// Main Menu
	{ HashStr("csdFiles/honban.xncp/Menu/logo"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/title"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/1pgame"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/2pgame"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/options"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/storymode"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/selectmode"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/hardmode"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/new"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/resume"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/back_button"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/back_button/back"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/select_button"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/honban.xncp/Menu/select_button/select"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },

	// Library (also contains menu for new game / continue / last story / expert)
	{ HashStr("csdFiles/Story_Library.xncp/StoryMode/Window"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Story_Library.xncp/StoryRoute/StoryRoute"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Story_Library.xncp/StoryRoute/StoryRoute/blkcover"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE | CSD_EXTEND_EDGE_H } }, // black box low opacity that needs special fix 
	{ HashStr("csdFiles/Story_Library.xncp/StoryRoute/SRIcons"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Story_Library.xncp/StoryRoute/SRIconsBG"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Story_Library.xncp/Buttons/Buttons"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Story_Library.xncp/Library/PageNumber"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Story_Library.xncp/Library/WindowLib"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Story_Library.xncp/Background/Background"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT | CSD_IGNORE_MAX_WIDTH } }, // "LIBRARY" Text in the corner

	// Stage select mode
	{ HashStr("csdFiles/SelectMode.xncp/SelectMode"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/SelectMode.xncp/SelectMode/bg"), { CSD_NONE } },
	{ HashStr("csdFiles/SelectMode.xncp/SelectMode/title"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT | CSD_IGNORE_MAX_WIDTH } },
	{ HashStr("csdFiles/SelectMode.xncp/SelectMode/text_bg_top"), { CSD_NONE } },
	{ HashStr("csdFiles/SelectMode.xncp/SelectMode/text_bg_bottom"), { CSD_NONE } },
	{ HashStr("csdFiles/SelectMode.xncp/SelectMode/text_bg_bottom/bg2"), { CSD_NONE } },
	{ HashStr("csdFiles/SelectMode.xncp/SelectMode/title"), { CSD_UNSTRETCH | CSD_ALIGN_SELF_H } },
	{ HashStr("csdFiles/SelectMode.xncp/IconsBG"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/SelectMode.xncp/Star"), { CSD_CENTER_UNSTRETCH } },


	// stage card
	{ HashStr("csdFiles/stageTitle/StageTitle.xncp/StageTitle"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },


	//Battle mode (2p menu)
	{ HashStr("csdFiles/BattleMode.xncp/Kazari"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT | CSD_IGNORE_MAX_WIDTH } }, // Battle mode in the corner
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_box/box_chara"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_box/box_chara/box1"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_box/box_chara/box2"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_box/box_chara/box3"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_box/box_chara/box4"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_box/box_chara/box5"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_box/box_chara/box6"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_chara/bg_1p"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_chara/bg_2p"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_chara/tx_1p"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Kazari/Null_chara/tx_2p"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	{ HashStr("csdFiles/BattleMode.xncp/1PCharaSelect"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/2PCharaSelect"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Cursor1P"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Cursor2P"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	{ HashStr("csdFiles/BattleMode.xncp/Arena"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Buttons/select_button"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Buttons/select_button/select"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Buttons/back_button"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BattleMode.xncp/Buttons/back_button/back"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },

	{ HashStr("csdFiles/RouteDisplay.xncp/StoryRoute/IconsBG"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/RouteDisplay.xncp/StoryRoute/Pics"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/RouteDisplay.xncp/StoryRoute/Arrows"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	// game over
	{ HashStr("csdFiles/GameOver.xncp/GameOver"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/GameOver.xncp/Loop"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },

	// mission failed
	{ HashStr("csdFiles/MissionFailed_EN.xncp/mf"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/MissionFailed_FR.xncp/mf"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/MissionFailed_GE.xncp/mf"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/MissionFailed_IT.xncp/mf"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/MissionFailed_JP.xncp/mf"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/MissionFailed_SP.xncp/mf"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	// mission result
	{ HashStr("csdFiles/Result_EN.xncp/Main"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_EN.xncp/Main/result"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT } },
	{ HashStr("csdFiles/Result_EN.xncp/Rank"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_FR.xncp/Main"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_FR.xncp/Main/result"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT } },
	{ HashStr("csdFiles/Result_FR.xncp/Rank"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_GE.xncp/Main"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_GE.xncp/Main/result"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT } },
	{ HashStr("csdFiles/Result_GE.xncp/Rank"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_SP.xncp/Main"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_SP.xncp/Main/result"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT } },
	{ HashStr("csdFiles/Result_SP.xncp/Rank"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_IT.xncp/Main"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_IT.xncp/Main/result"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT } },
	{ HashStr("csdFiles/Result_IT.xncp/Rank"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_JP.xncp/Main"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("csdFiles/Result_JP.xncp/Main/result"), { CSD_UNSTRETCH | CSD_ALIGN_LEFT } },
	{ HashStr("csdFiles/Result_JP.xncp/Rank"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	// pause
	{ HashStr("csdFiles/pause/pause.xncp/Pause/PauseText"), { CSD_SKIP | CSD_HIDE_PAUSE } },
	{ HashStr("csdFiles/pause/pauseBoss.xncp/Pause/PauseText"), { CSD_SKIP | CSD_HIDE_PAUSE | CSD_PAUSE_BOSS } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Pause/Null_pause/bottom"), { CSD_HIDE_PAUSE | CSD_EXTEND_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Pause/Null_pause/bg"), { CSD_HIDE_PAUSE | CSD_EXTEND_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pauseBoss.xncp/Pause/Pause/Null_pause/bottom"), { CSD_HIDE_PAUSE | CSD_EXTEND_BOTTOM | CSD_PAUSE_BOSS | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pauseBoss.xncp/Pause/Pause/Null_pause/bg"), { CSD_HIDE_PAUSE | CSD_EXTEND_BOTTOM | CSD_PAUSE_BOSS | CSD_CENTER_UNSTRETCH } },


	{ HashStr("csdFiles/pause/pause.xncp/Pause/Pause"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/black_cover"), { CSD_CENTER_UNSTRETCH | CSD_PAUSE_DIM } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission_Board"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconDark"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconHero"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconNormal"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/ChangeMission"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/SRText"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pauseBoss.xncp/Pause/Pause"), { CSD_CENTER_UNSTRETCH } },


	// pause story route mode
	{ HashStr("csdFiles/pause/pause.xncp/StoryRoute"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/StoryRoute/StoryRoute"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/StoryRoute/SRIconsBG"), { CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/StoryRoute/SRIcons"), { CSD_CENTER_UNSTRETCH } },


	// this is to move the mission board vertex below due to the pause menu bg being extended reaching the mission board position
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission_Board/Null_0189/bg1"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission_Board/Null_0189/bg2"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission_Board/Null_0189/bg3"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission_Board/Null_0189/bg4"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission_Board/Null_missionname"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/bg"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/line_top2"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/line_top"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/frame_edge"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/frame_edge___R_s_[(1)"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/frame_edge___R_s_["), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/frame_right"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/frame_left"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/line_side_left"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/line_side___R_s_["), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/Null_0190/line_side_left___R_s_["), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/iconshadow1"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/iconshadow2"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/iconshadow3"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/Mission/mission"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconHero/highlight"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconHero/hero"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconNormal/highlight"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconNormal/normal"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconDark/dark"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/MIconDark/highlight"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/ChangeMission/change"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/ChangeMission/arrow_left"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/ChangeMission/arrow_right"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/SRText/text"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },
	{ HashStr("csdFiles/pause/pause.xncp/Pause/SRText/button"), { CSD_HIDE_PAUSE | CSD_MOVE_BOTTOM | CSD_CENTER_UNSTRETCH } },

	// HUD widgets. these are small and stay in one corner, so the auto guess works fine

	// Egg Dealer roulette UI
	{ HashStr("csdFiles/EggBoss.xncp/DisplaySml"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT } },
	{ HashStr("csdFiles/EggBoss.xncp/1"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT } },
	{ HashStr("csdFiles/EggBoss.xncp/2"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT } },
	{ HashStr("csdFiles/EggBoss.xncp/3"), { CSD_UNSTRETCH | CSD_ALIGN_RIGHT } },
	{ HashStr("csdFiles/EggBoss.xncp/DisplayLg"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE_H } },

	// InfoWindow.xncp
	{ HashStr("csdFiles/InfoWindow.xncp/Ring"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/InfoWindow.xncp/Ring/Null_0000"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/InfoWindow.xncp/Ring/Null_0000/ring"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/InfoWindow.xncp/Zanki"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/InfoWindow.xncp/Zanki/Null_0000"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/InfoWindow.xncp/Zanki/Null_0000/key"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/InfoWindow.xncp/Zanki/Null_0000/zanki"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },

	// CommandWindow.xncp
	{ HashStr("csdFiles/CommandWindow.xncp/CarMeter"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/CarMeter/outline"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/CarMeter/meter_bg"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/CarMeter/meter"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Car_BCom"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Car_BCom/Null"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Car_BCom/Null/button2"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Car_BCom/Null/icon_car"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Weapon"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Weapon/weapon"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Weapon/weapon/button1"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Weapon/weapon/icon_gun"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Window"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Window/window"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Window/window/window2"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Window/window/window1"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Window/window/info_bg2"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/CommandWindow.xncp/Window/window/info_bg"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },

	// HeroGauge.xncp
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000/bar_bg"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000/meter_main1"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000/meter_main2"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000/meter_main3"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000/bar"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000/meter_RSide"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/HeroGauge.xncp/Scene_0000/meter_side"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },

	// DarkGauge.xncp
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000/meter_bg"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000/main1"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000/main2"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000/main3"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000/meter"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000/side1"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/DarkGauge.xncp/Scene_0000/side2"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },

	// BossGauge.xncp
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000/bg"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000/m1"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000/m2"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000/m3"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000/meter"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000/left_edge"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },
	{ HashStr("csdFiles/BossGauge.xncp/Scene_0000/right_edge"), { CSD_UNSTRETCH | CSD_ALIGN_BOTTOM_CENTRE } },

	// Hissatu_Hero.xncp
	{ HashStr("csdFiles/Hissatu_Hero.xncp/HissatuHero/XButton"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/Hissatu_Hero.xncp/HissatuHero/XButton/Eff_X"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },

	// Hissatu_Dark.xncp
	{ HashStr("csdFiles/Hissatu_Dark.xncp/HissatuDark/YButton"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },
	{ HashStr("csdFiles/Hissatu_Dark.xncp/HissatuDark/YButton/Eff_Y"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE } },

	// TWindow.xncp
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetDark/Null_dark"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetDark/Null_dark/dark_icon"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetDark/Null_dark/Cast_0005"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetDark/Null_dark/Cast_0005/Cast_0006"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetHero"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetHero/Null_hero"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetHero/Null_hero/hero_icon"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetHero/Null_hero/Cast_0003"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/TWindow.xncp/TargetWindow/TargetHero/Null_hero/Cast_0003/Cast_0004"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },

	// 2P_Ring.xncp
	{ HashStr("csdFiles/2P_Ring.xncp/Ring1P/Null_ring/ring"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_TOP } },
	{ HashStr("csdFiles/2P_Ring.xncp/Ring2P/Null_ring/ring"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_TOP } },

	{ HashStr("csdFiles/2P_Star.xncp/Star"), { CSD_UNSTRETCH | CSD_ALIGN_TOP_CENTRE} },
	{ HashStr("csdFiles/2P_Start.xncp/Start"), { CSD_UNSTRETCH | CSD_FORCE_UNSTRETCH} },
	{ HashStr("csdFiles/2P_Result.xncp/Result"), { CSD_UNSTRETCH | CSD_CENTER_UNSTRETCH | CSD_FORCE_UNSTRETCH} },
	{ HashStr("csdFiles/2P_Result.xncp/TimeUp"), { CSD_UNSTRETCH | CSD_CENTER_UNSTRETCH | CSD_FORCE_UNSTRETCH} },

	// 2P_CommandWindow.xncp
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Window/window/info_bg"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Window/window/info_bg2"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Window/window/window1"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Window/window/window2"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Weapon/weapon/icon_gun"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Weapon/weapon/button1"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Car_BCom/Null/icon_car"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/Car_BCom/Null/button2"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/CarMeter/outline"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/CarMeter/meter_bg"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },
	{ HashStr("csdFiles/2P_CommandWindow.xncp/CarMeter/meter"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H | CSD_ALIGN_BOTTOM } },


	// Checkpoint warp menu, no path here since those CSDs are loaded from the stage .one archive directly
	{ HashStr("CHECKPOINT5.XNCP/Box"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT5.XNCP/Icons"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT5.XNCP/Shadow"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT6.XNCP/Box"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT6.XNCP/Icons"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT6.XNCP/Shadow"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT7.XNCP/Box"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT7.XNCP/Icons"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT7.XNCP/Shadow"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT8.XNCP/Box"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT8.XNCP/Icons"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("CHECKPOINT8.XNCP/Shadow"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },

	// Partner intro name (NpcNameDisplay). each partner has its own file with the same layout
	{ HashStr("csdFiles/Mchara_sonic.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_tails.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_knuckles.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/MChara_amy.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_rouge.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_omega.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_vector.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_espio.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/MChara_charmy.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_maria.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/Mchara_eggman.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/MChara_doomeye.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/MChara.xncp/MissionCharacters"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },

	// Bosses intro name
	{ HashStr("csdFiles/BChara_BBull.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/BChara_EBreaker.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/BChara_HDog.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/BChara_BFalcon.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/BChara_SandD.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/BChara_BDoom.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/BChara_EDealer.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },
	{ HashStr("csdFiles/BChara_DevDoom.xncp/bchara"), { CSD_UNSTRETCH | CSD_ALIGN_AUTO_H } },

	// Circus Park (stg0302/stg0302_gdt.one)
	// gong, firering, minigame rewards 5, 15, 50, -50, and the bomb/ring fly in anims
	{ HashStr("PR0302RINGANM.XNCP/Scene_0000"), { CSD_UNSTRETCH | CSD_ALIGN_CENTRE } },
	{ HashStr("PR0302RINGANM.XNCP/Scene_0000/Null_0015"), { CSD_UNSTRETCH | CSD_ALIGN_SELF_H } },
	{ HashStr("PR0302RINGANM.XNCP/Scene_0000/Null_0015___R_s_["), { CSD_UNSTRETCH | CSD_ALIGN_SELF_H } },
	{ HashStr("PR0302RINGANM.XNCP/Scene_0000/bomb_ligid"), { CSD_UNSTRETCH | CSD_ALIGN_SELF_H } },
	{ HashStr("PR0302RINGANM.XNCP/Scene_0000/bomb_parp"), { CSD_UNSTRETCH | CSD_ALIGN_SELF_H } },
	// End Circus Park
};

static std::optional<XXH64_hash_t> FindCsdPath(uint32_t data)
{
	std::lock_guard lock(g_pathMutex);

	auto findResult = g_paths.find(g_memory.Translate(data));

	if (findResult == g_paths.end())
		return {};

	return findResult->second;
}

std::optional<CsdModifier> FindCsdModifier(uint32_t data)
{
	const auto path = FindCsdPath(data);

	if (!path.has_value())
		return {};

	auto findResult = g_csdModifiers.find(path.value());

	if (findResult != g_csdModifiers.end())
		return findResult->second;

	return {};
}


/*** START Render at any Aspect Ratio ***/
// Based on the Sonic Heroes PC widescreen patch (thanks Sewer56 for Heroes.Graphics.Essentials)
// and thanks DonutStopGaming for mapping the Rw functions in Xbox Shadow!
// RwCameraSetViewWindow @ 0x2DD1C0
// CameraBuildPerspClipPlanes @ 0x2DB9C0
// RwCamera: +0x68 viewWindow (clip planes/culling)
//           +0x70 recipViewWindow (projection matrix)

static constexpr uint32_t rwCameraViewWindow = 0x68;
static constexpr uint32_t rwCameraRecipViewWindow = 0x70;
static constexpr float nativeAspectRatio = NARROW_ASPECT_RATIO;

static float GetRelativeAspectRatio(float aspectRatio)
{
	return aspectRatio / nativeAspectRatio;
}

// Necessary because this runs before g_aspectRatio is init
static bool TryGetAspectRatio(float& aspectRatio)
{
	aspectRatio = g_aspectRatio;
	return aspectRatio > 0.0f;
}

static void StretchViewWindow(uint32_t camera, float aspectRatio, float relativeAspectRatio)
{
	const auto base = g_memory.base;
	if (aspectRatio > nativeAspectRatio)
	{
		// Hor+
		X86_STORE_F32(camera + rwCameraViewWindow, X86_LOAD_F32(camera + rwCameraViewWindow) * relativeAspectRatio);
	}
	else {
		// Vert+
		X86_STORE_F32(camera + rwCameraViewWindow + 4, X86_LOAD_F32(camera + rwCameraViewWindow + 4) / relativeAspectRatio);
	}
}

static void UnStretchViewWindow(uint32_t camera, float aspectRatio, float relativeAspectRatio)
{
	StretchViewWindow(camera, aspectRatio, 1.0f / relativeAspectRatio);
}

static void StretchRecipViewWindow(uint32_t camera, float aspectRatio, float relativeAspectRatio)
{
	const auto base = g_memory.base;
	if (aspectRatio > nativeAspectRatio)
	{
		// Hor+
		X86_STORE_F32(camera + rwCameraRecipViewWindow, X86_LOAD_F32(camera + rwCameraRecipViewWindow) * relativeAspectRatio);
	}
	else
	{
		// Vert+
		X86_STORE_F32(camera + rwCameraRecipViewWindow + 4, X86_LOAD_F32(camera + rwCameraRecipViewWindow + 4) / relativeAspectRatio);
	}
}

static void UnStretchRecipViewWindow(uint32_t camera, float aspectRatio, float relativeAspectRatio)
{
	StretchRecipViewWindow(camera, aspectRatio, 1.0f / relativeAspectRatio);
}

// RwCameraSetViewWindow is only called when fov changes
// Force a reaction
static void RecalculateRecipViewWindow(uint32_t camera)
{
	const auto base = g_memory.base;
	float viewWindowX = X86_LOAD_F32(camera + rwCameraViewWindow);
	float viewWindowY = X86_LOAD_F32(camera + rwCameraViewWindow + 4);
	if (viewWindowX != 0.0f)
	{
		X86_STORE_F32(camera + rwCameraRecipViewWindow, 1.0f / viewWindowX);
	}
	if (viewWindowY != 0.0f)
	{
		X86_STORE_F32(camera + rwCameraRecipViewWindow + 4, 1.0f / viewWindowY);
	}
}


// RwCameraSetViewWindow
X86_FUNC_IMPL(__imp__sub_2DD1C0);
X86_OVERRIDE_FUNC(sub_2DD1C0)
{
	uint32_t camera = X86_LOAD_U32(ctx.esp.u32 + 4);

	__imp__sub_2DD1C0(ctx, base);

	float aspectRatio;
	if (camera != 0 && TryGetAspectRatio(aspectRatio))
	{
		UnStretchRecipViewWindow(camera, aspectRatio, GetRelativeAspectRatio(aspectRatio));
	}
}

// CameraBuildPerspClipPlanes
X86_FUNC_IMPL(__imp__sub_2DB9C0);
X86_OVERRIDE_FUNC(sub_2DB9C0)
{
	uint32_t camera = ctx.eax.u32;
	if (camera == 0)
	{
		__imp__sub_2DB9C0(ctx, base);
		return;
	}

	float aspectRatio;
	if (TryGetAspectRatio(aspectRatio))
	{
		float relativeAspectRatio = GetRelativeAspectRatio(aspectRatio);

		StretchViewWindow(camera, aspectRatio, relativeAspectRatio);
		RecalculateRecipViewWindow(camera);

		__imp__sub_2DB9C0(ctx, base);

		UnStretchViewWindow(camera, aspectRatio, relativeAspectRatio);
	}
	else
	{
		__imp__sub_2DB9C0(ctx, base);
	}
}

// The game draws a quad in front of the game camera and scale it, but this only work for 4:3, 
// this is used for different effects, such as the camera in Gun Fortress or the rain in Sky Troops. 
// We can edit the math and apply the same stretch the projection got with the new aspect ratio (right before the matrix scale call).

static void StretchFullScreenQuadScale(uint32_t scaleArgs)
{

	float aspectRatio = g_aspectRatio;
	auto base = g_memory.base;

	const float relativeAspectRatio = GetRelativeAspectRatio(aspectRatio);

	// esp+0 = X scale, esp+4 = Y scale, esp+8 = Z scale (1.0f).

	if (aspectRatio > nativeAspectRatio) // horizontal
	{
		X86_STORE_F32(scaleArgs, X86_LOAD_F32(scaleArgs) * relativeAspectRatio);
	}
	else // graw the quad vertically
	{
		X86_STORE_F32(scaleArgs + 4, X86_LOAD_F32(scaleArgs + 4) / relativeAspectRatio);
	}
}

// The camera that shows a red filter that covers the screen in Gun Fortress.
void WatchCameraFilterScaleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	StretchFullScreenQuadScale(ctx.esp.u32);
}

// RainEffect (ctor sub_1C7C70) does the same thing for the rain overlay than the camera red filter in Gun Fortress
// So we can just re use the same code
void RainEffectScaleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	StretchFullScreenQuadScale(ctx.esp.u32);
}

X86_FUNC_IMPL(__imp__sub_29A850);
X86_OVERRIDE_FUNC(sub_29A850)
{
	g_isInStageMode = true;
	__imp__sub_29A850(ctx, base);
	g_isInStageMode = false;
}

// Synopsis draws the story lines with the DrawMessage func over the background.
// The text gets stretched at higher resolution, so skip the Rt2d unstretch code to avoid that.
X86_FUNC_IMPL(__imp__sub_29C830);
X86_OVERRIDE_FUNC(sub_29C830)
{
	g_isDrawingSynopsisText = true;
	__imp__sub_29C830(ctx, base);
	g_isDrawingSynopsisText = false;
}

// same as above but for library sequence list
X86_FUNC_IMPL(__imp__sub_2924B0);
X86_OVERRIDE_FUNC(sub_2924B0)
{
	g_isDrawingLibraryText = true;
	__imp__sub_2924B0(ctx, base);
	g_isDrawingLibraryText = false;
}

// StaffRollDrawTask draw
X86_FUNC_IMPL(__imp__sub_29CB50);
X86_OVERRIDE_FUNC(sub_29CB50)
{
	g_isDrawingStaffRoll = true;
	__imp__sub_29CB50(ctx, base);
	g_isDrawingStaffRoll = false;
}

// Note: IMPL already declared in CSD stuff for CPlatformRWDrawPrimitive2DMidAsmHook
X86_OVERRIDE_FUNC(sub_2ECC80)
{
	if (g_isDrawingStaffRoll)
	{
		// rebuilt every draw, no restore
		CsdSavedVertices saved{};
		UnstretchPrimitive(g_memory.Translate(X86_LOAD_U32(ctx.esp.u32 + 0x8)), CSDVertexCount, CSD_CENTER_UNSTRETCH, saved);
	}
	__imp__sub_2ECC80(ctx, base);
}

// Patch the text draw in the main menu (score in stage select, level name etc.)
// The font system (bitmap) draws every string as Rt2d geometry which is pushed through 3D Space.
// Each glyph quad is set up by the world matrix then puts the geometry in a 2D space. 
// However the widescreen projection stretch the scene which doesn't work at high res
// we can patch the world matrix's active axis with the relative aspect ratio to unstretch the text.

static constexpr uint32_t rt2dDeviceMatrix = 0x582510; // RwMatrix
static constexpr uint32_t rwMatrixRightX = 0x00;
static constexpr uint32_t rwMatrixUpY = 0x14;
static constexpr uint32_t rwMatrixPosX = 0x30;
static constexpr uint32_t rwMatrixPosY = 0x34;

static constexpr uint32_t rwIm3DVertexStride = 36;

struct Rt2dFontUnstretch
{
	bool active; // is building glyph geometry
	bool horizontal; // which axis the projection stretched
	float scale; // what to multiply that axis by to get the vanilla 4:3 proportions back
};

static Rt2dFontUnstretch g_rt2dFontUnstretch{};

static bool ShouldUnstretchRt2dFont()
{
	if (g_isInStageMode || g_isDrawingSynopsisText || g_isDrawingLibraryText || App::s_gamemode != -1 || IsEventPlaying())
	{
		return false;
	}

	float aspectRatio = 0.0f;

	if (!TryGetAspectRatio(aspectRatio))
		return false;

	const float relativeAspectRatio = GetRelativeAspectRatio(aspectRatio);

	g_rt2dFontUnstretch.horizontal = aspectRatio > nativeAspectRatio;
	g_rt2dFontUnstretch.scale = g_rt2dFontUnstretch.horizontal ? relativeAspectRatio : 1.0f / relativeAspectRatio;

	return true;
}

ImVec2 GuestTextToViewport(float x, float y)
{
	ImVec2 pos = { x / nativeAspectRatio, 1.0f - y };
	if (!ShouldUnstretchRt2dFont())
	{
		pos.x = 0.5f + (pos.x - 0.5f) * g_hudUnstretchX;
		pos.y = 0.5f + (pos.y - 0.5f) * g_hudUnstretchY;
	}
	return pos;
}

static void UnstretchRt2dVertices(uint32_t verts, uint32_t count)
{
	if (verts == 0 || count == 0 || count > 0x10000)
		return;

	auto base = g_memory.base;

	const bool horizontal = g_rt2dFontUnstretch.horizontal;
	const float axisScale = X86_LOAD_F32(rt2dDeviceMatrix + (horizontal ? rwMatrixRightX : rwMatrixUpY));

	// Rt2d has not mapped anything yet, so there is no screen centre to work with
	if (axisScale == 0.0f)
		return;

	const float scale = g_rt2dFontUnstretch.scale;
	const float pivot = -X86_LOAD_F32(rt2dDeviceMatrix + (horizontal ? rwMatrixPosX : rwMatrixPosY)) / axisScale;
	const uint32_t component = horizontal ? 0 : 4;

	for (uint32_t i = 0; i < count; i++)
	{
		const uint32_t addr = verts + i * rwIm3DVertexStride + component;
		X86_STORE_F32(addr, pivot + (X86_LOAD_F32(addr) - pivot) * scale);
	}
}

// rt2dFontShowBitmap
X86_FUNC_IMPL(__imp__sub_3065C0);
X86_OVERRIDE_FUNC(sub_3065C0)
{
	const Rt2dFontUnstretch previous = g_rt2dFontUnstretch;

	g_rt2dFontUnstretch.active = ShouldUnstretchRt2dFont();
	__imp__sub_3065C0(ctx, base);

	g_rt2dFontUnstretch = previous;
}

// rt2dFontShowOutline (draws its glyphs as Rt2d paths (sub_308230) instead of quads
X86_FUNC_IMPL(__imp__sub_304E70);
X86_OVERRIDE_FUNC(sub_304E70)
{
	const Rt2dFontUnstretch previous = g_rt2dFontUnstretch;

	g_rt2dFontUnstretch.active = ShouldUnstretchRt2dFont();
	__imp__sub_304E70(ctx, base);

	g_rt2dFontUnstretch = previous;
}

// RwIm3DTransform(verts, count, ltm, flags) which is basically SetTransform on the RenderWare Layer.
X86_FUNC_IMPL(__imp__sub_2E1890);
X86_OVERRIDE_FUNC(sub_2E1890)
{
	if (g_rt2dFontUnstretch.active)
		UnstretchRt2dVertices(X86_LOAD_U32(ctx.esp.u32 + 4), X86_LOAD_U32(ctx.esp.u32 + 8));

	__imp__sub_2E1890(ctx, base);
}
/*** END Render at any Aspect Ratio ***/


// remove the infamous half-pixel offset issue 
// see https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#3.3.1%20Pixel%20Coordinate%20System
void D3DDevice_SetScreenSpaceOffset(float a1, float a2)
{
	;
}
GUEST_FUNCTION_HOOK_STDCALL(sub_3D1320, D3DDevice_SetScreenSpaceOffset);

// Lens flare aspect fix (draws with camera BUT the sprites still need adjustment)
static constexpr uint32_t LensFlareVertexStride = 0x1C;
static constexpr uint32_t LensFlareQuadVertices = 4;
static void UnstretchLensFlareQuad(uint32_t quad)
{
	auto base = g_memory.base;
	float centerX = 0.0f;
	float centerY = 0.0f;
	for (uint32_t i = 0; i < LensFlareQuadVertices; i++)
	{
		centerX += X86_LOAD_F32(quad + i * LensFlareVertexStride);
		centerY += X86_LOAD_F32(quad + i * LensFlareVertexStride + 0x4);
	}

	centerX /= LensFlareQuadVertices;
	centerY /= LensFlareQuadVertices;
	for (uint32_t i = 0; i < LensFlareQuadVertices; i++)
	{
		uint32_t vertex = quad + i * LensFlareVertexStride;
		X86_STORE_F32(vertex, centerX + (X86_LOAD_F32(vertex) - centerX) * g_hudUnstretchX);
		X86_STORE_F32(vertex + 0x4, centerY + (X86_LOAD_F32(vertex + 0x4) - centerY) * g_hudUnstretchY);
	}
}

X86_FUNC_IMPL(__imp__sub_323990);
X86_OVERRIDE_FUNC(sub_323990)
{
	const uint32_t lensFlare = ctx.ecx.u32;
	__imp__sub_323990(ctx, base);

	// 4:3 or other aspect with no squash
	if (g_hudUnstretchX >= 1.0f && g_hudUnstretchY >= 1.0f)
	{
		return;
	}

	const uint32_t quads = X86_LOAD_U32(lensFlare + 0x28);
	const int32_t count = int32_t(X86_LOAD_U32(lensFlare + 0x24));

	// skipped some frames
	if (quads == 0)
	{
		return;
	}

	// base
	if (X86_LOAD_U8(lensFlare + 0x9C) == 0 && count > 0)
	{
		UnstretchLensFlareQuad(quads);
	}

	// sub sprites
	if (X86_LOAD_U8(lensFlare + 0x9D) == 0)
	{
		for (int32_t i = 1; i < count; i++)
		{
			UnstretchLensFlareQuad(quads + i * LensFlareQuadVertices * LensFlareVertexStride);
		}
	}
}
