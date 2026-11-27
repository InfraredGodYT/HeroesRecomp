#include "profiler.h"
#include "../video.h"
#include "../imgui/imgui_snapshot.h"
#include <app.h>
#include <kernel/heap.h>
#include <ui/game_window.h>
#include <user/config.h>

size_t g_profilerValueIndex;
std::atomic<bool> g_detailedProfilerEnabled{ false };

static double g_applicationValues[PROFILER_VALUE_COUNT];
double g_frameIntervalMs = 0.0;
Profiler g_gpuFrameProfiler;
Profiler g_presentProfiler;
Profiler g_updateDirectorProfiler;
Profiler g_renderDirectorProfiler;
Profiler g_frameFenceProfiler;
Profiler g_presentWaitProfiler;
Profiler g_swapChainAcquireProfiler;
Profiler g_frameEndWaitProfiler;

uint32_t g_drawCallCountSnapshot = 0;
uint32_t g_stateChangeCountSnapshot = 0;
uint32_t g_totalCommandCountSnapshot = 0;
uint32_t g_psoCacheHitsSnapshot = 0;
uint32_t g_psoCacheMissesSnapshot = 0;

FrameAccumulator g_rtPsoWorkAccum;
FrameAccumulator g_rtFlushWorkAccum;
FrameAccumulator g_rtDrawCallAccum;
FrameAccumulator g_rtQueueWaitAccum;
FrameAccumulator g_rtQueueWaitInFrameAccum;
FrameAccumulator g_gtDrawRecordingAccum;
FrameAccumulator g_gtStateFlushAccum;
FrameAccumulator g_rtOtherCmdsAccum;
FrameAccumulator g_rtDrawHandlerAccum;
FrameAccumulator g_rtBeginCmdListWorkAccum;

static bool g_profilerVisible;
static bool g_profilerWasToggled;

void DrawProfiler()
{
	bool toggleProfiler = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_F1] != 0;

	if (!g_profilerWasToggled && toggleProfiler)
	{
		g_profilerVisible = !g_profilerVisible;

		GameWindow::SetFullscreenCursorVisibility(App::s_isInit ? g_profilerVisible : true);
	}

	g_profilerWasToggled = toggleProfiler;
	g_detailedProfilerEnabled.store(g_profilerVisible, std::memory_order_relaxed);

	if (!g_profilerVisible)
		return;

	ImFont* font = ImFontAtlasSnapshot::GetFont("FOT-SeuratPro-DB.otf"); //ImGui::GetIO().Fonts->Fonts[0];

	float defaultScale = font->Scale;
	font->Scale = (ImGui::GetDefaultFont()->FontSize / font->FontSize) * Config::ProfilerFontScale.Value;
	ImGui::PushFont(font);

#define IMGUI_GENERIC_ROW(name, value, ...) \
	ImGui::TableNextColumn(); \
	ImGui::Text(name); \
	ImGui::TableNextColumn(); \
	ImGui::Text(value, __VA_ARGS__);

	if (ImGui::Begin("Profiler", &g_profilerVisible))
	{
		g_applicationValues[g_profilerValueIndex] = g_frameIntervalMs;

		const double applicationAvg = std::accumulate(g_applicationValues, g_applicationValues + PROFILER_VALUE_COUNT, 0.0) / PROFILER_VALUE_COUNT;
		double gpuFrameAvg = g_gpuFrameProfiler.UpdateAndReturnAverage();
		double presentAvg = g_presentProfiler.UpdateAndReturnAverage();
		double gameThreadAvg = g_updateDirectorProfiler.UpdateAndReturnAverage();
		double renderThreadWorkAvg = g_renderDirectorProfiler.UpdateAndReturnAverage();
		double frameFenceAvg = g_frameFenceProfiler.UpdateAndReturnAverage();
		double presentWaitAvg = g_presentWaitProfiler.UpdateAndReturnAverage();
		double swapChainAcquireAvg = g_swapChainAcquireProfiler.UpdateAndReturnAverage();
		double frameEndWaitAvg = g_frameEndWaitProfiler.UpdateAndReturnAverage();
		double rtPsoWorkAvg = g_rtPsoWorkAccum.UpdateAndReturnAverage();
		double rtFlushWorkAvg = g_rtFlushWorkAccum.UpdateAndReturnAverage();
		double rtDrawCallAvg = g_rtDrawCallAccum.UpdateAndReturnAverage();
		double rtOtherCmdsAvg = g_rtOtherCmdsAccum.UpdateAndReturnAverage();
		double rtBeginCmdListWorkAvg = g_rtBeginCmdListWorkAccum.UpdateAndReturnAverage();
		double rtDrawHandlerAvg = g_rtDrawHandlerAccum.UpdateAndReturnAverage();
		double rtQueueWaitAvg = g_rtQueueWaitAccum.UpdateAndReturnAverage();
		double rtQueueWaitInFrameAvg = g_rtQueueWaitInFrameAccum.UpdateAndReturnAverage();
		double gtDrawRecordingAvg = g_gtDrawRecordingAccum.UpdateAndReturnAverage();
		double gtStateFlushAvg = g_gtStateFlushAccum.UpdateAndReturnAverage();

		if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImPlot::BeginPlot("Frame Time"))
			{
				ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 20.0);
				ImPlot::SetupAxis(ImAxis_Y1, "ms", ImPlotAxisFlags_None);
				ImPlot::PlotLine<double>("Application", g_applicationValues, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("GPU Frame", g_gpuFrameProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("Present", g_presentProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("Game Thread", g_updateDirectorProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("Render-Thread Work", g_renderDirectorProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("Present Wait", g_presentWaitProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("Frame Fence", g_frameFenceProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("Swap Chain Acquire", g_swapChainAcquireProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::PlotLine<double>("Frame End Wait", g_frameEndWaitProfiler.values, PROFILER_VALUE_COUNT, 1.0, 0.0, ImPlotLineFlags_None, g_profilerValueIndex);
				ImPlot::EndPlot();
			}

			if (ImGui::Button("Copy Stats"))
			{
				double psoCur = g_rtPsoWorkAccum.value.load();
				double flushCur = g_rtFlushWorkAccum.value.load();
				double drawHandlerCur = g_rtDrawHandlerAccum.value.load();
				double drawCallCur = g_rtDrawCallAccum.value.load();
				double queueWaitCur = g_rtQueueWaitAccum.value.load();
				std::string text = fmt::format(
					"AVERAGES:\n"
					"Application          {:.4g} ms\n"
					"GPU Frame            {:.4g} ms\n"
					"Present              {:.4g} ms\n"
					"Game Thread          {:.4g} ms\n"
					"Render-Thread Work   {:.4g} ms\n"
					"Present Wait         {:.4g} ms\n"
					"Frame Fence          {:.4g} ms\n"
					"Swap Chain Acquire   {:.4g} ms\n"
					"Frame End Wait       {:.4g} ms\n"
					"\n"
					"RENDER-THREAD BREAKDOWN (averages):\n"
					"PSO Work             {:.4g} ms\n"
					"Per-Draw Flush       {:.4g} ms\n"
					"  (excl. PSO)        {:.4g} ms\n"
					"Draw Calls (API)     {:.4g} ms\n"
					"Draw Handlers (tot)  {:.4g} ms\n"
					"  Draw Pre-Work      {:.4g} ms\n"
					"Other Cmds           {:.4g} ms\n"
					"Begin CmdList Work   {:.4g} ms\n"
					"Queue Wait (starved) {:.4g} ms\n"
					"\n"
					"COMMAND COUNTS (last frame):\n"
					"Draw Calls            {}\n"
					"State Changes         {}\n"
					"Total Commands        {}\n"
					"PSO per Draw          {:.2f} us\n"
					"API Call per Draw     {:.2f} us\n"
					"PSO Cache Hits        {} / {} ({:.1f}%)\n"
					"\n"
					"GAME-THREAD BREAKDOWN (averages):\n"
					"Draw Recording        {:.4g} ms\n"
					"  State Flush         {:.4g} ms\n"
					"Present (in GT)       {:.4g} ms\n"
					"Game Logic            {:.4g} ms\n"
					"\n"
					"CURRENT:\n"
					"Application          {:.4g} ms ({:.1f} FPS)\n"
					"GPU Frame            {:.4g} ms ({:.1f} FPS)\n"
					"Render-Thread Work   {:.4g} ms\n"
					"PSO Work (current)   {:.4g} ms\n"
					"Per-Draw Flush (cur) {:.4g} ms\n"
					"Draw Handlers (cur)  {:.4g} ms\n"
					"  Draw Pre-Work(cur) {:.4g} ms\n"
					"Draw Calls (cur)     {:.4g} ms\n"
					"Other Cmds (cur)     {:.4g} ms\n"
					"Queue Wait (cur)     {:.4g} ms\n"
					"Draw Recording (cur) {:.4g} ms\n"
					"Present (in GT, cur) {:.4g} ms\n"
					"Game Logic (cur)     {:.4g} ms\n",
					applicationAvg, gpuFrameAvg, presentAvg, gameThreadAvg, renderThreadWorkAvg,
					presentWaitAvg, frameFenceAvg, swapChainAcquireAvg, frameEndWaitAvg,
					rtPsoWorkAvg, rtFlushWorkAvg, std::max(rtFlushWorkAvg - rtPsoWorkAvg, 0.0), rtDrawCallAvg,
					rtDrawHandlerAvg, std::max(rtDrawHandlerAvg - rtFlushWorkAvg - rtDrawCallAvg, 0.0),
					rtOtherCmdsAvg, rtBeginCmdListWorkAvg, rtQueueWaitAvg,
					g_drawCallCountSnapshot, g_stateChangeCountSnapshot, g_totalCommandCountSnapshot,
					g_drawCallCountSnapshot > 0 ? rtPsoWorkAvg * 1000.0 / g_drawCallCountSnapshot : 0.0,
					g_drawCallCountSnapshot > 0 ? rtDrawCallAvg * 1000.0 / g_drawCallCountSnapshot : 0.0,
					g_psoCacheHitsSnapshot, g_psoCacheHitsSnapshot + g_psoCacheMissesSnapshot,
					(g_psoCacheHitsSnapshot + g_psoCacheMissesSnapshot) > 0 ? 100.0 * g_psoCacheHitsSnapshot / (g_psoCacheHitsSnapshot + g_psoCacheMissesSnapshot) : 0.0,
					gtDrawRecordingAvg, gtStateFlushAvg, presentAvg, std::max(gameThreadAvg - gtDrawRecordingAvg - presentAvg, 0.0),
					g_frameIntervalMs, 1000.0 / std::max(g_frameIntervalMs, 0.001),
					g_gpuFrameProfiler.value.load(), (g_gpuFrameProfiler.value.load() > 0.0 ? 1000.0 / g_gpuFrameProfiler.value.load() : 0.0),
					g_renderDirectorProfiler.value.load(),
					psoCur, flushCur, drawHandlerCur, std::max(drawHandlerCur - flushCur - drawCallCur, 0.0), drawCallCur, g_rtOtherCmdsAccum.value.load(), queueWaitCur,
					g_gtDrawRecordingAccum.value.load(), g_presentProfiler.value.load(), std::max(g_updateDirectorProfiler.value.load() - g_gtDrawRecordingAccum.value.load() - g_presentProfiler.value.load(), 0.0));
				ImGui::SetClipboardText(text.c_str());
			}

			g_profilerValueIndex = (g_profilerValueIndex + 1) % PROFILER_VALUE_COUNT;

			if (ImGui::BeginTable("Performance", 5))
			{
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Current Time");
				ImGui::TableSetupColumn("Average Time");
				ImGui::TableSetupColumn("Current FPS");
				ImGui::TableSetupColumn("Average FPS");
				ImGui::TableHeadersRow();

				auto drawPerfRow = [&](const char* name, double ms, double msAvg, bool showFPS = false)
					{
						ImGui::TableNextColumn();
						ImGui::TextUnformatted(name);

						ImGui::TableNextColumn();
						ImGui::Text("%.3f ms (%.1f%%)", ms, g_frameIntervalMs > 0.0 ? ms / g_frameIntervalMs * 100.0 : 0.0);
						ImGui::TableNextColumn();
						ImGui::Text("%.3f ms (%.1f%%)", msAvg, applicationAvg > 0.0 ? msAvg / applicationAvg * 100.0 : 0.0);
						ImGui::TableNextColumn();

						if (showFPS)
							ImGui::Text("%.3f FPS", 1000.0 / std::max(ms, 0.001));
						else
							ImGui::TextUnformatted("N/A");

						ImGui::TableNextColumn();

						if (showFPS)
							ImGui::Text("%.3f FPS", 1000.0 / std::max(msAvg, 0.001));
						else
							ImGui::TextUnformatted("N/A");
					};

				auto drawPerfGroup = [&](const char* label)
					{
						ImGui::TableNextColumn();
						ImGui::TextDisabled("%s", label);

						ImGui::TableNextColumn();
						ImGui::TableNextColumn();
						ImGui::TableNextColumn();
						ImGui::TableNextColumn();
					};

				drawPerfRow("Application", g_frameIntervalMs, applicationAvg, true);
				drawPerfGroup("Work (non-idle)");
				drawPerfRow("    Game Thread", std::max(g_updateDirectorProfiler.value.load() - g_presentProfiler.value.load(), 0.0), std::max(gameThreadAvg - presentAvg, 0.0), true);
				drawPerfRow("    Render Thread (async)", std::max(g_renderDirectorProfiler.value.load() - g_rtQueueWaitInFrameAccum.value.load(), 0.0), std::max(renderThreadWorkAvg - rtQueueWaitInFrameAvg, 0.0), true);
				drawPerfRow("    GPU (async)", g_gpuFrameProfiler.value.load(), gpuFrameAvg, true);
				drawPerfGroup("Idle");
				drawPerfGroup("    Game Thread");
				drawPerfRow("        Present", g_presentProfiler.value.load(), presentAvg, false);
				drawPerfRow("            Frame End (wait)", g_frameEndWaitProfiler.value.load(), frameEndWaitAvg, false);
				drawPerfGroup("    Render Thread");
				drawPerfRow("        Frame Fence (wait)", g_frameFenceProfiler.value.load(), frameFenceAvg, false);
				drawPerfRow("        Swap Chain Acquire", g_swapChainAcquireProfiler.value.load(), swapChainAcquireAvg, false);
				drawPerfRow("        Present Wait (VSync)", g_presentWaitProfiler.value.load(), presentWaitAvg, false);
				drawPerfRow("        Queue Wait", g_rtQueueWaitInFrameAccum.value.load(), rtQueueWaitInFrameAvg, false);

				ImGui::EndTable();
			}

			if (ImGui::CollapsingHeader("Render-Thread Breakdown"))
			{
				if (ImGui::BeginTable("RT Breakdown", 4))
				{
					ImGui::TableSetupColumn("Name");
					ImGui::TableSetupColumn("Current");
					ImGui::TableSetupColumn("Average");
					ImGui::TableSetupColumn("% of Render Thread Work");
					ImGui::TableHeadersRow();

					auto drawRtRow = [&](const char* name, double cur, double avg)
						{
							double rtCur = g_renderDirectorProfiler.value.load();
							double rtAvg = renderThreadWorkAvg;
							ImGui::TableNextColumn();
							ImGui::TextUnformatted(name);

							ImGui::TableNextColumn();
							ImGui::Text("%.3f ms", cur);
							ImGui::TableNextColumn();
							ImGui::Text("%.3f ms", avg);
							ImGui::TableNextColumn();
							ImGui::Text("%.1f%% / %.1f%%", rtCur > 0.0 ? (cur / rtCur * 100.0) : 0.0, rtAvg > 0.0 ? (avg / rtAvg * 100.0) : 0.0);
						};

					auto drawRtGroup = [&](const char* label)
						{
							ImGui::TableNextColumn();
							ImGui::TextDisabled("%s", label);
							ImGui::TableNextColumn();
							ImGui::TableNextColumn();
							ImGui::TableNextColumn();
						};

					double psoCur = g_rtPsoWorkAccum.value.load();
					double flushCur = g_rtFlushWorkAccum.value.load();
					double drawHandlerCur = g_rtDrawHandlerAccum.value.load();
					double drawCallCur = g_rtDrawCallAccum.value.load();
					drawRtGroup("Work (non-idle)");
					drawRtRow("Begin CmdList Work", g_rtBeginCmdListWorkAccum.value.load(), rtBeginCmdListWorkAvg);
					drawRtRow("Other Cmds", g_rtOtherCmdsAccum.value.load(), rtOtherCmdsAvg);
					drawRtRow("Draw Handlers (total)", drawHandlerCur, rtDrawHandlerAvg);
					drawRtRow("  Draw Pre-Work", std::max(drawHandlerCur - flushCur - drawCallCur, 0.0),
						std::max(rtDrawHandlerAvg - rtFlushWorkAvg - rtDrawCallAvg, 0.0));
					drawRtRow("  Per-Draw Flush", flushCur, rtFlushWorkAvg);
					drawRtRow("    PSO Work", psoCur, rtPsoWorkAvg);
					drawRtRow("    (excl. PSO)", std::max(flushCur - psoCur, 0.0), std::max(rtFlushWorkAvg - rtPsoWorkAvg, 0.0));
					drawRtRow("  Draw Calls (API)", drawCallCur, rtDrawCallAvg);
					drawRtGroup("Idle");
					drawRtRow("Queue Wait (idle)", g_rtQueueWaitInFrameAccum.value.load(), rtQueueWaitInFrameAvg);
					double totalAcctCur = flushCur + drawCallCur + g_rtOtherCmdsAccum.value.load() + g_rtBeginCmdListWorkAccum.value.load() + g_rtQueueWaitInFrameAccum.value.load();
					double totalAcctAvg = rtFlushWorkAvg + rtDrawCallAvg + rtOtherCmdsAvg + rtBeginCmdListWorkAvg + rtQueueWaitInFrameAvg;
					ImGui::TableNextColumn(); ImGui::TextDisabled("Total Acct.");
					ImGui::TableNextColumn(); ImGui::TextDisabled("%.3f ms", totalAcctCur);
					ImGui::TableNextColumn(); ImGui::TextDisabled("%.3f ms", totalAcctAvg);
					ImGui::TableNextColumn(); ImGui::TextDisabled("(vs Render-Thread Work: %.3f ms)", renderThreadWorkAvg);

					ImGui::EndTable();
				}
				ImGui::Separator();
				ImGui::TextUnformatted("Per-Frame Command Counts");
				ImGui::SameLine();
				ImGui::TextDisabled("(last frame)");
				{
					double psoPerDraw = g_drawCallCountSnapshot > 0 ? g_rtPsoWorkAccum.value.load() / g_drawCallCountSnapshot : 0.0;
					double drawApiPerDraw = g_drawCallCountSnapshot > 0 ? g_rtDrawCallAccum.value.load() / g_drawCallCountSnapshot : 0.0;
					ImGui::Text("  Draw Calls:        %u", g_drawCallCountSnapshot);
					ImGui::Text("  State Changes:     %u", g_stateChangeCountSnapshot);
					ImGui::Text("  Total Commands:    %u", g_totalCommandCountSnapshot);
					ImGui::Text("  PSO / Draw:        %.2f us", psoPerDraw * 1000.0);
					ImGui::Text("  API Call / Draw:   %.2f us", drawApiPerDraw * 1000.0);
					uint32_t psoTotal = g_psoCacheHitsSnapshot + g_psoCacheMissesSnapshot;
					ImGui::Text("  PSO Cache Hits:    %u / %u (%.1f%%)", g_psoCacheHitsSnapshot, psoTotal,
						psoTotal > 0 ? 100.0 * g_psoCacheHitsSnapshot / psoTotal : 0.0);
				}
			}

			if (ImGui::CollapsingHeader("Game-Thread Breakdown"))
			{
				if (ImGui::BeginTable("GT Breakdown", 4))
				{
					ImGui::TableSetupColumn("Name");
					ImGui::TableSetupColumn("Current");
					ImGui::TableSetupColumn("Average");
					ImGui::TableSetupColumn("% of Game Thread");
					ImGui::TableHeadersRow();

					auto drawGtRow = [&](const char* name, double cur, double avg)
						{
							double gtCur = g_updateDirectorProfiler.value.load();
							double gtAvg = gameThreadAvg;
							ImGui::TableNextColumn();
							ImGui::TextUnformatted(name);

							ImGui::TableNextColumn();
							ImGui::Text("%.3f ms", cur);
							ImGui::TableNextColumn();
							ImGui::Text("%.3f ms", avg);
							ImGui::TableNextColumn();
							ImGui::Text("%.1f%% / %.1f%%", gtCur > 0.0 ? (cur / gtCur * 100.0) : 0.0, gtAvg > 0.0 ? (avg / gtAvg * 100.0) : 0.0);
						};

					double drawRecCur = g_gtDrawRecordingAccum.value.load();
					double stateFlushCur = g_gtStateFlushAccum.value.load();
					double presentCur = g_presentProfiler.value.load();
					double gameLogicCur = std::max(g_updateDirectorProfiler.value.load() - drawRecCur - presentCur, 0.0); // Game Thread includes present, so subtract (sub_57630)
					double gameLogicAvg = std::max(gameThreadAvg - gtDrawRecordingAvg - presentAvg, 0.0);
					drawGtRow("Game Logic", gameLogicCur, gameLogicAvg);
					drawGtRow("Draw Recording", drawRecCur, gtDrawRecordingAvg);
					drawGtRow("  State Flush", stateFlushCur, gtStateFlushAvg);
					drawGtRow("Present", presentCur, presentAvg);
					ImGui::EndTable();
				}
			}
		}

		if (ImGui::CollapsingHeader("Configuration", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextUnformatted("Profiler Font Size");
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("##ProfilerFontSize", &Config::ProfilerFontScale.Value, 0.1f, 2.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
			ImGui::PopItemWidth();

			ImGui::Separator();

			ImGui::Checkbox("Show FPS", &Config::ShowFPS.Value);
		}

		if (g_userHeap.heap != nullptr && g_userHeap.physicalHeap != nullptr)
		{
			if (ImGui::CollapsingHeader("Memory"))
			{
				O1HeapDiagnostics diagnostics, physicalDiagnostics;
				{
					std::lock_guard lock(g_userHeap.mutex);
					diagnostics = o1heapGetDiagnostics(g_userHeap.heap);
				}
				{
					std::lock_guard lock(g_userHeap.physicalMutex);
					physicalDiagnostics = o1heapGetDiagnostics(g_userHeap.physicalHeap);
				}

				if (ImGui::BeginTable("Memory", 2))
				{
					IMGUI_GENERIC_ROW("Heap Allocated", "%d MB", int32_t(diagnostics.allocated / (1024 * 1024)));
					IMGUI_GENERIC_ROW("Physical Heap Allocated", "%d MB", int32_t(physicalDiagnostics.allocated / (1024 * 1024)));

					ImGui::EndTable();
				}
			}
		}

		if (ImGui::CollapsingHeader("GPU"))
		{
			GpuInfo gpu = Video::GetGpuInfo();

			if (ImGui::BeginTable("GPU", 2))
			{
				IMGUI_GENERIC_ROW("API", "%s", gpu.api);

				if (auto pSDLVideoDriver = SDL_GetCurrentVideoDriver())
				{
					IMGUI_GENERIC_ROW("SDL Video Driver", "%s", pSDLVideoDriver);
				}

				IMGUI_GENERIC_ROW("Device", "%s", gpu.deviceName);
				IMGUI_GENERIC_ROW("Device Type", "%s", gpu.deviceType);
				IMGUI_GENERIC_ROW("VRAM", "%.2f MiB", gpu.vramMiB);
				IMGUI_GENERIC_ROW("GPU Waits", "%d", int32_t(gpu.gpuWaits));
				IMGUI_GENERIC_ROW("Buffer Uploads", "%d", int32_t(gpu.bufferUploads));

				IMGUI_GENERIC_ROW("Resolution", "%dx%d (%dx%d)",
					Video::s_viewportWidth, Video::s_viewportHeight,
					uint32_t(round(Video::s_viewportWidth * Config::ResolutionScale)),
					uint32_t(round(Video::s_viewportHeight * Config::ResolutionScale)));

				ImGui::EndTable();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Devices"))
			{
				ImGui::Indent();

				if (ImGui::BeginTable("Devices", 2))
				{
					auto deviceIndex = 0;

					for (const auto& deviceName : gpu.deviceNames)
					{
						ImGui::TableNextColumn();
						ImGui::Text("Device #%d", deviceIndex++);
						ImGui::TableNextColumn();
						ImGui::Text("%s", deviceName.c_str());
						ImGui::SameLine();
					}

					ImGui::EndTable();
				}

				ImGui::Unindent();
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Features"))
			{
				ImGui::Indent();

				if (ImGui::BeginTable("Features", 2))
				{
					IMGUI_GENERIC_ROW("Dynamic Depth Bias", "%s", gpu.dynamicDepthBias ? "Supported" : "Unsupported");
					IMGUI_GENERIC_ROW("GPU Upload Heap", "%s", gpu.gpuUploadHeap ? "Supported" : "Unsupported");
					IMGUI_GENERIC_ROW("Hardware Resolve Modes", "%s", gpu.resolveModes ? "Supported" : "Unsupported");
					IMGUI_GENERIC_ROW("Hardware Resolve", "%s", gpu.hardwareResolve ? "Enabled" : "Disabled");
					IMGUI_GENERIC_ROW("Hardware Depth Resolve", "%s", gpu.hardwareDepthResolve ? "Enabled" : "Disabled");
					IMGUI_GENERIC_ROW("Present Wait", "%s", gpu.presentWait ? "Supported" : "Unsupported");
					if (gpu.vulkan)
					{
						IMGUI_GENERIC_ROW("Mailbox Present Mode", "%s", gpu.mailboxPresentMode ? "Supported" : "Unsupported");
						IMGUI_GENERIC_ROW("Present Mode", "%s", gpu.presentMode);
					}
					IMGUI_GENERIC_ROW("Triangle Fan", "%s", gpu.triangleFan ? "Supported" : "Unsupported");
					IMGUI_GENERIC_ROW("UMA", "%s", gpu.uma ? "Supported" : "Unsupported");

					ImGui::EndTable();
				}

				ImGui::Unindent();
				ImGui::TreePop();
			}
		}
	}

#undef IMGUI_GENERIC_ROW

	ImGui::End();
	ImGui::PopFont();

	font->Scale = defaultScale;
}
