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
#if 0 // HEROES-TODO: Shadow address(es) sub_116B0 not matched in Heroes yet (docs/shadow-matches.md)
X86_OVERRIDE_FUNC(sub_116B0)
{
	uint64_t micros = Timer_GetScaledPerformanceCounter(1000000);
	ctx.eax.u32 = uint32_t(micros);
	ctx.edx.u32 = uint32_t(micros >> 32);
	ctx.esp.u32 += 4;
}
#endif // HEROES-TODO

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
GUEST_FUNCTION_HOOK_STDCALL(sub_2784A1, QueryPerformanceCounterImpl);
#if 0 // HEROES-TODO: Shadow address(es) sub_3D7000, sub_388458, sub_37FDA0, sub_380F60 not matched in Heroes yet (docs/shadow-matches.md)
GUEST_FUNCTION_HOOK(sub_3D7000, GetRdtSc);

GUEST_FUNCTION_HOOK(sub_388458, GetKeTickCountImpl);

GUEST_FUNCTION_HOOK(sub_37FDA0, memcpy);
GUEST_FUNCTION_HOOK(sub_380F60, memset);
#endif // HEROES-TODO

GUEST_FUNCTION_HOOK(sub_323E10, strcpy); //originally has an invalid size because the original function "share" some part of this function with another one, creating a false positive for when it end
GUEST_FUNCTION_HOOK(sub_27BEA0, strncpy);
GUEST_FUNCTION_HOOK(sub_323E20, strcat); //same as above
GUEST_FUNCTION_HOOK(sub_323CD0, strncat);
GUEST_FUNCTION_HOOK(sub_2B3B30, _StrChr);
GUEST_FUNCTION_HOOK(sub_2B3B50, _StrrChr);
#if 0 // HEROES-TODO: Shadow address(es) sub_37EB60 not matched in Heroes yet (docs/shadow-matches.md)
GUEST_FUNCTION_HOOK(sub_37EB60, strstr_);
#endif // HEROES-TODO
GUEST_FUNCTION_HOOK(sub_27C500, memmove);
//GUEST_FUNCTION_HOOK(sub_37E7BB, vsprintf); -> used for logs

#if 0 // HEROES-TODO: Shadow address(es) sub_3800E0 not matched in Heroes yet (docs/shadow-matches.md)
GUEST_FUNCTION_HOOK(sub_3800E0, strcmp_);
#endif // HEROES-TODO
GUEST_FUNCTION_HOOK(sub_27CAB0, strncmp_);
GUEST_FUNCTION_HOOK(sub_2B3A80, _stricmp);
GUEST_FUNCTION_HOOK(sub_323BB0, strlen);
GUEST_FUNCTION_HOOK(sub_2B3AD0, _strupr);
GUEST_FUNCTION_HOOK(sub_2B3B00, _strlwr);
#if 0 // HEROES-TODO: Shadow address(es) sub_38066D not matched in Heroes yet (docs/shadow-matches.md)
GUEST_FUNCTION_HOOK(sub_38066D, strtok);
#endif // HEROES-TODO


// The guest version ends in `ret 4`, so it pops its own argument.
GUEST_FUNCTION_HOOK_STDCALL(sub_323082, OutputDebugStringA);
