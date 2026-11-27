#include <kernel/function.h>
#include <os/logger.h>


//some error print function
X86_FUNC_IMPL(__imp__sub_32EFE0);
X86_OVERRIDE_FUNC(sub_32EFE0)
{
	auto msg = X86_LOAD_U32(ctx.eax.u32);
	auto s = (char*)(base + msg);

	__imp__sub_32EFE0(ctx, base);
}


X86_FUNC_IMPL(__imp__sub_11430);
X86_OVERRIDE_FUNC(sub_11430)
{

	LOGN_WARNING("....RsRwInitialize....");
	__imp__sub_11430(ctx, base);
}

X86_FUNC_IMPL(__imp__sub_578B0);
X86_OVERRIDE_FUNC(sub_578B0)
{
	LOGN_UTILITY("Successfully initialized RenderWare.");
	LOGN_WARNING("....Now Loading Game Files....");
	__imp__sub_578B0(ctx, base);
}

X86_FUNC_IMPL(__imp__sub_28AF30);
X86_OVERRIDE_FUNC(sub_28AF30)
{
	LOGN_UTILITY("....ADVERTISE...");
	__imp__sub_28AF30(ctx, base);
}
