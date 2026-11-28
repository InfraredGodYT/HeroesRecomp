// Heroes: kernel imports Heroes uses that ShadowRecomp never implemented (Shadow doesn't import them).
// All are stubs: they log once when first called, so we see whether the game actually reaches them.
// Parameter counts follow the Xbox kernel signatures (Cxbx-Reloaded / xboxkrnl headers). The hook
// macro reads that many arguments and, for stdcall, removes them from the guest stack on return.
// Guest pointers are passed as plain uint32_t guest addresses; none are dereferenced here.

#include <stdafx.h>
#include <kernel/function.h>
#include <os/logger.h>

static constexpr uint32_t STATUS_UNSUCCESSFUL = 0xC0000001;
static constexpr uint32_t STATUS_INVALID_DEVICE_REQUEST = 0xC0000010;

#define HEROES_STUB_LOG_ONCE(name) \
    do { static bool s_logged = false; if (!s_logged) { s_logged = true; LOGN_WARNING("!!! STUB !!! kernel import " name " called (heroes/kernel_stubs.cpp)"); } } while (0)

// @address kernel ordinal 358 (0x166)
// @original HalIsResetOrShutdownPending
// @status stub: BOOLEAN(void), always "no reset or shutdown pending"
// @tested builds only
uint32_t HalIsResetOrShutdownPending()
{
    HEROES_STUB_LOG_ONCE("HalIsResetOrShutdownPending");
    return 0;
}

// @address kernel ordinal 65 (0x041)
// @original IoCreateDevice(DriverObject, DeviceExtensionSize, DeviceName, DeviceType, Exclusive, DeviceObject*)
// @status stub: NTSTATUS, fails (no device created). Probably XAPI's storage/USB driver init.
// @tested builds only
uint32_t IoCreateDevice(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)
{
    HEROES_STUB_LOG_ONCE("IoCreateDevice");
    return STATUS_UNSUCCESSFUL;
}

// @address kernel ordinal 74 (0x04A)
// @original IoInvalidDeviceRequest(DeviceObject, Irp)
// @status stub: NTSTATUS. Normally used as a driver dispatch-table entry, not called directly.
// @tested builds only
uint32_t IoInvalidDeviceRequest(uint32_t, uint32_t)
{
    HEROES_STUB_LOG_ONCE("IoInvalidDeviceRequest");
    return STATUS_INVALID_DEVICE_REQUEST;
}

// @address kernel ordinal 359 (0x167)
// @original IoMarkIrpMustComplete(Irp)
// @status stub: void
// @tested builds only
void IoMarkIrpMustComplete(uint32_t)
{
    HEROES_STUB_LOG_ONCE("IoMarkIrpMustComplete");
}

// @address kernel ordinal 81 (0x051)
// @original IoStartNextPacket(DeviceObject)
// @status stub: void
// @tested builds only
void IoStartNextPacket(uint32_t)
{
    HEROES_STUB_LOG_ONCE("IoStartNextPacket");
}

// @address kernel ordinal 83 (0x053)
// @original IoStartPacket(DeviceObject, Irp, Key*)
// @status stub: void
// @tested builds only
void IoStartPacket(uint32_t, uint32_t, uint32_t)
{
    HEROES_STUB_LOG_ONCE("IoStartPacket");
}

// @address kernel ordinal 87 (0x057)
// @original IofCompleteRequest(Irp, PriorityBoost), fastcall (the "Iof" prefix)
// @status stub: void
// @tested builds only
void IofCompleteRequest(uint32_t, uint32_t)
{
    HEROES_STUB_LOG_ONCE("IofCompleteRequest");
}

// @address kernel ordinal 344 (0x158)
// @original XcVerifyPKCS1Signature(Signature, PublicKey, Digest)
// @status stub: BOOLEAN, always "valid". A retail Xbox accepts the game's own legitimately signed
//         data, so "valid" matches the expected outcome. Verify what Heroes uses it for when reached.
// @tested builds only
uint32_t XcVerifyPKCS1Signature(uint32_t, uint32_t, uint32_t)
{
    HEROES_STUB_LOG_ONCE("XcVerifyPKCS1Signature");
    return 1;
}

GUEST_FUNCTION_HOOK_STDCALL(__imp__HalIsResetOrShutdownPending, HalIsResetOrShutdownPending);
GUEST_FUNCTION_HOOK_STDCALL(__imp__IoCreateDevice, IoCreateDevice);
GUEST_FUNCTION_HOOK_STDCALL(__imp__IoInvalidDeviceRequest, IoInvalidDeviceRequest);
GUEST_FUNCTION_HOOK_STDCALL(__imp__IoMarkIrpMustComplete, IoMarkIrpMustComplete);
GUEST_FUNCTION_HOOK_STDCALL(__imp__IoStartNextPacket, IoStartNextPacket);
GUEST_FUNCTION_HOOK_STDCALL(__imp__IoStartPacket, IoStartPacket);
GUEST_FUNCTION_HOOK_FASTCALL(__imp__IofCompleteRequest, IofCompleteRequest);
GUEST_FUNCTION_HOOK_STDCALL(__imp__XcVerifyPKCS1Signature, XcVerifyPKCS1Signature);
