#include <kernel/function.h>

// animated textures (ADB "TextureChange") sometimes don't play, bug in all versions (PS2/Xbox/GC)
// sub_22A440 (async stage geo clump callback) - never sets the current TXD before running (lethal highway bug)
// sub_23240 (objects call sub_17D0B0, which set the TXD)
// sub_21EB0 (setupEntry) wrong TXD can be used depending on async stage geo timing

struct StageDictEntry
{
	uint32_t streamObj;
	uint32_t dict;
};

// one entry per in-flight stage DFF request, evicted on completion
static StageDictEntry g_StageDictMap[128]{};
static uint32_t g_StageDictMapNext = 0;

static uint32_t g_SavedCurDict = 0;
static bool g_DictPinned = false;

static uint32_t GetCurrentTexDictionary()
{
	auto base = g_memory.base;
	return X86_LOAD_U32(X86_LOAD_U32(0x55F634) + 0x57DE10);
}

static void SetCurrentTexDictionary(uint32_t dict)
{
	auto base = g_memory.base;
	X86_STORE_U32(X86_LOAD_U32(0x55F634) + 0x57DE10, dict);
}

// The pinned TXD can still be wrong if it runs while checking stage vs eff vs cmn TXDs
// sub_2D8400 (RwTexDictionaryFindTextureByName) add a fallback to check all TXD dict on a miss
// RwTexture: +0x04 owner dict, +0x08 list node (texture = node-8), +0x10 name, +0x54 refCount
// RwTexDictionary: +0x08 texture list sentinel, +0x10 global list node
// Global list head sentinel: *(0x55F634) + 0x57DE00

// case-insensitive texture compare, match sub_2D8040
static bool TexNameEquals(uint32_t tex, uint32_t name)
{
	auto base = g_memory.base;

	for (uint32_t i = 0; i < 64; i++)
	{
		uint8_t a = X86_LOAD_U8(name + i);
		uint8_t b = X86_LOAD_U8(tex + 0x10 + i);

		if (a >= 'a' && a <= 'z') a -= 0x20;
		if (b >= 'a' && b <= 'z') b -= 0x20;

		if (a != b)
			return false;

		if (a == 0)
			return true;
	}

	return true;
}

static uint32_t FindTextureInDict(uint32_t dict, uint32_t name)
{
	auto base = g_memory.base;
	if (dict == 0)
		return 0;

	uint32_t sentinel = dict + 8;
	uint32_t node = X86_LOAD_U32(sentinel);

	for (uint32_t guard = 0; node != sentinel && guard < 4096; guard++)
	{
		uint32_t tex = node - 8;
		if (TexNameEquals(tex, name))
			return tex;

		node = X86_LOAD_U32(node);
	}

	return 0;
}

static uint32_t FindTextureInAnyDict(uint32_t name)
{
	auto base = g_memory.base;

	uint32_t head = X86_LOAD_U32(0x55F634) + 0x57DE00;
	uint32_t node = X86_LOAD_U32(head);

	for (uint32_t guard = 0; node != head && guard < 256; guard++)
	{
		uint32_t dict = node - 0x10; // TXD global-list node sits at +0x10
		uint32_t tex = FindTextureInDict(dict, name);
		if (tex != 0)
			return tex;

		node = X86_LOAD_U32(node);
	}

	return 0;
}

// sub_22A830 (stage GEO request)
// save which TXD should be used for the async stage geo clump
void StageStreamCaptureDictMidAsmHook(X86Register& esi)
{
	uint32_t streamObj = esi.u32;
	if (streamObj == 0)
		return;

	uint32_t dict = GetCurrentTexDictionary();

	for (auto& e : g_StageDictMap)
	{
		if (e.streamObj == streamObj)
		{
			e.dict = dict; // pin the proper TXD (main bug in the original / they forgot to do this)
			return;
		}
	}

	g_StageDictMap[g_StageDictMapNext] = { streamObj, dict };
	g_StageDictMapNext = (g_StageDictMapNext + 1) % 128;
}

// sub_22A440 (clump completion, before sub_23240): pin the captured dict around createPlayer
// like the object path does. ESI = stream object
void StageClumpPreCreatePlayerMidAsmHook(X86Register& esi)
{
	g_DictPinned = false;

	uint32_t streamObj = esi.u32;
	if (streamObj == 0)
		return;

	uint32_t dict = 0;
	for (auto& e : g_StageDictMap)
	{
		if (e.streamObj == streamObj)
		{
			dict = e.dict;
			e.streamObj = 0; // request completed - evict
			break;
		}
	}

	if (dict == 0)
		return;

	uint32_t cur = GetCurrentTexDictionary();
	if (cur == dict)
		return; // already correct

	g_SavedCurDict = cur;
	SetCurrentTexDictionary(dict);
	g_DictPinned = true;
}

// sub_22A440 (clump completion, after sub_23240) restore the previous dict
void StageClumpPostCreatePlayerMidAsmHook()
{
	if (g_DictPinned)
	{
		SetCurrentTexDictionary(g_SavedCurDict);
		g_DictPinned = false;
	}
}

// sub_21EB0 (setupEntry, after 0x21F3D)
// Patch to prevent count mismatch (space gadget green fluid rooms for example)
void TexChangeAddRefOriginalMidAsmHook(X86Register& eax)
{
	auto base = g_memory.base;
	uint32_t tex = eax.u32;
	if (tex == 0)
		return;

	X86_STORE_U32(tex + 0x54, X86_LOAD_U32(tex + 0x54) + 1);
}

// TXD getter for ANY texture Dict, Name -> RwTexture*
// Confirmed this DOES allow events to use shadow.one TXDs as a side effect (modders will like this, so keeping it)
X86_OVERRIDE_FUNC(sub_2D8400)
{
	uint32_t dict = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	uint32_t name = X86_LOAD_U32(ctx.esp.u32 + 0x8);

	uint32_t tex = FindTextureInDict(dict, name);
	if (tex == 0)
		tex = FindTextureInAnyDict(name);

	ctx.eax.u32 = tex;
	ctx.esp.u32 += 0x4;
}

// sub_22350 (TTextureChange destructor)
// cleanup for TexChangeAddRefOriginalMidAsmHook
void TexChangeReleaseOriginalRefsMidAsmHook(X86Register& esi)
{
	auto base = g_memory.base;

	uint32_t player = esi.u32;
	if (player == 0)
		return;

	uint32_t count = X86_LOAD_U32(player + 0x30);
	if (count > 0x20)
		return;

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t entry = player + 0x54 + i * 0x14;
		uint32_t tex = X86_LOAD_U32(entry + 8);
		if (tex == 0)
			continue;

		uint32_t refs = X86_LOAD_U32(tex + 0x54);

		if (refs <= 1)
		{
			// dict already destroyed (chunk unloaded already)
			X86_STORE_U32(tex + 0x4, 0);
			X86_STORE_U32(tex + 0x54, 0);
		}
		else
		{
			// update count like original
			X86_STORE_U32(tex + 0x54, refs - 1);
		}
	}
}
