#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <numeric>

inline constexpr size_t PROFILER_VALUE_COUNT = 256;
extern size_t g_profilerValueIndex;

struct Profiler
{
	std::atomic<double> value;
	double values[PROFILER_VALUE_COUNT];
	std::chrono::steady_clock::time_point start;
	bool started = false;
	bool active = false;

	void Begin()
	{
		start = std::chrono::steady_clock::now();
		started = true;
		active = true;
	}

	void End()
	{
		if (!started)
			return;
		value = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
		active = false;
	}

	void Set(double v)
	{
		value = v;
	}

	void Reset()
	{
		End();
		Begin();
	}

	double UpdateAndReturnAverage()
	{
		values[g_profilerValueIndex] = value;
		return std::accumulate(values, values + PROFILER_VALUE_COUNT, 0.0) / PROFILER_VALUE_COUNT;
	}
};

struct FrameAccumulator
{
	std::atomic<double> value{ 0.0 };
	double values[PROFILER_VALUE_COUNT];
	double accum = 0.0;

	void Add(double ms) { accum += ms; }
	void SnapshotAndReset()
	{
		value = accum;
		accum = 0.0;
	}
	double UpdateAndReturnAverage()
	{
		values[g_profilerValueIndex] = value.load();
		return std::accumulate(values, values + PROFILER_VALUE_COUNT, 0.0) / PROFILER_VALUE_COUNT;
	}
};

extern std::atomic<bool> g_detailedProfilerEnabled;

// RAII scope timer that adds elapsed time to a FrameAccumulator.
struct RtScopeTimer
{
	std::chrono::steady_clock::time_point start;
	FrameAccumulator& accum;
	bool enabled;
	RtScopeTimer(FrameAccumulator& a) : accum(a), enabled(g_detailedProfilerEnabled.load(std::memory_order_relaxed))
	{
		if (enabled)
			start = std::chrono::steady_clock::now();
	}
	~RtScopeTimer()
	{
		if (enabled)
			accum.Add(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
	}
};

extern double g_frameIntervalMs;
extern Profiler g_gpuFrameProfiler;
extern Profiler g_presentProfiler;
extern Profiler g_updateDirectorProfiler;
extern Profiler g_renderDirectorProfiler;
extern Profiler g_frameFenceProfiler;
extern Profiler g_presentWaitProfiler;
extern Profiler g_swapChainAcquireProfiler;
extern Profiler g_frameEndWaitProfiler;

// --- Per-frame counters (last frame info) ---
extern uint32_t g_drawCallCountSnapshot;
extern uint32_t g_stateChangeCountSnapshot;
extern uint32_t g_totalCommandCountSnapshot;
extern uint32_t g_psoCacheHitsSnapshot;
extern uint32_t g_psoCacheMissesSnapshot;

// --- Render-thread breakdown (last frame, added up across calls) ---
extern FrameAccumulator g_rtPsoWorkAccum;
extern FrameAccumulator g_rtFlushWorkAccum;
extern FrameAccumulator g_rtDrawCallAccum;
extern FrameAccumulator g_rtQueueWaitAccum;
extern FrameAccumulator g_rtQueueWaitInFrameAccum;
extern FrameAccumulator g_gtDrawRecordingAccum;
extern FrameAccumulator g_gtStateFlushAccum;
extern FrameAccumulator g_rtOtherCmdsAccum;
extern FrameAccumulator g_rtDrawHandlerAccum;
extern FrameAccumulator g_rtBeginCmdListWorkAccum;

void DrawProfiler();
