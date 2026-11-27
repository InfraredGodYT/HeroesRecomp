#include <kernel/function.h>
#include <kernel/heap.h>
#include <audio/audio.h>
#include <audio/driver/sdl3_driver.h>
#include <user/config.h>
#include <unordered_set>

// It's no secret that this game has a lot of audio issues on the Xbox port
// Sounds not looping properly, Distance for 3D Sound tend to break, volume issue and so on.

// Goal Ring has a 3D Sound issue, the game has no 3D support for stereo sound so it fallbacks to a function that doesn't work at all
// We can patch this on the AudioXBOX class level to make it works regardless of sound type
// This was made with Goal Ring in mind, but shoud in theory work for any similar 3D sound issue in the game.

static constexpr uint32_t g_LanguageText = 0x54DD88;
static constexpr uint32_t SLOT_FLAGS = 0x00; // bit 0x4 = near
static constexpr uint32_t SLOT_MUTED = 0x02; // sub_25FE80
static constexpr uint32_t SLOT_BUFFER = 0x08; // Guest key for the IDirectSoundBuffer
static constexpr uint32_t SLOT_VOLUME = 0x10; // centibels volume
static constexpr uint32_t SLOT_POSITION = 0x14; // 25EF70
static constexpr uint32_t SLOT_SOFTWARE_3D = 0x21; // flag when sample is stereo

static constexpr uint32_t g_SLOT_TABLE = 0x558590; // driver channels/slots NOT to be confused with SE SLOTS
static constexpr uint32_t SLOT_STRIDE = 0x3C;
static constexpr int32_t SLOT_COUNT = 0x40;

static constexpr uint32_t CALLSE_VOLUME = 0x06;
static constexpr uint32_t CALLSE_FLAGS = 0x14;
static constexpr uint32_t CALLSE_3D = 0x10;

static constexpr float FULL_VOLUME_DISTANCE = 60.0f;
static constexpr float SILENT_DISTANCE = 400.0f;
static constexpr float NEAR_LATCH_DISTANCE = 300.0f;

// recreation of sub_25FD60's volume curve, it's identical ot the original (for volume >=99).
// we clamp the volume to fix the distance sound issue where it originally dissapears too early
static int32_t IsndVolumeToCentibels(float volume)
{
	if (volume > 100.0f)
		volume = 100.0f;

	constexpr float centibelsAt99 = -2301.03f; // 1000 * log10((-99 + 100) * 0.005)
	constexpr float centibelsFloor = -6400.0f;

	float centibels;
	if (volume >= -99.0f)
	{
		centibels = log10f((volume + 100.0f) * 0.005f) * 1000.0f;
	}
	else
	{
		float t = (volume + 99.0f) / (-127.0f + 99.0f);
		if (t > 1.0f)
			t = 1.0f;

		centibels = centibelsAt99 + t * (centibelsFloor - centibelsAt99);
	}

	if (centibels > 0.0f)
		centibels = 0.0f;

	if (centibels < centibelsFloor)
		centibels = centibelsFloor;

	return int32_t(centibels);
}

// For 2P & Extended MP, ripped straight from GC 0x8056D098
static int32_t IsndVolumeToCentibelsGC(int32_t volume)
{
	constexpr float centibelsAt0 = -301.03f;
	constexpr float centibelsFloor = -6400.0f;
	if (volume <= -127)
	{
		return int32_t(centibelsFloor);
	}
	const float centibels = centibelsAt0 + log10f(1.0f + float(volume) / 127.0f) * 6000.0f;
	if (centibels < centibelsFloor)
	{
		return int32_t(centibelsFloor);
	}
	return int32_t(centibels);
}

// thiscall so ecx = slot, stack = listener
X86_OVERRIDE_FUNC(sub_25FEC0)
{
	const uint32_t slot = ctx.ecx.u32;
	const uint32_t listener = X86_LOAD_U32(ctx.esp.u32 + 0x4);

	ctx.esp.u32 += 0x8; // handle esp return early so we don't have to worry about it later.

	if (slot == 0 || listener == 0)
		return;

	const uint32_t flags = X86_LOAD_U32(slot + SLOT_FLAGS);
	const uint32_t buffer = X86_LOAD_U32(slot + SLOT_BUFFER);

	if (X86_LOAD_U8(slot + SLOT_SOFTWARE_3D) == 0 || (flags & 2) != 0 || buffer == 0)
		return;

	const float dx = X86_LOAD_F32(listener + 0x0) - X86_LOAD_F32(slot + SLOT_POSITION + 0x0);
	const float dy = X86_LOAD_F32(listener + 0x4) - X86_LOAD_F32(slot + SLOT_POSITION + 0x4);
	const float dz = X86_LOAD_F32(listener + 0x8) - X86_LOAD_F32(slot + SLOT_POSITION + 0x8);
	const float distance = sqrtf(dx * dx + dy * dy + dz * dz);

	// clamp volume like the game originally do with IsndCallSE::GetOffsetVolume
	float volume;
	if (distance <= FULL_VOLUME_DISTANCE)
		volume = 0.0f;
	else if (distance >= SILENT_DISTANCE)
		volume = -127.0f;
	else
		volume = -((distance - FULL_VOLUME_DISTANCE) / (SILENT_DISTANCE - FULL_VOLUME_DISTANCE)) * 127.0f;


	if (distance < NEAR_LATCH_DISTANCE)
		X86_STORE_U32(slot + SLOT_FLAGS, flags | 4);
	else if (distance > SILENT_DISTANCE)
		X86_STORE_U32(slot + SLOT_FLAGS, flags & ~4u);

	const int32_t centibels = IsndVolumeToCentibels(volume);

	if (int32_t(X86_LOAD_U32(slot + SLOT_VOLUME)) == centibels)
		return;

	X86_STORE_U32(slot + SLOT_VOLUME, uint32_t(centibels));
	IDirectSoundBuffer_SetVolume(int32_t(buffer), centibels);
}

// Slot volume
X86_OVERRIDE_FUNC(sub_25FD60)
{
	const uint32_t slot = ctx.ecx.u32;
	const int32_t volume = int32_t(X86_LOAD_U32(ctx.esp.u32 + 0x4));
	ctx.esp.u32 += 0x8;

	// 0x54DDE2 is MP mode (used directly in the sound distance check)
	const bool multiplayer = X86_LOAD_U8(0x54DDE2) != 0 || Config::ExtendedMultiplayer;
	const int32_t centibels = (multiplayer && volume < 0)
		? IsndVolumeToCentibelsGC(volume)
		: IsndVolumeToCentibels(float(volume));
	X86_STORE_U32(slot + SLOT_VOLUME, uint32_t(centibels));

	const uint32_t buffer = X86_LOAD_U32(slot + SLOT_BUFFER);
	const bool muted = (X86_LOAD_U32(slot + SLOT_FLAGS) & SLOT_MUTED) != 0;
	const bool stereo = X86_LOAD_U8(slot + SLOT_SOFTWARE_3D) != 0;

	// muted and stereo slots are handled in their own funcs
	if (buffer != 0 && !muted && !stereo)
	{
		IDirectSoundBuffer_SetVolume(int32_t(buffer), centibels);
	}
}

// Original just plays at max volume instead of getting the distance volume
void SE3DStartVolumeMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t callSE = ctx.esi.u32;
	if ((X86_LOAD_U32(callSE + CALLSE_FLAGS) & CALLSE_3D) != 0)
	{
		// IsndCallSE::GetOffsetVolume
		const uint32_t volume = GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_25DC30, callSE);
		X86_STORE_U8(callSE + CALLSE_VOLUME, uint8_t(volume));
	}
}

static int32_t GetSlotHandle(uint32_t high)
{
	auto base = g_memory.base;
	const int32_t handle = (int32_t(int8_t(X86_LOAD_U8(high))) << 4) | (X86_LOAD_U8(high + 0x4) & 0xF);
	return handle < SLOT_COUNT ? handle : -1;
}

static void SetSlotBufferPan(int32_t handle, int8_t pan)
{
	auto base = g_memory.base;
	const uint32_t key = X86_LOAD_U32(g_SLOT_TABLE + handle * SLOT_STRIDE + SLOT_BUFFER);
	if (key == 0)
	{
		return;
	}

	if (auto* buffer = GetCurrentSoundBuffer(int32_t(key)))
	{
		// game pan is -127 to 127
		buffer->SetPan(pan / 127.0f);
	}
}

// Pan angle (pi/2 radians; rather than GC BAMS)
void SEPanAngleMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t yaw = ctx.esp.u32 + 0x4;
	X86_STORE_F32(yaw, X86_LOAD_F32(yaw) + float(M_PI) / 2.0f);
}

void SEPanSkipGCBAMSMidAsmHook(X86Context& __restrict ctx, uint8_t* base) {}

// Slot pan
// OG function is a nullsub since DirectSound + HRTF handles panning normally
static int8_t g_slotPan[SLOT_COUNT];
X86_OVERRIDE_FUNC(sub_25B280)
{
	const int8_t pan = int8_t(X86_LOAD_U8(ctx.esp.u32 + 0x4));
	const int32_t handle = GetSlotHandle(ctx.esp.u32 + 0x8);
	ctx.esp.u32 += 0x10;

	if (handle < 0)
	{
		return;
	}

	g_slotPan[handle] = pan;
	SetSlotBufferPan(handle, pan);
}

// Slot play
X86_FUNC_IMPL(__imp__sub_25F120);
X86_OVERRIDE_FUNC(sub_25F120)
{
	const int32_t handle = GetSlotHandle(ctx.esp.u32 + 0xC);
	const uint32_t position = X86_LOAD_U32(ctx.esp.u32 + 0x14);
	__imp__sub_25F120(ctx, base);
	if (handle < 0)
	{
		return;
	}
	SetSlotBufferPan(handle, position != 0 ? g_slotPan[handle] : 0);
	g_slotPan[handle] = 0; // reset pan since channels get reused
}

/* --MUSIC RACE ISSUE PATCH-- */

static constexpr uint32_t PORT_IN_USE = 0x04;
static constexpr uint32_t MAX_PLAY_RETRIES = 16; // about 0.27s at 60Hz

static std::unordered_map<uint32_t, uint32_t> g_rnaPlayRetries;
static std::mutex g_rnaPlayRetriesMutex;

static bool RnaPortIsPlaying(uint32_t rna)
{
	auto base = g_memory.base;

	const uint32_t port = X86_LOAD_U32(rna + RNA_PORT);
	if (!port)
		return false;

	// both are still empty while mwSndOpenPort runs, and the play gives up on them
	if (!X86_LOAD_U32(port + PORT_IN_USE))
		return false;

	const uint32_t key = X86_LOAD_U32(port + PORT_BUFFER);
	if (!key)
		return false;

	// also catches a dead key, or a play that never ran because SetFrequency failed first
	auto* buffer = GetCurrentSoundBuffer(int32_t(key));

	if (!buffer)
		return false;

	uint32_t status = 0;
	return (buffer->GetStatus(&status) & X_DSBSTATUS_PLAYING) != 0;
}

// true means we skip it, so the game asks for the play again next vblank call
bool RnaPlayStateLatchMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const uint32_t rna = ctx.esi.u32;

	std::lock_guard<std::mutex> lock(g_rnaPlayRetriesMutex);

	if (RnaPortIsPlaying(rna))
	{
		g_rnaPlayRetries.erase(rna);
		return false;
	}

	uint32_t& retries = g_rnaPlayRetries[rna];
	if (retries >= MAX_PLAY_RETRIES)
	{
		// give up and mark it like the game would, instead of asking all stage long
		g_rnaPlayRetries.erase(rna);
		return false;
	}

	++retries;
	return true;
}

// Extra attempt: This patch is to fix audio Event not starting (which use ADX files too).
// ADXT_Pause reads the stream state before it takes the CRI lock, then it tells the RNA to play
// (or stop) when that state is 3 or 4. Meanwhile, TObjBGM::Play pauses a new track and unpauses it 2 frames later.
// If the ADX vsync thread sets the stream to state 3 while the unpause waits on the lock, it will see the pause
// flag set and skips the play. We can patch this by reading the state again once the lock is held.
static constexpr uint32_t ADXT_STAT = 0x01;
void AdxtPauseReloadStatMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const int32_t stat = int8_t(X86_LOAD_U8(ctx.esi.u32 + ADXT_STAT));

	ctx.edi.u32 = uint32_t(stat);
}

/* --END MUSIC RACE ISSUE PATCH-- */

/* --CHAOS POWER MUSIC OPTION-- */
static int32_t g_awakeJingle = -1;
static bool g_awakeThemePlaying = false;
bool IdleChaosPowersMusicStartMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	g_awakeJingle = int32_t(ctx.ecx.u32);
	g_awakeThemePlaying = Config::IdleChaosPowersMusic;
	return !g_awakeThemePlaying;
}

bool IdleChaosPowersMusicStopMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	const bool wasPlaying = g_awakeThemePlaying;
	g_awakeJingle = -1;
	g_awakeThemePlaying = false;
	return !wasPlaying;
}

// ActStageController::Exec
X86_FUNC_IMPL(__imp__sub_25A960);
X86_OVERRIDE_FUNC(sub_25A960)
{
	const bool chaosPowerMusicEnabled = Config::IdleChaosPowersMusic;
	if (g_awakeJingle >= 0 && chaosPowerMusicEnabled != g_awakeThemePlaying)
	{
		if (!chaosPowerMusicEnabled)
		{
			GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_25A790, uint32_t(g_awakeJingle));
			g_awakeThemePlaying = false;
		}
		// 0x54DDF0 = music playing flag
		// 0x54DEC8 = music state (& 0x2 = stopped, invoke restart)
		else if (X86_LOAD_U8(0x54DDF0) == 0 && (X86_LOAD_U8(0x54DEC8) & 0x2) == 0)
		{
			GuestToHostFunction<void, CallConvention::Cdecl>(ctx, sub_25A720, uint32_t(g_awakeJingle));
			g_awakeThemePlaying = true;
		}
	}
	__imp__sub_25A960(ctx, base);
}
/* --END CHAOS POWER MUSIC OPTION-- */

/* --GRIND RAIL SAMPLE PATCH-- */

// The black comet stages all load bank79.pac for sound effect, however on Xbox that bank shipped with 
// the regular grind rail sound in sample slots 0 and 1 instead of the organic grind rail instead.
// Thankfully that sound still exist on a different bank, so we can patch this by loading that bank
// and make the two wrong entries point at the right sound.

// Additionally, the Death Ruins organic rails has missing loop flag and points so the sound never loop.
// Bank71.pac is the only bank that still ships loop points properly for that sound, 
// so we will reuse its values to fix that.

// runtime sample table, filled by sub_25ECF0 and the play sfx functions.
static constexpr uint32_t g_SAMPLE_TABLE = 0x550318;
static constexpr uint32_t SAMPLE_BANK_STRIDE = 4096;
static constexpr uint32_t SAMPLE_ENTRY_STRIDE = 32;
static constexpr uint32_t SAMPLE_HEADER = 0x00; // WAVEFORMATEX, 32 bytes
static constexpr uint32_t SAMPLE_DATA = 0x04;
static constexpr uint32_t SAMPLE_SIZE = 0x08;
static constexpr uint32_t SAMPLE_FLAGS = 0x0C; // bit 0 = loop
static constexpr uint32_t SAMPLE_LOOP_START = 0x10;
static constexpr uint32_t SAMPLE_LOOP_LENGTH = 0x14;
static constexpr uint32_t SAMPLE_VALID = 0x1C;

static constexpr uint32_t STAGE_BANK_SLOT = 7;
static constexpr uint32_t GRIND_LANDING = 0; // SE_Z0000_GRINDLANDING
static constexpr uint32_t GRIND_LOOP = 1; // SE_Z0001_GRINDING
static constexpr uint32_t GRIND2_LOOP = 3; // SE_Z0003_GRINDING2

static constexpr uint32_t ORGANIC_GRIND_SIZE = 82988;
static constexpr uint32_t ORGANIC_GRIND_LOOP_START = 4;
static constexpr uint32_t ORGANIC_GRIND_LOOP_LENGTH = 82972;

static constexpr uint32_t g_FILE_LOADER = 0x537768; // sub_25F2F0 sets it up before any bank load
static constexpr char ORGANIC_GRIND_BANK[] = "sound/bank7b.pac";


static uint32_t LoadOrganicGrindBankSound(const X86Context& ctx)
{
	static uint32_t image = 0;

	if (image)
		return image;

	auto base = g_memory.base;

	const uint32_t loader = X86_LOAD_U32(g_FILE_LOADER);
	if (!loader)
		return 0;

	// build string path dynamically
	auto* path = static_cast<char*>(g_userHeap.Alloc(sizeof(ORGANIC_GRIND_BANK)));
	memcpy(path, ORGANIC_GRIND_BANK, sizeof(ORGANIC_GRIND_BANK));

	image = GuestToHostFunction<uint32_t, CallConvention::Thiscall>(ctx, sub_1A330, loader,
		g_memory.MapVirtual(path), 0);

	return image;
}

// bank layout: +0 count, +4 record table, +12 data start. 
// records are 16 bytes, +1 index, +4 offset
static uint32_t FindPacSample(uint32_t image, uint32_t index)
{
	auto base = g_memory.base;

	const uint32_t count = X86_LOAD_U32(image);
	const uint32_t records = image + X86_LOAD_U32(image + 0x4);
	const uint32_t data = image + X86_LOAD_U32(image + 0xC);

	for (uint32_t i = 0; i < count; ++i)
	{
		const uint32_t record = records + i * 16;

		if (X86_LOAD_U8(record + 1) == index)
			return data + X86_LOAD_U32(record + 4); // the 32 byte header, sample data follows it.
	}

	return 0;
}

static uint32_t StageBankEntry(uint32_t index)
{
	return g_SAMPLE_TABLE + STAGE_BANK_SLOT * SAMPLE_BANK_STRIDE + index * SAMPLE_ENTRY_STRIDE;
}

static void SetGrindLoopRegion(uint32_t entry)
{
	auto base = g_memory.base;

	X86_STORE_U32(entry + SAMPLE_FLAGS, X86_LOAD_U32(entry + SAMPLE_FLAGS) | 1);
	X86_STORE_U32(entry + SAMPLE_LOOP_START, ORGANIC_GRIND_LOOP_START);
	X86_STORE_U32(entry + SAMPLE_LOOP_LENGTH, ORGANIC_GRIND_LOOP_LENGTH);
}

static void PointEntryAtSample(uint32_t entry, uint32_t header)
{
	auto base = g_memory.base;

	X86_STORE_U32(entry + SAMPLE_HEADER, header);
	X86_STORE_U32(entry + SAMPLE_VALID, header);
	X86_STORE_U32(entry + SAMPLE_DATA, header + 32);
	X86_STORE_U32(entry + SAMPLE_SIZE, X86_LOAD_U32(header + 28));
}

static void InstallOrganicGrindSound(const X86Context& ctx)
{
	auto base = g_memory.base;

	// A mod may have already shipped a fixed bank79.
	if (X86_LOAD_U32(StageBankEntry(GRIND_LOOP) + SAMPLE_SIZE) == ORGANIC_GRIND_SIZE)
		return;

	const uint32_t image = LoadOrganicGrindBankSound(ctx);
	if (!image)
		return;

	const uint32_t landing = FindPacSample(image, GRIND_LANDING);
	const uint32_t loop = FindPacSample(image, GRIND_LOOP);

	if (!landing || !loop)
		return;

	PointEntryAtSample(StageBankEntry(GRIND_LANDING), landing);
	PointEntryAtSample(StageBankEntry(GRIND_LOOP), loop);

	// Bank7B has missing loop data for rail grinding
	SetGrindLoopRegion(StageBankEntry(GRIND_LOOP));
}

// wherever the organic grind shows up in a stage bank, check and fix loop points and flag if needed
static void RestoreGrindLoop(uint32_t index)
{
	auto base = g_memory.base;

	const uint32_t entry = StageBankEntry(index);

	if (!X86_LOAD_U32(entry + SAMPLE_VALID))
		return;

	if (X86_LOAD_U32(entry + SAMPLE_SIZE) != ORGANIC_GRIND_SIZE)
		return;

	if (X86_LOAD_U32(entry + SAMPLE_FLAGS) & 1)
		return;

	SetGrindLoopRegion(entry);
}

/* --END GRIND RAIL SAMPLE PATCH-- */


/* --DASH LOOP SEAM PATCH-- */

// Spin dash and light dash make a small "tic" every time their loop fire on Xbox.
// The loop points are correct, but because the Xbox samples were resampled, the seam lands in the wrong
// region. We can move the points to a spot where the wave lines up, then fade the end of the loop properly.

struct DashLoopFix
{
	uint32_t index;
	uint32_t size; // sample bytes, used so if a mod replace the sounds, the loop patch won't run
	uint32_t oldStart;
	uint32_t oldLength;
	uint32_t newStart;
	uint32_t newLength;
};

static constexpr uint32_t PLAYER_BANK_SLOT = 3;
static constexpr uint32_t LOOP_CROSSFADE_SAMPLES = 512; // about 10ms at 48kHz
static constexpr uint16_t SAMPLE_FORMAT_PCM = 1; // WAVEFORMATEX tag thing

// Memo: values are in bytes, the samples are 16 bit mono pcm
static constexpr DashLoopFix g_dashLoopFixes[] =
{
	{ 14, 189160, 124112, 65044, 124406, 64744 }, // SE_SD14_SPINDASH1
	{ 16, 188008, 117798, 70204, 118592, 68640 }, // SE_SD16_LIGHTDASH
};

static void FixDashLoopSeam(const DashLoopFix& fix)
{
	auto base = g_memory.base;

	const uint32_t entry = g_SAMPLE_TABLE + PLAYER_BANK_SLOT * SAMPLE_BANK_STRIDE + fix.index * SAMPLE_ENTRY_STRIDE;
	const uint32_t header = X86_LOAD_U32(entry + SAMPLE_VALID);

	if (!header || X86_LOAD_U16(header) != SAMPLE_FORMAT_PCM)
		return;

	if (X86_LOAD_U32(entry + SAMPLE_SIZE) != fix.size
		|| X86_LOAD_U32(entry + SAMPLE_LOOP_START) != fix.oldStart
		|| X86_LOAD_U32(entry + SAMPLE_LOOP_LENGTH) != fix.oldLength)
	{
		return;
	}

	X86_STORE_U32(entry + SAMPLE_LOOP_START, fix.newStart);
	X86_STORE_U32(entry + SAMPLE_LOOP_LENGTH, fix.newLength);

	auto* samples = reinterpret_cast<int16_t*>(g_memory.Translate(X86_LOAD_U32(entry + SAMPLE_DATA)));
	const uint32_t loopStart = fix.newStart / 2;
	const uint32_t loopEnd = (fix.newStart + fix.newLength) / 2;

	// the fade ends exactly on the sample before loop start
	if (samples[loopEnd - 1] == samples[loopStart - 1] && samples[loopEnd - 2] == samples[loopStart - 2])
		return;

	for (uint32_t i = 0; i < LOOP_CROSSFADE_SAMPLES; ++i)
	{
		const int32_t from = samples[loopEnd - LOOP_CROSSFADE_SAMPLES + i];
		const int32_t to = samples[loopStart - LOOP_CROSSFADE_SAMPLES + i];
		samples[loopEnd - LOOP_CROSSFADE_SAMPLES + i] = int16_t(from + (to - from) * int32_t(i + 1) / int32_t(LOOP_CROSSFADE_SAMPLES));
	}
}

/* --END DASH LOOP SEAM PATCH-- */


/* --EGG DEALER ROULETTE LOOP PATCH-- */

// Egg Dealer bank comes with the roulette sound lacking loop flag for some reason
// The GameCube version has all the same samples with the loop flag and points, so we can copy them safely.

struct SFXLoopFix
{
	uint32_t index;
	uint32_t size; // sample bytes, used so if a mod replace the sounds, the loop patch won't run
	uint32_t loopStart;
	uint32_t loopLength;
};

// Memo: values are in bytes, the samples are 16 bit mono pcm
static constexpr SFXLoopFix g_SFXLoopFixes = { 49, 112974, 98090, 14880 }; // SE_ZF049_SLOTROLL_LP, bank7F


static void FixStageLoop(const SFXLoopFix& sfxData)
{
	auto base = g_memory.base;

	const uint32_t entry = StageBankEntry(sfxData.index);
	const uint32_t header = X86_LOAD_U32(entry + SAMPLE_VALID);

	if (!header || X86_LOAD_U16(header) != SAMPLE_FORMAT_PCM)
		return;

	if (X86_LOAD_U32(entry + SAMPLE_SIZE) != sfxData.size || (X86_LOAD_U32(entry + SAMPLE_FLAGS) & 1))
		return;

	X86_STORE_U32(entry + SAMPLE_FLAGS, X86_LOAD_U32(entry + SAMPLE_FLAGS) | 1);
	X86_STORE_U32(entry + SAMPLE_LOOP_START, sfxData.loopStart);
	X86_STORE_U32(entry + SAMPLE_LOOP_LENGTH, sfxData.loopLength);
}

/* --END EGG DEALER ROULETTE LOOP PATCH-- */


// InitSoundBank(slot, stageID)
X86_FUNC_IMPL(__imp__sub_25F420);
X86_OVERRIDE_FUNC(sub_25F420)
{
	const uint32_t slot = X86_LOAD_U32(ctx.esp.u32 + 0x4);
	const int32_t stage = int32_t(X86_LOAD_U32(ctx.esp.u32 + 0x8));

	// The SoundBanks check what language they should load with the variable text language instead of voices for some reason...
	// so lie to the game about the current text language if needed then restore it after

	uint32_t languageBackup = X86_LOAD_U32(g_LanguageText);

	if (Config::VoiceLanguage.Value == EVoiceLanguage::Japanese)
		X86_STORE_U32(g_LanguageText, 0x0);

	__imp__sub_25F420(ctx, base);

	X86_STORE_U32(g_LanguageText, languageBackup);

	if (slot == PLAYER_BANK_SLOT)
	{
		for (const auto& fix : g_dashLoopFixes)
			FixDashLoopSeam(fix);
	}

	if (slot != STAGE_BANK_SLOT)
		return;

	// black comet levels
	if (stage == 601 || stage == 604 || stage == 700)
		InstallOrganicGrindSound(ctx);

	RestoreGrindLoop(GRIND_LOOP);
	RestoreGrindLoop(GRIND2_LOOP);

	FixStageLoop(g_SFXLoopFixes);
}

/** --SFX limit increase Patch START-- */
// This patches all SFX table references to allow us to use the full channel slots
// We double the SFX table and increase normal SFX limit from 32 to 59
//
// SEManager has a table of 0x2B (43) slots, but only 0x20 (32) are assigned to normal SFX
// Xbox driver has 0x40 (64) channels, slots entries map to the same channel number (slot 0x30 maps to channel 0x30)
// 0x27-0x2A are an exception, mapping to channels outside of 0x40 (ignored by driver)
//
// Before patch:
// 0x2B (43) SFX table size
// 0x0-0x1F normal slots
// 0x20-0x25 never used
// 0x26-0x2A special slots
//
// After patch:
// 0x40 (64) SFX table size
// 0x0-0x25 and 0x2B-0x3F normal slots
// 0x26-0x2A special slots (unchanged)

constexpr uint32_t SE_SLOT_SIZE = 0x34; // size of one slot in bytes
constexpr uint32_t SE_SLOT_COUNT = 0x40; // new max total slot count
constexpr uint32_t SE_NORMAL_SLOT_COUNT = 0x3B;
constexpr uint8_t SE_SPECIAL_SLOT_START = 0x26;
constexpr uint8_t SE_SPECIAL_SLOT_END = 0x2B;

// allocator scan count 0x20 (32) -> 0x3B (59)
void SESlotScanCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U8(ctx.esp.u32 + 0x20, SE_NORMAL_SLOT_COUNT);
}

// allocator index wrap
bool SESlotIndexWrapMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (ctx.ecx.u8_lo == SE_SPECIAL_SLOT_START)
	{
		ctx.ecx.u8_lo = SE_SPECIAL_SLOT_END;
		ctx.eax.u32 += (SE_SPECIAL_SLOT_END - SE_SPECIAL_SLOT_START) * SE_SLOT_SIZE;
		return true;
	}
	return ctx.ecx.u8_lo < SE_SLOT_COUNT;
}

// allocator search start wrap
bool SESlotSearchStartWrapMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	if (ctx.eax.u8_lo == SE_SPECIAL_SLOT_START)
	{
		X86_STORE_U8(ctx.edi.u32 + 0xC, SE_SPECIAL_SLOT_END);
		return true;
	}
	return ctx.eax.u8_lo < SE_SLOT_COUNT;
}

// slot table allocation size
void SESlotTableSizeMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.esp.u32, 4 + SE_SLOT_COUNT * SE_SLOT_SIZE);
}

// slot table count header and vector constructor count
void SESlotTableCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.eax.u32, SE_SLOT_COUNT);
	X86_STORE_U32(ctx.esp.u32 + 0x8, SE_SLOT_COUNT);
}

// slot table bounds (for the funcs byte stepping)
static bool SESlotLoop(const X86Register& offset)
{
	return offset.u32 < SE_SLOT_COUNT * SE_SLOT_SIZE;
}

bool SESlotLoopEsiMidAsmHook(X86Context& __restrict ctx, uint8_t* base) { return SESlotLoop(ctx.esi); }
bool SESlotLoopEdiMidAsmHook(X86Context& __restrict ctx, uint8_t* base) { return SESlotLoop(ctx.edi); }
bool SESlotLoopEbpMidAsmHook(X86Context& __restrict ctx, uint8_t* base) { return SESlotLoop(ctx.ebp); }

// per frame update loop count
void SESlotUpdateLoopCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	X86_STORE_U32(ctx.esp.u32 + 0xC, SE_SLOT_COUNT);
}

// stop all loop count
void SESlotStopAllLoopCountMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	ctx.ebp.u32 = SE_SLOT_COUNT;
}

// find slot loop
bool SESlotFindLoopMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return ctx.ecx.u8_lo < SE_SLOT_COUNT;
}

// normal vs special check
// in the OG code >= 0x26 is considered special
// include the new extra slots in normal slot availability
bool SESlotIsNormalMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return ctx.eax.u8_lo <= SE_SPECIAL_SLOT_START || ctx.eax.u8_lo >= SE_SPECIAL_SLOT_END;
}
/** --SFX limit increase Patch END-- */

void SatelliteLaserWeaponSFXFixMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	// 0xE027 = SE_SY39_HUGELASER1
	// 0xE028 = SE_SY40_HUGELASER2
	const uint32_t sfxId = X86_LOAD_U32(ctx.esp.u32);
	X86_STORE_U32(ctx.esp.u32, sfxId == 0xE027 ? 0xE028 : 0xE027);
}

/** GravityChangeSwitch / GravityChangeZone SFX spam fix Patch START */
// These sounds always sounded awful on Xbox, and it got worse at higher framerates.
// It actually plays 9 times at normal framerates per contact.
// Patch to do one sound per object per contact

static std::unordered_set<uint32_t> g_gravityChangeTouches;
bool GravityChangeTouchSFXMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{
	return !g_gravityChangeTouches.insert(ctx.esi.u32).second;
}

// GravityChangeSwitch state
// Exec at sub_20DA10
// Exec calls while the obj isn't acting on prior collision
X86_FUNC_IMPL(__imp__sub_20D980);
X86_OVERRIDE_FUNC(sub_20D980)
{
	const uint32_t gravitySwitch = ctx.ecx.u32;
	g_gravityChangeTouches.erase(gravitySwitch);
	__imp__sub_20D980(ctx, base);
}

// GravityChangeZone state
// Exec at sub_20E2E0
// Exec calls while the obj isn't acting on prior collision
X86_FUNC_IMPL(__imp__sub_20E260);
X86_OVERRIDE_FUNC(sub_20E260)
{
	const uint32_t zone = ctx.ecx.u32;
	g_gravityChangeTouches.erase(zone);
	__imp__sub_20E260(ctx, base);
}
/** GravityChangeSwitch / GravityChangeZone SFX spam fix Patch END */

