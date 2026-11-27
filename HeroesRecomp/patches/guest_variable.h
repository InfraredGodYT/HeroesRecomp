#pragma once

#include <kernel/memory.h>

// A variable in guest memory, with the type the game stores there.
//
// e.g.
//   static const GuestVariable<float> g_deltaTime{ 0x541FA8 };
//
//   float dt = g_deltaTime;   // read
//   g_deltaTime = 1 / 60.0f;  // write
//   g_deltaTime.address       // 0x541FA8, when the game needs a pointer
template<typename T>
struct GuestVariable
{
	uint32_t address;

	operator T() const
	{
		return *reinterpret_cast<volatile T*>(g_memory.base + address);
	}

	const GuestVariable& operator=(T value) const
	{
		*reinterpret_cast<volatile T*>(g_memory.base + address) = value;
		return *this;
	}
};
