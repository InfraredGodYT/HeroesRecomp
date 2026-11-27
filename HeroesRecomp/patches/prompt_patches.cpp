#include <kernel/function.h>
#include <locale/locale.h>
#include <patches/aspect_ratio_patches.h>
#include <ui/message_window.h>
#include <user/config.h>

static constexpr uint32_t GUEST_MESSAGE_DESCRIPTORS = 0x47AB08;
static constexpr uint32_t GUEST_MESSAGE_DESCRIPTOR_SIZE = 0x24;

static constexpr uint32_t MESSAGE_REQUESTED = 0x08;
static constexpr uint32_t MESSAGE_DISPLAYED = 0x0C;
static constexpr uint32_t MESSAGE_SELECTION = 0x10;
static constexpr uint32_t MESSAGE_RESULT = 0x14;
static constexpr uint32_t MESSAGE_OPEN = 0x18;
static constexpr uint32_t MESSAGE_FINISHED = 0x2C;

static constexpr uint32_t DESCRIPTOR_TEXT_ID = 0x00;
static constexpr uint32_t DESCRIPTOR_OPTION_COUNT = 0x04;
static constexpr uint32_t DESCRIPTOR_DEFAULT_OPTION = 0x08;
static constexpr uint32_t DESCRIPTOR_OPTION_TEXT_IDS = 0x10;
static constexpr uint32_t DESCRIPTOR_RESULT_CODES = 0x1C;

static constexpr uint32_t GUEST_MESSAGE_ORIGIN = 0x529BE4;
static constexpr uint32_t GUEST_MESSAGE_TEXT_OFFSET = 0x529BEC;
static constexpr uint32_t GUEST_MESSAGE_TEXT_HEIGHT = 0x47AB04;
static constexpr uint32_t GUEST_OPTION_ROWS = 0x47B608;
static constexpr uint32_t GUEST_OPTION_ROW_DROP = 0x42BD24;
static constexpr uint32_t GUEST_TEXT_SCALE_X = 0x42A9B4;
static constexpr uint32_t TEXT_DRAWER_FONTS = 0x4;
static constexpr uint32_t TEXT_DRAWER_FONT_COUNT = 10;
static constexpr uint32_t FONT_BASELINE = 0x8;

struct CustomMessage
{
	int32_t TextID;
	const char* LocaleKey;
};

static const CustomMessage g_customMessages[] =
{
	{ 6068, "Title_Message_CreateSave" },
	{ 6069, "Title_Message_OverwriteSave" },
	{ 6070, "Title_Message_CreateSaveFailed" },
	{ 6073, "Title_Message_SaveDamaged" },
	{ 6076, "Title_Message_SaveDamaged" },
	{ 6066, "Title_Message_SaveDamagedNoA" },
	{ 6074, "Title_Message_SaveFailed" },
	{ 6081, "Title_Message_CreateSave" },
	{ 6090, "Title_Message_LoadSave"},
	{ 6091, "Legacy_Options_Message_OverwriteSave"},
	{ 5490, "Title_Message_ContinueWithoutSave"},
	{ 5520, "Legacy_Options_Message_LoadSaveOK"},
	{ 5493, "Pause_Message_Restart"},
	{ 5523, "Pause_Message_Quit"},
	{ 5524, "Unlock_Message_LastStory"},
	{ 5526, "Unlock_Message_ExpertMode"},
	{ 5550, "Unlock_Message_SamuraiBladeLv1"},
	{ 5551, "Unlock_Message_SamuraiBladeLv2"},
	{ 5552, "Unlock_Message_SatelliteLaserLv1"},
	{ 5553, "Unlock_Message_SatelliteLaserLv2"},
	{ 5554, "Unlock_Message_VacuumEggLv1"},
	{ 5555, "Unlock_Message_VacuumEggLv2"},
	{ 5556, "Unlock_Message_OmochaoGunLv1"},
	{ 5557, "Unlock_Message_OmochaoGunLv2"},
	{ 5558, "Unlock_Message_HealCannonLv1"},
	{ 5559, "Unlock_Message_HealCannonLv2"},
	{ 5560, "Unlock_Message_ShadowRifle"},
};

static const CustomMessage* FindCustomMessage(int32_t textID)
{
	for (auto& message : g_customMessages)
	{
		if (message.TextID == textID)
			return &message;
	}

	return nullptr;
}

// Get common buttons used for message / prompt
static const char* GetOptionLocaleKey(int32_t textID)
{
	switch (textID)
	{
	case 6047:
		return "Common_Yes";
	case 6048:
		return "Common_No";
	case 6049:
		return "Common_Ok";
	default:
		return nullptr;
	}
}



bool IsMessageBoxActive()
{
	auto base = g_memory.base;

	auto managerPtr = X86_LOAD_U32(0x541F1C); // guest message manager ptr

	if (managerPtr == 0)
		return false;

	auto message = X86_LOAD_U32(managerPtr);

	if (message == 0)
		return false;


	// Ensure prompt animation is completely over, otherwise assume prompt is still running. 
	return int32_t(X86_LOAD_U32(message + MESSAGE_DISPLAYED)) >= 0 || X86_LOAD_F32(message + MESSAGE_OPEN) > 0.0f;
}

static int g_messageResult = -1;
static int32_t g_messageIndex = -1;

static int32_t GetMessageTextID(uint8_t* base, int32_t index)
{
	return int32_t(X86_LOAD_U32(GUEST_MESSAGE_DESCRIPTORS + index * GUEST_MESSAGE_DESCRIPTOR_SIZE + DESCRIPTOR_TEXT_ID));
}

// skip error message save
X86_FUNC_IMPL(__imp__sub_2A6530);
X86_OVERRIDE_FUNC(sub_2A6530)
{
	auto mode = X86_LOAD_U32(ctx.ecx.u32);

	if (mode == 6) //this technically should no longer trigger
	{
		X86_STORE_U32(ctx.ecx.u32, 14);
	}

	__imp__sub_2A6530(ctx, base);
}

static bool g_shouldPreventPrompt = false;

// player quit prompt
X86_FUNC_IMPL(__imp__sub_5A7B0);
X86_OVERRIDE_FUNC(sub_5A7B0)
{
	g_shouldPreventPrompt = Config::ConfirmationPrompt.Value == false;
	__imp__sub_5A7B0(ctx, base);
	g_shouldPreventPrompt = false;
}

// player restart prompt
X86_FUNC_IMPL(__imp__sub_5A750);
X86_OVERRIDE_FUNC(sub_5A750)
{
	g_shouldPreventPrompt = Config::ConfirmationPrompt.Value == false;
	__imp__sub_5A750(ctx, base);
	g_shouldPreventPrompt = false;
}

// load or create save file confirmation prompt
X86_FUNC_IMPL(__imp__sub_2A6D00);
X86_OVERRIDE_FUNC(sub_2A6D00)
{
	g_shouldPreventPrompt = Config::ConfirmationPrompt.Value == false;
	int mode = GetHostParameter(ctx, 2, CallConvention::Thiscall);
	if (mode == 3 || mode == 18)
		g_shouldPreventPrompt = false; // if user's save data is damaged, we should let the prompt draw to inform user.
	__imp__sub_2A6D00(ctx, base);
	g_shouldPreventPrompt = false;
}

// confirmation prompts logic
X86_FUNC_IMPL(__imp__sub_2A5B60);
X86_OVERRIDE_FUNC(sub_2A5B60)
{
	if (g_shouldPreventPrompt)
	{
		ctx.eax.u32 = 2;
		ctx.AddEsp(4); // thiscall with 3 param + ret value = (4*4)

		return;
	}

	__imp__sub_2A5B60(ctx, base);
}

// Builds the buttons the message originally use
// This is used for text replacement that require an input from user (e.g: ok, yes / no etc.)

static std::vector<std::string> GetMessageOptions(uint8_t* base, uint32_t descriptor)
{
	std::vector<std::string> options;

	auto optionCount = int32_t(X86_LOAD_U32(descriptor + DESCRIPTOR_OPTION_COUNT));

	for (int32_t i = 0; i < optionCount; i++)
	{
		auto textID = int32_t(X86_LOAD_U32(descriptor + DESCRIPTOR_OPTION_TEXT_IDS + i * 4));

		if (auto localeKey = GetOptionLocaleKey(textID))
			options.push_back(Localise(localeKey));
		else
			options.push_back(std::to_string(textID));
	}

	return options;
}

static void CloseCustomMessage()
{
	if (MessageWindow::s_isVisible)
		MessageWindow::Close(true);

	g_messageResult = -1;
	g_messageIndex = -1;
}


// Text and buttons

static bool g_suppressedGuestText = false;

bool DrawAdvertiseMessageMidAsmHook(X86Context& __restrict ctx, uint8_t* base)
{

	g_suppressedGuestText = FindCustomMessage(int32_t(X86_LOAD_U32(ctx.esi.u32))) != nullptr;

	return g_suppressedGuestText;
}

// draw message
X86_FUNC_IMPL(__imp__sub_2A6020);
X86_OVERRIDE_FUNC(sub_2A6020)
{
	X86Context ctxBackup = ctx;

	// The function adds an SEH frame and only unlinks it on the paths that reach its
	// end, so we have to backup and restore it in case of early return.
	auto sehBackup = X86_LOAD_U32(ctx.kpcr_ptr);

	__imp__sub_2A6020(ctx, base);

	if (g_suppressedGuestText) // custom message was used, meaning we returned early, fix esp and reg
	{
		ctx = ctxBackup;
		ctx.AddEsp(1);
		X86_STORE_U32(ctx.kpcr_ptr, sehBackup);
		g_suppressedGuestText = false;
	}
}


static uint8_t g_pauseDimAlpha = 0;

void SetPauseDimAlpha(uint8_t alpha)
{
	g_pauseDimAlpha = alpha;
}

// Patch the black_cover pause NCP bug
X86_FUNC_IMPL(__imp__sub_2A57E0);
X86_OVERRIDE_FUNC(sub_2A57E0)
{
	static constexpr uint32_t dimAlphaAddr = 0x529C37; // 0xA0 (color of the dim)
	auto dimAlpha = X86_LOAD_U8(dimAlphaAddr);
	auto pauseAlpha = g_pauseDimAlpha / 255.0f;
	auto target = dimAlpha / 255.0f;
	g_pauseDimAlpha = 0;

	// avoid double draw
	auto alpha = pauseAlpha < target ? (target - pauseAlpha) / (1.0f - pauseAlpha) : 0.0f;
	X86_STORE_U8(dimAlphaAddr, uint8_t(std::round(alpha * 255.0f)));
	__imp__sub_2A57E0(ctx, base);
	X86_STORE_U8(dimAlphaAddr, dimAlpha);
}

// matching sub_3065C0
static float GetGuestGlyphCentreY(uint8_t* base, uint32_t message)
{
	auto drawer = X86_LOAD_U32(message);
	auto refCount = X86_LOAD_U32(message + 0x4);
	if (refCount == 0 || X86_LOAD_U32(refCount + 0x4) == 0)
	{
		return 0.0f;
	}

	for (uint32_t i = 0; i < TEXT_DRAWER_FONT_COUNT; i++)
	{
		auto font = X86_LOAD_U32(drawer + TEXT_DRAWER_FONTS + i * 0x8);
		if (font != 0)
		{
			return 0.5f - X86_LOAD_F32(font + FONT_BASELINE);
		}
	}
	return 0.0f;
}

// match the game's text and option positions
// sub_1E5A0
// sub_2A6020
static void SetGuestMessageLayout(uint8_t* base, uint32_t message, size_t optionCount)
{
	auto originX = X86_LOAD_F32(GUEST_MESSAGE_ORIGIN);
	auto originY = X86_LOAD_F32(GUEST_MESSAGE_ORIGIN + 0x4) + X86_LOAD_F32(GUEST_MESSAGE_TEXT_HEIGHT) * GetGuestGlyphCentreY(base, message);
	auto scaleX = X86_LOAD_F32(GUEST_TEXT_SCALE_X);
	auto textX = originX + X86_LOAD_F32(GUEST_MESSAGE_TEXT_OFFSET);
	auto textY = originY + X86_LOAD_F32(GUEST_MESSAGE_TEXT_OFFSET + 0x4);

	// buttons
	auto row = GUEST_OPTION_ROWS + optionCount * 0x8;
	auto buttonY = originY - X86_LOAD_F32(GUEST_OPTION_ROW_DROP);
	std::vector<ImVec2> buttons;
	for (size_t i = 0; i < optionCount; i++)
	{
		auto buttonX = originX + X86_LOAD_F32(row) + X86_LOAD_F32(row + 0x4) * i;
		buttons.push_back(GuestTextToViewport(buttonX * scaleX, buttonY));
	}

	MessageWindow::SetLayout(GuestTextToViewport(textX * scaleX, textY), buttons);
}

static void ShowCustomMessage(uint8_t* base, const CustomMessage& custom, uint32_t message, int32_t index)
{
	auto descriptor = GUEST_MESSAGE_DESCRIPTORS + index * GUEST_MESSAGE_DESCRIPTOR_SIZE;

	// make sure our custom message only draw when the message box animation has advanced enough
	// e.g if opening, draw late, and if closing, stop rendering when the
	// message box size is small enough
	if (X86_LOAD_F32(message + MESSAGE_OPEN) < 1.0f)
	{
		if (MessageWindow::s_isVisible)
			MessageWindow::Close(true);

		return;
	}

	auto options = GetMessageOptions(base, descriptor);
	auto lastOption = std::max(int32_t(options.size()) - 1, 0);
	auto selection = std::clamp(int32_t(X86_LOAD_U32(message + MESSAGE_SELECTION)), 0, lastOption);

	MessageWindow::Open(Localise(custom.LocaleKey), &g_messageResult, options, selection, lastOption, true, true);
	MessageWindow::SetSelection(selection); // set user's input for button choice
	SetGuestMessageLayout(base, message, options.size());
}

// hack: prevent the game to draw its own text replace with our own one instead
X86_FUNC_IMPL(__imp__sub_2A5DD0);
X86_OVERRIDE_FUNC(sub_2A5DD0)
{
	auto message = ctx.ecx.u32;

	__imp__sub_2A5DD0(ctx, base);

	auto displayed = int32_t(X86_LOAD_U32(message + MESSAGE_DISPLAYED));
	auto custom = displayed >= 0 ? FindCustomMessage(GetMessageTextID(base, displayed)) : nullptr;

	if (custom == nullptr || displayed != g_messageIndex)
	{
		if (g_messageIndex >= 0) // The game closed our message or swapped it.
			CloseCustomMessage();

		if (custom == nullptr)
			return;

		g_messageIndex = displayed;
	}

	ShowCustomMessage(base, *custom, message, displayed);
}
