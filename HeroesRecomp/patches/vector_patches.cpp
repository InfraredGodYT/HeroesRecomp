#include <gpu/video.h>
#include <kernel/function.h>
#include <patches/global.h>

/*
// --- Performance patches. ---
// Perf pointed us here: Ate roughly ~7% of game-thread in areas
// that made 2P lag. Reimplemented by hand.

// sub_30ECB0(ecx=v*): distance squared. i.e. squared Euclidean length of v,
// i.e. (|v|^2 = x^2 + y^2 + z^2).
X86_FUNC_IMPL(__imp__sub_30ECB0);
X86_OVERRIDE_FUNC(sub_30ECB0) {
  // memcpy is faster here than 3 reads for some reason
  // baffled clang does not optimize this away.
  float v[3];
  std::memcpy(v, base + ctx.ecx.u32, sizeof(v));

  // Speed hack:
  // f64 is enough: f32 squares are exact in it, and rounding to full
  // f32 was what tanked the framerate before (313 -> 298 fps).
  const double xx = (double)v[0] * v[0];
  const double yy = (double)v[1] * v[1];
  const double zz = (double)v[2] * v[2];
  ctx.fpu.push((x87_float)(xx + yy + zz));
  ctx.esp.u32 += 0x4;
}
*/
