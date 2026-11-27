#include "message_window.h"
#include <locale/locale.h>
#include <gpu/imgui/imgui_snapshot.h>
#include <ui/imgui_utils.h>
#include <app.h>
#include <input/input.h>
#include <sdl_listener.h>
#include <exports.h>
#include <input/padstate.h>


constexpr double OVERLAY_CONTAINER_COMMON_MOTION_START = 0;
constexpr double OVERLAY_CONTAINER_COMMON_MOTION_END = 11;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_START = 0;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_END = 4;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_START = 5;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_END = 9;

static bool g_isAwaitingResult = false;
static ImVec2 g_joypadAxis = {};
static bool g_isAccepted;
static bool g_isDeclined;
static bool g_isClosing = false;
static double g_time{};

static double g_appearTime;
static double g_controlsAppearTime;
static bool g_isControlsVisible = false;
static bool g_isControlsReady = false;
static int g_selectedRowIndex;
static int g_foregroundCount;

static double g_rowSelectionTime;
static int g_prevSelectedRowIndex;


static ImFont* g_fntSeurat;

std::string g_text;
int g_result;
static std::vector<std::string> g_buttons;
int g_defaultButtonIndex;
int g_cancelButtonIndex;
static uint32_t g_padIndex;

static bool g_upWasHeld;
static bool g_downWasHeld;
static bool g_leftWasHeld;
static bool g_rightWasHeld;
static bool g_hideFrame = false;
static bool g_guestHasControl = false;

static float g_openProgress = 0.0f;

static bool g_hasLayout = false;
static ImVec2 g_textAnchor;
static std::vector<ImVec2> g_buttonAnchors;
const float sclFontSize = 22.f;
static float g_textFitScale = 1.0f;

class SDLEventListenerForMessageWindow : public SDLEventListener
{
public:
	bool OnSDLEvent(SDL_Event* event) override
	{
		if (App::s_isInit || !MessageWindow::s_isVisible || !IsInputAllowed())
			return false;

		constexpr float axisValueRange = 32767.0f;
		constexpr float axisTapRange = 0.5f;

		ImVec2 tapDirection = {};

		switch (event->type)
		{
		case SDL_EVENT_KEY_DOWN:
		{
			switch (event->key.scancode)
			{
			case SDL_SCANCODE_UP:
				g_joypadAxis.y = -1.0f;
				break;

			case SDL_SCANCODE_DOWN:
				g_joypadAxis.y = 1.0f;
				break;

			case SDL_SCANCODE_RETURN:
			case SDL_SCANCODE_KP_ENTER:
				g_isAccepted = true;
				break;

			case SDL_SCANCODE_ESCAPE:
				g_isDeclined = true;
				break;
			}

			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			// Only accept left mouse button.
			if (event->button.button != SDL_BUTTON_LEFT)
				break;

			// Only accept mouse buttons when an item is selected.
			if (g_isControlsVisible && g_selectedRowIndex == -1)
				break;

			g_isAccepted = true;

			break;
		}

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		{
			switch (event->gbutton.button)
			{
			case SDL_GAMEPAD_BUTTON_DPAD_UP:
				g_joypadAxis = { 0.0f, -1.0f };
				break;

			case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
				g_joypadAxis = { 0.0f, 1.0f };
				break;

			case SDL_GAMEPAD_BUTTON_SOUTH:
				g_isAccepted = true;
				break;

			case SDL_GAMEPAD_BUTTON_EAST:
				g_isDeclined = true;
				break;
			}

			break;
		}

		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		{
			if (event->gaxis.axis < 2)
			{
				float newAxisValue = event->gaxis.value / axisValueRange;
				bool sameDirection = (newAxisValue * g_joypadAxis[event->gaxis.axis]) > 0.0f;
				bool wasInRange = abs(g_joypadAxis[event->gaxis.axis]) > axisTapRange;
				bool isInRange = abs(newAxisValue) > axisTapRange;

				if (sameDirection && !wasInRange && isInRange)
					tapDirection[event->gaxis.axis] = newAxisValue;

				g_joypadAxis[event->gaxis.axis] = newAxisValue;
			}

			break;
		}
		}

		return false;
	}
}
g_sdlEventListenerForMessageWindow;


void DrawMessageInWindow(const char* text, ImVec2 center, ImVec2 size)
{
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	auto maxWidth = Scale(820);
	auto fontSize = Scale(sclFontSize);

	const auto input = RemoveRubyAnnotations(g_text.c_str());
	auto lines = Split(input.first.c_str(), g_fntSeurat, fontSize, maxWidth);

	for (auto& line : lines)
	{
		line = ReAddRubyAnnotations(line, input.second);
	}

	// Text centered inside

	auto lineMargin = Config::Language != ELanguage::Japanese ? 5.0f : 5.5f;
	auto textSize = MeasureCentredParagraph(g_fntSeurat, fontSize, lineMargin, lines);

	auto textMarginX = Scale(37);
	auto textMarginY = Scale(45);

	auto textX = center.x;
	auto textY = center.y + Scale(3);

	if (Config::Language == ELanguage::Japanese)
	{
		textMarginX -= Scale(2.5f);
		textMarginY -= Scale(2.0f);

		textY += Scale(lines.size() % 2 == 0 ? 1.5f : 8.0f);
	}

	auto fitWidth = 2.0f * (std::min(center.x - dl->GetClipRectMin().x, dl->GetClipRectMax().x - center.x) - textMarginX);
	g_textFitScale = std::clamp(fitWidth / textSize.x, 0.5f, 1.0f);
	fontSize *= g_textFitScale;
	maxWidth *= g_textFitScale;
	textSize = MeasureCentredParagraph(g_fntSeurat, fontSize, lineMargin, lines);

	if (g_hasLayout)
	{
		// left align to match og advertise style
		textX = center.x - textSize.x / 2;
		textY = center.y - (textSize.y - Scale(lineMargin)) / 2;
	}

	DrawRubyAnnotatedText
	(
		g_fntSeurat,
		fontSize,
		maxWidth,
		{ textX, textY },
		lineMargin,
		g_text.c_str(),

		[=](const char* str, ImVec2 pos)
		{
			DrawTextWithShadow(g_fntSeurat, fontSize, pos, IM_COL32(255, 255, 255, 255), str, 3.f, 0.5f);
		},
		[=](const char* str, float size, ImVec2 pos)
		{
			// DrawTextWithShadow(g_fntSeurat, size, pos, IM_COL32(255, 255, 255, 255), str, 3.f, 0.5f);
		},

		!g_hasLayout
		);

	dl->PopClipRect();
}

void DrawButton(X86Context* ctx, int rowIndex, float yOffset, float width, float height, std::string& text)
{
	auto drawList = ImGui::GetBackgroundDrawList();

	auto clipRectMin = drawList->GetClipRectMin();
	auto clipRectMax = drawList->GetClipRectMax();

	auto totalButtons = (float)g_buttons.size();
	auto padding = Scale(20.f);
	auto totalWidth = width * totalButtons + padding * (totalButtons - 1);
	// calculate offset for position
	ImVec2 min = {
	clipRectMin.x + ((clipRectMax.x - clipRectMin.x) - totalWidth) / 2  // center the whole group
		+ rowIndex * (width + padding),                                   // offset per button
	clipRectMax.y - height - yOffset
	};
	ImVec2 max = { min.x + width, min.y + height };

	bool isSelected = rowIndex == g_selectedRowIndex;
	auto fontSize = Scale(sclFontSize) * g_textFitScale;
	auto textSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, text.c_str());

	ImVec2 textPos = { min.x + ((max.x - min.x) - textSize.x) / 2, min.y + ((max.y - min.y) - textSize.y) / 2 };
	if (g_hasLayout && rowIndex < g_buttonAnchors.size())
	{
		auto& res = ImGui::GetIO().DisplaySize;
		auto& anchor = g_buttonAnchors[rowIndex];
		textPos = { res.x * anchor.x - textSize.x / 2, res.y * anchor.y - textSize.y / 2 };
	}

	DrawTextWithShadow
	(
		g_fntSeurat,
		fontSize,
		textPos,
		isSelected ? IM_COL32(234, 234, 69, 255) : IM_COL32(255, 255, 255, 255),
		text.c_str()
	);
}

static float g_msgboxTime = 0.0f;
void DrawGameStyleMessageBox(ImVec2 center, ImVec2 size)
{
	ImDrawList* dl = ImGui::GetBackgroundDrawList();


	ImVec2 min = { center.x - size.x * 0.5f, center.y - size.y * 0.5f };
	ImVec2 max = { center.x + size.x * 0.5f, center.y + size.y * 0.5f };

	if (g_hideFrame)
	{
		max.x += 1.f;
		max.y += 1.f;
		dl->PushClipRect(min, max);
		return;
	}


	// the game make the whole screen darker behind the box (see 0x529C34)
	dl->AddRectFilled({ 0.0f, 0.0f }, ImGui::GetIO().DisplaySize, IM_COL32(0, 0, 0, 160));

	DrawShadowWindow(dl, min, max, Scale(1.0f), g_openProgress);

	max.x += 1.f;
	max.y += 1.f;
	dl->PushClipRect(min, max);
}

static void ResetSelection()
{
	g_selectedRowIndex = g_defaultButtonIndex;

	g_upWasHeld = false;
	g_downWasHeld = false;
	g_joypadAxis = {};
	g_isAccepted = false;
	g_isDeclined = false;
	g_hideFrame = false;
}

bool MessageWindow::Open(std::string text, int* result, std::span<std::string> buttons, int defaultButtonIndex, int cancelButtonIndex, bool hideFrame, bool guestHasControl, uint32_t padIndex)
{
	if (!g_isAwaitingResult && !g_isClosing && *result == -1)
	{

		g_isControlsReady = false;
		s_isVisible = true;
		g_isClosing = false;
		g_time = ImGui::GetTime();
		g_isControlsVisible = false;
		g_foregroundCount = 0;
		g_appearTime = ImGui::GetTime();
		g_controlsAppearTime = ImGui::GetTime();

		g_text = text;
		g_buttons = std::vector(buttons.begin(), buttons.end());
		g_defaultButtonIndex = defaultButtonIndex;
		g_cancelButtonIndex = cancelButtonIndex;
		g_padIndex = padIndex;

		ResetSelection();


		g_result = -1;
		g_hasLayout = false;
		g_hideFrame = hideFrame;
		g_guestHasControl = guestHasControl;
		g_openProgress = hideFrame ? 1.0f : 0.0f;
		g_isAwaitingResult = true;
	}

	*result = g_result;

	// Returns true when the message window is closed.
	return !g_isAwaitingResult;
}

void MessageWindow::Init()
{
	auto& io = ImGui::GetIO();

	g_fntSeurat = ImFontAtlasSnapshot::GetFont("FOT-SeuratPro-DB.otf");
}

void MessageWindow::Draw(X86Context* ctx)
{
	if (!s_isVisible)
		return;

	// recreate the message box opening and closing anim
	if (!g_hideFrame)
		g_openProgress = StepOpenProgress(g_openProgress, g_isClosing);

	bool isClosed = g_hideFrame ? (ImGui::GetTime() - g_time) > (1.0 / 60.0) * 15.f : g_openProgress <= 0.0f;

	if (g_isClosing && isClosed)
	{
		g_isAwaitingResult = false;
		g_isClosing = false;
		s_isVisible = false;
		return;
	}

	// arbitrary wait to lock control for a bit to avoid early press until message is ready.
	if (!g_isControlsReady && ImGui::GetTime() - g_time > (1.0 / 60.0) * 2.f)
	{
		g_isControlsReady = true;
	}
	else if (g_isControlsReady == false)
	{
		return;
	}

	auto isController = IsInputDeviceController();
	auto isKeyboard = input::g_inputDevice == input::EInputDevice::Keyboard;

	if (App::s_isInit)
	{
		// Always assume keyboard to prevent mouse from blocking control in-game.
		isKeyboard = true;

		if (auto pInputState = PJSPeripheral::GetControllerSlot(g_padIndex))
		{
			pInputState = pInputState;

			g_joypadAxis.x = -pInputState->leftX;

			if (PJSPeripheral::IsJustPressed(g_padIndex, PJSPeripheral::BTN_DPAD_LEFT))
				g_joypadAxis.x = 1.0f;

			if (PJSPeripheral::IsJustPressed(g_padIndex, PJSPeripheral::BTN_DPAD_RIGHT))
				g_joypadAxis.x = -1.0f;

			g_isAccepted = PJSPeripheral::IsJustPressed(g_padIndex, PJSPeripheral::BTN_A);
			g_isDeclined = PJSPeripheral::IsJustPressed(g_padIndex, PJSPeripheral::BTN_B);

			if (isKeyboard)
				g_isAccepted = g_isAccepted || PJSPeripheral::IsJustPressed(g_padIndex, PJSPeripheral::BTN_START);
		}
	}
	auto drawList = ImGui::GetBackgroundDrawList();
	auto& res = ImGui::GetIO().DisplaySize;

	ImVec2 centre = { res.x / 2, res.y / 2 };
	ImVec2 max = { res.x * 0.9f, res.y * 0.7f };

	DrawGameStyleMessageBox(centre, max);

	// show text once anim is complete
	if (g_openProgress < 1.0f)
	{
		drawList->PopClipRect();
		g_isAccepted = false;
		g_isDeclined = false;
		return;
	}

	centre = { res.x / 2, res.y / 2 * 0.8f };

	if (g_hasLayout)
	{
		centre = { res.x * g_textAnchor.x, res.y * g_textAnchor.y };
	}

	DrawMessageInWindow(g_text.c_str(), centre, max);

	if (isController || isKeyboard)
	{

		auto fontSize = Scale(28);
		auto itemWidth = std::max(Scale(162), Scale(CalcWidestTextSize(g_fntSeurat, fontSize, g_buttons)));
		auto itemHeight = Scale(120);
		auto windowMarginX = Scale(10);
		auto windowMarginY = Scale(80);

		ImVec2 controlsMax = { /* X */ itemWidth / 2 + windowMarginX, /* Y */ itemHeight / 2 * g_buttons.size() + windowMarginY };

		auto rowCount = 0;

		for (auto& button : g_buttons)
			DrawButton(ctx, rowCount++, windowMarginY, itemWidth, itemHeight, button);

		if (g_guestHasControl)
			return;

		auto leftIsHeld = g_joypadAxis.x > 0.5f;
		auto rightIsHeld = g_joypadAxis.x < -0.5f;
		auto scrollLeft = !g_leftWasHeld && leftIsHeld;
		auto scrollRight = !g_rightWasHeld && rightIsHeld;

		auto prevSelectedRowIndex = g_selectedRowIndex;

		if (scrollLeft)
		{
			if (g_selectedRowIndex > 0)
				--g_selectedRowIndex;
		}
		else if (scrollRight)
		{
			if (g_selectedRowIndex < rowCount - 1)
				++g_selectedRowIndex;
		}

		if (scrollLeft || scrollRight)
		{
			Game_PlaySound(ctx, "cursor");
			g_prevSelectedRowIndex = prevSelectedRowIndex;
			g_joypadAxis.x = 0;
		}

		g_leftWasHeld = leftIsHeld;
		g_rightWasHeld = rightIsHeld;

		if (g_isDeclined && !g_isClosing)
		{
			g_selectedRowIndex = g_cancelButtonIndex;
			g_result = g_cancelButtonIndex;
			Game_PlaySound(ctx, "cancel");
			MessageWindow::Close();
			g_isDeclined = false;
		}

		else //installer wizard thing
		{

		}

		if (g_selectedRowIndex != -1 && g_isAccepted && !g_isClosing)
		{
			g_result = g_selectedRowIndex;
			if (g_selectedRowIndex != g_cancelButtonIndex)
				Game_PlaySound(ctx, "decide");
			else
				Game_PlaySound(ctx, "cancel");
			MessageWindow::Close();
		}

	}
}


void MessageWindow::SetSelection(int index)
{
	g_selectedRowIndex = index;
}

void MessageWindow::SetLayout(ImVec2 textAnchor, std::span<ImVec2> buttonAnchors)
{
	g_hasLayout = true;
	g_textAnchor = textAnchor;
	g_buttonAnchors.assign(buttonAnchors.begin(), buttonAnchors.end());
}

void MessageWindow::Close(bool immediate)
{
	if (!g_isClosing)
	{
		g_appearTime = ImGui::GetTime();
		g_time = ImGui::GetTime();
		g_controlsAppearTime = ImGui::GetTime();
		g_isClosing = true;
		g_isControlsVisible = false;
		g_foregroundCount = 0;
		g_isAwaitingResult = false;

		//ButtonGuide::Close();
	}

	// Just like above: make sure our custom message only draw when the 
	// message box animation has advanced enough, unless there is no anim.
	if (immediate)
	{
		g_isClosing = false;
		s_isVisible = false;
	}
}
