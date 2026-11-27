#include <cstring>
#include <kernel/function.h>
#include <patches/global.h>
#include <os/logger.h>


// Originally, all players shares the same motions files because the game re use Shadow's BON and MTP files. 
// This creates issues with mods because you may only want custom animation for a specific Shadow model and not its variations.
// This will also have issues if one or more models has a custom skeleton; animations will break on multiplayer unless 
// all models are replaced to use the same skeleton as well.

// We can patch this by making the game look for a motion with the same name as the one it is about to play, but with the current skeleton's name as prefix. 
// And if such a motion exists, make the game play it instead of Shadow's.

// (This is mostly for memo):
// - Each motion has a number of "tracks" that must match the BON it is bound to.
// - A track contains: length, key count and a flags field. The flags tell us which bone ID it is animating (pos, rot scale etc) and if it has tangents or not. 
// - The BON has a mask for each bone that tells us which numbers are animated for that bone.


constexpr uint32_t g_MotionPackList = 0x57C690;
constexpr uint32_t g_BonNameRegistry = 0x57C440;
constexpr uint32_t g_BonFileList = 0x57C43C;

// Helper function to get a loaded BON from its id (essentially re create what sub_367200 do), returns 0 if not loaded.
// Memo: A BON file has a header size of 0x30 + 0x70 per bone and the id is stored at +4.
// The game works on the BON from + 4.
uint32_t GetLoadedBon(uint8_t* base, uint32_t bonId)
{
	for (uint32_t group = X86_LOAD_U32(g_BonFileList); group != 0; group = X86_LOAD_U32(group + 0x14))
	{
		uint32_t bon = X86_LOAD_U32(group + 0x10);
		uint32_t count = X86_LOAD_U32(group);

		for (uint32_t i = 0; i < count; i++)
		{
			if (X86_LOAD_U32(bon + 4) == bonId)
				return bon + 4;

			bon += 0x30 + 0x70 * X86_LOAD_U16(bon + 0xA);
		}
	}

	return 0;
}

// Get the BON id from its model name (copy paste of sub_379B40: node [0] = id, [1] = name, [4] = next)
uint32_t GetBonIdByName(uint8_t* base, const char* model)
{
	uint32_t registry = X86_LOAD_U32(g_BonNameRegistry);
	if (registry == 0)
		return 0;

	for (uint32_t node = X86_LOAD_U32(registry); node != 0; node = X86_LOAD_U32(node + 0x10))
	{
		if (strncmp((const char*)(base + X86_LOAD_U32(node + 4)), model, 32) == 0)
			return X86_LOAD_U32(node);
	}

	return 0;
}


// Helper function to verify if a motion is valid, originally the game doesn't count the number of tracks that the skeleton's masks need.
// Thus, in theory, a corrupted motion could make the game read garbage since there is no proper end check.
// Memo: BON in memory start at +4, bone count at +6, bone records (0x70) at +0x2C and mask at bone +6.
bool IsMotionValid(uint8_t* base, uint32_t mtn, uint32_t bon)
{
	uint32_t expectedTracks = 0;
	uint16_t boneCount = X86_LOAD_U16(bon + 6);

	for (uint16_t i = 0; i < boneCount; i++)
	{
		uint16_t mask = X86_LOAD_U16(bon + 0x2C + i * 0x70 + 6);

		if (mask & 0x400)
			expectedTracks += 3;

		// use popcount to count the number of bits set in the mask (this tells us how many tracks are present for this bone)
		expectedTracks += std::popcount(uint32_t(mask & 0x387));

		if ((mask & 0x40) == 0)
			expectedTracks += std::popcount(uint32_t(mask & 0x38));
	}


	uint32_t size = X86_LOAD_U32(mtn + 8);
	uint32_t offset = 0x34; // tracks start here
	uint32_t tracksCount = 0;

	while (true)
	{

		if (offset + 8 > size || tracksCount >= UINT16_MAX) // something is wrong we've been in this loop for way too long
			return false;

		uint32_t length = X86_LOAD_U32(mtn + offset);

		if (length < 8 || length > size - offset)
			return false;

		tracksCount++;


		if (X86_LOAD_U8(mtn + offset + 7) == 0)
			break;

		offset += length;
	}

	return tracksCount == expectedTracks;
}


// Memo: bone tag at bone +0
bool SkeletonHasShadowBody(uint8_t* base, uint32_t bonAddr, uint32_t shBonAddr)
{
	uint16_t bonesCount = X86_LOAD_U16(bonAddr + 6);
	uint16_t shBonesCount = X86_LOAD_U16(shBonAddr + 6);
	uint32_t bodyTags = 0;
	uint32_t count = 0;

	for (uint16_t i = 0; i < shBonesCount; i++)
	{
		uint32_t tag = X86_LOAD_U32(shBonAddr + 0x2C + i * 0x70);

		if (tag >= 100)
			continue;

		bodyTags++;

		for (uint16_t j = 0; j < bonesCount; j++)
		{
			if (X86_LOAD_U32(bonAddr + 0x2C + j * 0x70) == tag)
			{
				count++;
				break;
			}
		}
	}

	return count * 4 >= bodyTags * 3;
}

uint32_t FindMtnPackEntry(uint8_t* base, uint32_t pack, const char* mtnName, uint32_t bonId, uint32_t bon)
{
	uint16_t count = X86_LOAD_U16(pack + 2);
	uint32_t entries = X86_LOAD_U32(pack + 0x10);

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t entry = entries + i * 12;
		uint32_t entryName = X86_LOAD_U32(entry);
		uint32_t mtn = X86_LOAD_U32(entry + 4);

		if (entryName == 0 || mtn == 0 || X86_LOAD_U32(mtn + 4) != bonId)
			continue;

		if (strcmp((const char*)(base + entryName), mtnName) != 0)
			continue;

		if (!IsMotionValid(base, mtn, bon))
		{
			LOGFN_WARNING("Motion {} does not fit its skeleton, keeping the original for safety.", mtnName);
			return 0;
		}

		return X86_LOAD_U16(pack) + i;
	}

	return 0;
}


uint32_t FindMtnInPacks(uint8_t* base, uint32_t sourcePack, const char* mtnName, uint32_t bonId, uint32_t bon)
{
	uint32_t mtn = FindMtnPackEntry(base, sourcePack, mtnName, bonId, bon);
	for (uint32_t pack = X86_LOAD_U32(g_MotionPackList); pack != 0 && mtn == 0; pack = X86_LOAD_U32(pack + 0xC))
	{
		if (pack != sourcePack)
			mtn = FindMtnPackEntry(base, pack, mtnName, bonId, bon);
	}

	return mtn;
}

// Note: this runs only when a motion get queued on a slot
static uint16_t FindNewMotionID(uint8_t* base, uint32_t playWK, uint16_t motionId)
{
	uint32_t bon = X86_LOAD_U32(playWK + 8);

	if (bon == 0 || motionId == 0)
		return motionId;

	uint32_t bonId = X86_LOAD_U32(bon);
	const char* bonNameStr = (const char*)(base + bon + 0xC);
	uint32_t sourcePack = 0;
	uint32_t sourceEntry = 0;

	for (uint32_t pack = X86_LOAD_U32(g_MotionPackList); pack != 0; pack = X86_LOAD_U32(pack + 0xC))
	{
		uint16_t first = X86_LOAD_U16(pack);

		if (motionId >= first && motionId < first + X86_LOAD_U16(pack + 2))
		{
			sourcePack = pack;
			sourceEntry = X86_LOAD_U32(pack + 0x10) + (motionId - first) * 12;
			break;
		}
	}

	if (sourcePack == 0)
		return motionId;

	uint32_t mtn = X86_LOAD_U32(sourceEntry + 4);
	uint32_t entryName = X86_LOAD_U32(sourceEntry);

	if (mtn == 0 || entryName == 0 || X86_LOAD_U32(mtn + 4) == bonId)
		return motionId;

	const char* name = (const char*)(base + entryName);
	const char* model = (const char*)(base + mtn + 0x14);
	size_t modelLen = strnlen(model, 32);

	if (strncmp(name, model, modelLen) != 0)
		return motionId;

	char mtnNameBuffer[96];
	if (snprintf(mtnNameBuffer, sizeof(mtnNameBuffer), "%s%s", bonNameStr, name + modelLen) >= (int)sizeof(mtnNameBuffer))
		return motionId;

	uint32_t mtnFromPack = FindMtnInPacks(base, sourcePack, mtnNameBuffer, bonId, bon);

	// The 2P androids have their own skeleton (except shadow2py), but they still use Shadow's motions directly.
	// As explained above, this create issues for mods: the game looks for animation starting with "sh_XXX",
	// so custom skeletons and animations will fail with them.
	// We patch this to make it so if a skeleton is named "amypl" for example and is asked to play
	// "sh_drive_idle", it will look for "amypl_drive_idle" instead.
	if (mtnFromPack == 0 && strcmp(bonNameStr, "sh") != 0 && strcmp(model, "sh") != 0)
	{
		uint32_t shId = GetBonIdByName(base, "sh");
		uint32_t shBon = shId != 0 ? GetLoadedBon(base, shId) : 0;

		// The game also play motions on other skeletons on its own (like for the gun walker) 
		// so ensure we are targeting Shadow body only.
		if (shBon != 0 && SkeletonHasShadowBody(base, bon, shBon) && snprintf(mtnNameBuffer, sizeof(mtnNameBuffer), "sh%s", name + modelLen) < (int)sizeof(mtnNameBuffer))
			mtnFromPack = FindMtnInPacks(base, sourcePack, mtnNameBuffer, shId, shBon);
	}

	return mtnFromPack != 0 ? (uint16_t)mtnFromPack : motionId;
}


// Returns the address of a slot number from a play work.
// Memo: a play work is the animation state of one skeleton. It has slots, one per motion being
// played or blended. The queued motion id is set at +0x110 and the playing one at +0x128.
uint32_t GetPlaySlot(uint8_t* base, uint32_t playWork, uint32_t index)
{
	return X86_LOAD_U32(playWork + 0x10) + index * 0x130;
}

// Write the new motion id in a slot field and returns the original one so it can be put back properly.
uint16_t SwapMotionId(uint8_t* base, uint32_t playWork, uint32_t field)
{
	uint16_t motionId = X86_LOAD_U16(field);

	if (playWork != 0)
		X86_STORE_U16(field, FindNewMotionID(base, playWork, motionId));

	return motionId;
}

// This function is used to start the queued motion of a slot. It copy the id and read the loop flag of a motion.
// We give it the new custom motion values so its behavior stay correct.
X86_FUNC_IMPL(__imp__sub_36FC90);
X86_OVERRIDE_FUNC(sub_36FC90)
{
	uint32_t playWork = X86_LOAD_U32(ctx.esp.u32 + 4);
	uint32_t slot = GetPlaySlot(base, playWork, X86_LOAD_U32(ctx.esp.u32 + 8));
	uint16_t newMotionId = SwapMotionId(base, playWork, slot + 0x110);
	uint16_t mtn = X86_LOAD_U16(slot + 0x110);

	__imp__sub_36FC90(ctx, base);

	X86_STORE_U16(slot + 0x110, newMotionId);

	if (X86_LOAD_U16(slot + 0x128) == mtn)
		X86_STORE_U16(slot + 0x128, newMotionId);
}

// Keep two blended slots in step (walk and run): (playWork, index slot)
// It reads the frame range of both queued motions, and ours can be shorter or longer than the original.
X86_FUNC_IMPL(__imp__sub_36E3B0);
X86_OVERRIDE_FUNC(sub_36E3B0)
{
	uint32_t playWK = X86_LOAD_U32(ctx.esp.u32 + 4);
	uint32_t slotIndex = X86_LOAD_U32(ctx.esp.u32 + 8);
	uint32_t linked = X86_LOAD_U32(X86_LOAD_U32(playWK + 0x2C) + slotIndex * 4);

	uint32_t slot = GetPlaySlot(base, playWK, slotIndex) + 0x110;
	uint32_t linkedSlot = linked != UINT32_MAX ? GetPlaySlot(base, playWK, linked) + 0x110 : 0;

	uint16_t motionId = SwapMotionId(base, playWK, slot);
	uint16_t linkedId = linkedSlot != 0 ? SwapMotionId(base, playWK, linkedSlot) : 0;

	__imp__sub_36E3B0(ctx, base);

	X86_STORE_U16(slot, motionId);

	if (linkedSlot != 0)
		X86_STORE_U16(linkedSlot, linkedId);
}

// Motions have events on some frames (footsteps, effects etc.). Each frame the game asks which events happened.
// It then takes the events from the playing id. However this fails with custom anim if it has a different length; 
// the events will fire at the wrong time, if ever.
// Hacky Fix: we swap the custom id only for the query, then put the original back after.
// Memo: a null play work means "use the last query" so it keeps the motion it started with.
template <void (*OriginalFunc)(X86Context&, uint8_t*)>
void QueryPlayingMotionPatch(X86Context& ctx, uint8_t* base)
{
	uint32_t playWork = X86_LOAD_U32(ctx.esp.u32 + 4);
	uint32_t slotADdr = 0;

	if (playWork != 0)
	{
		uint32_t index = X86_LOAD_U32(X86_LOAD_U32(playWork + 0x28) + X86_LOAD_U32(ctx.esp.u32 + 8) * 4);

		if (index != UINT32_MAX && (X86_LOAD_U32(GetPlaySlot(base, playWork, index) + 0x90) & 4) != 0)
			slotADdr = GetPlaySlot(base, playWork, index) + 0x128;
	}

	uint16_t motionId = slotADdr != 0 ? SwapMotionId(base, playWork, slotADdr) : 0;

	OriginalFunc(ctx, base);

	if (slotADdr != 0)
		X86_STORE_U16(slotADdr, motionId);
}

// motion event queries
// args: playWork, index, type, out, count
X86_FUNC_IMPL(__imp__sub_376DC0);
X86_OVERRIDE_FUNC(sub_376DC0)
{
	QueryPlayingMotionPatch<__imp__sub_376DC0>(ctx, base);
}

// same as above
X86_FUNC_IMPL(__imp__sub_376FE0);
X86_OVERRIDE_FUNC(sub_376FE0)
{
	QueryPlayingMotionPatch<__imp__sub_376FE0>(ctx, base);
}

// The game blends two motions (run and top run) and sets their speed and frame from the length of both.
// It reads them with their original ids, meaning different custom anim per player will break if they use different frames count / speed.
// We can patch this by giving back the IDs the custom anim uses.
// Memo: player motion ids at + 0x440 and +0x444 (-1 when there is no blend), play work at + 0x3E8.
template <void (*OriginalFunc)(X86Context&, uint8_t*)>
void PatchPlayerBlendMotionsID(X86Context& ctx, uint8_t* base)
{
	uint32_t this_ = ctx.ecx.u32;
	uint32_t playWK = X86_LOAD_U32(this_ + 0x3E8);
	bool isBlend = (int32_t)X86_LOAD_U32(this_ + 0x444) >= 0;

	uint16_t motionId = SwapMotionId(base, playWK, this_ + 0x440);
	uint16_t blendId = isBlend ? SwapMotionId(base, playWK, this_ + 0x444) : 0;

	OriginalFunc(ctx, base);

	X86_STORE_U16(this_ + 0x440, motionId);

	if (isBlend)
		X86_STORE_U16(this_ + 0x444, blendId);
}

// Set the frame of both blended motions (param: this, ratio)
X86_FUNC_IMPL(__imp__sub_93190);
X86_OVERRIDE_FUNC(sub_93190)
{
	PatchPlayerBlendMotionsID<__imp__sub_93190>(ctx, base);
}

// Get the play speed of the blended motions
X86_FUNC_IMPL(__imp__sub_934D0);
X86_OVERRIDE_FUNC(sub_934D0)
{
	PatchPlayerBlendMotionsID<__imp__sub_934D0>(ctx, base);
}


// Each motion get its BON id when its MTP loads. If the BON is not loaded yet, the id stays 0 and
// the model t-pose. So after every .BON load, we fix the wrong ids still at 0 by checking up the motion's model name.
// That way mods won't have to care about file order inside .one archives.

// BON load: (data, freeFn): returns the BON id
X86_FUNC_IMPL(__imp__sub_366B60);
X86_OVERRIDE_FUNC(sub_366B60)
{
	__imp__sub_366B60(ctx, base);

	if (ctx.eax.u32 == 0)
		return;

	for (uint32_t pack = X86_LOAD_U32(g_MotionPackList); pack != 0; pack = X86_LOAD_U32(pack + 0xC))
	{
		uint16_t count = X86_LOAD_U16(pack + 2);
		uint32_t entries = X86_LOAD_U32(pack + 0x10);
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t mtn = X86_LOAD_U32(entries + i * 12 + 4);
			if (mtn == 0 || X86_LOAD_U32(mtn + 4) != 0)
				continue;

			uint32_t bonId = GetBonIdByName(base, (const char*)(base + mtn + 0x14));
			if (bonId != 0)
				X86_STORE_U32(mtn + 4, bonId);
		}
	}
}

// Some player models reuse Shadow's original skeleton and don't have their own ones (Super Shadow and Shadow2py)
static thread_local bool s_IsUsingShadowOGSkeleton = false;

// Player body setup: (archive, slot)
// Memo: +503 = body +529 = MTP, +531 = BON
// The yellow android (shadow2py.one) is told to use shadow.one's SH.BON, so a mod can't give it its own skeleton.
// If the body's archive has a BON of its own, we point the player to it instead,
// and the motions for it are then found by name like for any other skeleton.
X86_FUNC_IMPL(__imp__sub_9C420);
X86_OVERRIDE_FUNC(sub_9C420)
{
	uint32_t this_ = ctx.ecx.u32;
	uint32_t bodyArchive = X86_LOAD_U32(this_ + 503 * 4);

	if (bodyArchive != X86_LOAD_U32(this_ + 531 * 4))
	{
		uint32_t bonSlot = FindBonSlot(base, bodyArchive);
		if (bonSlot != 0)
		{
			X86_STORE_U32(this_ + 531 * 4, bodyArchive);
			X86_STORE_U32(this_ + 532 * 4, bonSlot);
		}
	}

	s_IsUsingShadowOGSkeleton = bodyArchive != X86_LOAD_U32(this_ + 531 * 4);
	__imp__sub_9C420(ctx, base);
	s_IsUsingShadowOGSkeleton = false;
}

// Create play work from a BON id: (bonId, flags)
X86_FUNC_IMPL(__imp__sub_369BB0);
X86_OVERRIDE_FUNC(sub_369BB0)
{
	if (s_IsUsingShadowOGSkeleton)
	{
		uint32_t shId = GetBonIdByName(base, "sh");
		uint32_t bonId = X86_LOAD_U32(ctx.esp.u32 + 4);

		if (shId != 0 && bonId != shId && GetLoadedBon(base, shId) != 0)
			X86_STORE_U32(ctx.esp.u32 + 4, shId);
	}

	__imp__sub_369BB0(ctx, base);
}
