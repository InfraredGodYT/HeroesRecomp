#include "stdafx.h"
#include <kernel/function.h>
#include <string>
#include <chrono>
#include <cstdint>


static constexpr uint32_t XBOX_TSC_FREQUENCY = 733333333; // Xbox Time Stamp Counter Frequency
static constexpr uint32_t XBOX_ACPI_FREQUENCY = 3375000;  // Xbox ACPI frequency (3.375 mhz)


static auto g_StartTime = std::chrono::steady_clock::now();
int64_t Timer_GetScaledPerformanceCounter(int64_t Period)
{
	auto currentTime = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
		currentTime - g_StartTime
	).count();

	constexpr int64_t NANOSECONDS_PER_SECOND = 1000000000;
	const int64_t whole = (elapsed / NANOSECONDS_PER_SECOND) * Period;
	const int64_t part = (elapsed % NANOSECONDS_PER_SECOND) * Period / NANOSECONDS_PER_SECOND;

	return whole + part;
}

static uint64_t HostGetPerformanceCounter(bool acpi)
{
	const int64_t period = acpi ? XBOX_ACPI_FREQUENCY : XBOX_TSC_FREQUENCY;
	return Timer_GetScaledPerformanceCounter(period);
}

// The game's microsecond timer.
// The original divides the 733.333...MHz counter by 733.
// So it's 0.045% faster than should be. Return real ones instead.
X86_OVERRIDE_FUNC(sub_116B0)
{
	uint64_t micros = Timer_GetScaledPerformanceCounter(1000000);
	ctx.eax.u32 = uint32_t(micros);
	ctx.edx.u32 = uint32_t(micros >> 32);
	ctx.esp.u32 += 4;
}

uint32_t GetKeTickCountImpl()
{
	//return GetTickCount();
	return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}


//has a huge impact on FPS
BOOL QueryPerformanceCounterImpl(LARGE_INTEGER* lpPerformanceCount)
{
	lpPerformanceCount->QuadPart = HostGetPerformanceCounter(false);
	return TRUE;
}

uint64_t GetRdtSc()
{
	return HostGetPerformanceCounter(false);
}


const char* strstr_(const char* src, const char* subStr)
{
	return strstr(src, subStr);
}

int strcmp_(const char* Str1, const char* Str2)
{

	//printf("strcmp Str1: %s, Str2: %s\n", Str1, Str2);
	return strcmp(Str1, Str2);
}

int __cdecl strncmp_(const char* Str1, const char* Str2, size_t MaxCount)
{
	return strncmp(Str1, Str2, MaxCount);
}

int sub_2C0390_r(int Str1)
{

	const char** t = (const char**)(g_memory.base + Str1);
	return 0;
}


char* __cdecl _StrChr(char* a1, char a2)
{
	return strchr(a1, a2);
}

char* __cdecl _StrrChr(char* a1, char a2)
{
	return strrchr(a1, a2);
}

//very important for timing and FPS
GUEST_FUNCTION_HOOK_STDCALL(sub_2AAF10, QueryPerformanceCounterImpl);
GUEST_FUNCTION_HOOK(sub_3D7000, GetRdtSc);

GUEST_FUNCTION_HOOK(sub_388458, GetKeTickCountImpl);

GUEST_FUNCTION_HOOK(sub_37FDA0, memcpy);
GUEST_FUNCTION_HOOK(sub_380F60, memset);

GUEST_FUNCTION_HOOK(sub_37FC10, strcpy); //originally has an invalid size because the original function "share" some part of this function with another one, creating a false positive for when it end
GUEST_FUNCTION_HOOK(sub_37F1E0, strncpy);
GUEST_FUNCTION_HOOK(sub_37FC20, strcat); //same as above
GUEST_FUNCTION_HOOK(sub_380720, strncat);
GUEST_FUNCTION_HOOK(sub_2EC170, _StrChr);
GUEST_FUNCTION_HOOK(sub_2EC190, _StrrChr);
GUEST_FUNCTION_HOOK(sub_37EB60, strstr_);
GUEST_FUNCTION_HOOK(sub_37E820, memmove);
//GUEST_FUNCTION_HOOK(sub_37E7BB, vsprintf); -> used for logs

GUEST_FUNCTION_HOOK(sub_3800E0, strcmp_);
GUEST_FUNCTION_HOOK(sub_37DEC0, strncmp_);
GUEST_FUNCTION_HOOK(sub_2EC0C0, _stricmp);
GUEST_FUNCTION_HOOK(sub_37FD10, strlen);
GUEST_FUNCTION_HOOK(sub_2EC110, _strupr);
GUEST_FUNCTION_HOOK(sub_2EC140, _strlwr);
GUEST_FUNCTION_HOOK(sub_38066D, strtok);


// The guest version ends in `ret 4`, so it pops its own argument.
GUEST_FUNCTION_HOOK_STDCALL(sub_2AAEDE, OutputDebugStringA);
