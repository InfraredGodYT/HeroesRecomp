#include <stdafx.h>
#include <kernel/function.h>
#include <cpu/guest_thread.h>
#include <kernel/kernel.h>
#include <user/config.h>
#include <audio/audio.h>
#include <audio/driver/sdl3_driver.h>


HRESULT DirectSoundCreate_r(uint32_t pcGuidDevice, uint32_t* ppDirectSound, uint32_t pUnkOuter)
{
	auto key = SHTHRSoundCreate(&ppDS);
	if (key <= 0)
	{
		return E_FAIL;
	}

	if (ppDirectSound)
	{
		*ppDirectSound = key;
	}
	return S_OK;
}

int32_t IDirectSoundBuffer_Lock(int32_t keyBuffer, int32_t dwOffset, int32_t dwBytes, uint32_t* ppvAudioPtr1, uint32_t* pdwAudioBytes1, uint32_t* ppvAudioPtr2, uint32_t* pdwAudioBytes2, uint32_t dwFlags)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}

	void* hostAudioPtr1 = nullptr;
	int32_t hostAudioBytes1 = 0;
	void* hostAudioPtr2 = nullptr;
	int32_t hostAudioBytes2 = 0;

	auto res = pThis->Lock(dwOffset, dwBytes, &hostAudioPtr1, &hostAudioBytes1, &hostAudioPtr2, &hostAudioBytes2, dwFlags);
	if (res != S_OK)
	{
		return res;
	}

	if (ppvAudioPtr1)
	{
		*ppvAudioPtr1 = hostAudioPtr1 ? g_memory.MapVirtual(hostAudioPtr1) : 0;
	}
	if (pdwAudioBytes1)
	{
		*pdwAudioBytes1 = hostAudioBytes1;
	}
	if (ppvAudioPtr2)
	{
		*ppvAudioPtr2 = hostAudioPtr2 ? g_memory.MapVirtual(hostAudioPtr2) : 0;
	}
	if (pdwAudioBytes2)
	{
		*pdwAudioBytes2 = hostAudioBytes2;
	}
	return res;
}

int32_t IDirectSoundBuffer_Unlock(int32_t keyBuffer, void* pvAudioPtr1, int32_t dwAudioBytes1, void* pvAudioPtr2, int32_t dwAudioBytes2)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
}

HRESULT IDirectSoundBuffer_GetStatus(int32_t keyBuffer, uint32_t* pdwStatus)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		if (pdwStatus)
		{
			*pdwStatus = 0;
		}
		return E_FAIL;
	}

	auto status = pThis->GetStatus(pdwStatus);
	if (pdwStatus)
	{
		*pdwStatus = status;
	}
	return S_OK;
}

// stub (not needed)
int32_t CDirectSound_DownloadEffectsImage(uint32_t pThis, uint32_t pvImageBuffer, uint32_t dwImageSize, uint32_t pImageLoc, uint32_t ppImageDesc)
{
	return S_OK;
}

int32_t IDirectSoundBuffer_GetCurrentPosition(int32_t keyBuffer, uint32_t* pdwCurrentPlayCursor, uint32_t* pdwCurrentWriteCursor)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}

	int32_t hostPlayCursor = 0;
	int32_t hostWriteCursor = 0;
	auto res = pThis->GetCurrentPosition(&hostPlayCursor, &hostWriteCursor);
	if (pdwCurrentPlayCursor)
	{
		*pdwCurrentPlayCursor = hostPlayCursor;
	}
	if (pdwCurrentWriteCursor)
	{
		*pdwCurrentWriteCursor = hostWriteCursor;
	}
	return res;
}

int32_t IDirectSoundBuffer_SetCurrentPosition(int32_t keyBuffer, int32_t dwNewPosition)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->SetCurrentPosition(dwNewPosition);
}

int32_t IDirectSound_CreateSoundBuffer(int32_t keyDevice, X_DSBUFFERDESC* pdsbd, uint32_t* ppBufferOut, uint32_t pUnkOuter)
{
	auto pDevice = ppDS;
	if (!pDevice)
	{
		return E_FAIL;
	}

	SHTHR_SOUNDBUFFER* buffer = nullptr;
	auto bufferKey = pDevice->CreateSoundBuffer(pdsbd, &buffer);
	if (bufferKey)
	{
		if (ppBufferOut)
		{
			*ppBufferOut = bufferKey;
		}
		return S_OK;
	}
	return E_FAIL;
}

int32_t IDirectSoundBuffer_SetBufferData(int32_t keyBuffer, void* pvBufferData, uint32_t dwBufferBytes)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->SetBufferData(pvBufferData, dwBufferBytes);
}

int32_t IDirectSoundBuffer_Play(int32_t keyBuffer, int32_t dwReserved1, int32_t dwReserved2, int32_t dwFlags)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->Play(dwReserved1, dwReserved2, dwFlags);
}

int32_t IDirectSoundBuffer_Stop(int32_t keyBuffer)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->Stop();
}

// this function had originally 3 params, but ended being 4 by the compiler due to the use of int 64 bits.
int32_t IDirectSoundBuffer_StopEx(int32_t keyBuffer, uint32_t rtTimeStamp_Low, uint32_t rtTimeStamp_High, dword_xt dwFlags)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}

	// Recombine into 64-bit value
	int64_t rtTimeStamp = (static_cast<int64_t>(rtTimeStamp_High) << 32) | rtTimeStamp_Low;
	return pThis->StopEx(rtTimeStamp, dwFlags);
}

int32_t IDirectSoundBuffer_Release(int32_t keyBuffer)
{
	auto pDevice = ppDS;
	if (!pDevice)
	{
		return E_FAIL;
	}

	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}

	pThis->Stop();
	pDevice->ReleaseSoundBuffer(pThis);
	delete pThis;
	return S_OK;
}

int32_t IDirectSoundBuffer_SetVolume(int32_t keyBuffer, int32_t lVolume)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->SetVolume(lVolume);
}

int32_t IDirectSoundBuffer_SetPosition(int32_t keyBuffer, float x, float y, float z, int32_t dwApply)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis || !pThis->m3DBuffer)
	{
		return E_FAIL;
	}
	return pThis->m3DBuffer->SetPosition(x, y, z, dwApply);
}

int32_t IDirectSoundBuffer_SetLoopRegion(int32_t keyBuffer, int32_t dwLoopStart, int32_t dwLoopLength)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->SetLoopRegion(dwLoopStart, dwLoopLength);
}

int32_t IDirectSound_SetAllParameters(int32_t keyDevice, Listener3D* pDS3DListenerParameters, int32_t dwApply)
{
	auto pDevice = ppDS;
	if (!pDevice)
	{
		return E_FAIL;
	}
	return pDevice->SetAllParameters(pDS3DListenerParameters);
}

int32_t IDirectSoundBuffer_SetMinDistance(int32_t keyBuffer, float flMinDistance, int32_t dwApply)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis || !pThis->m3DBuffer)
	{
		return E_FAIL;
	}
	return pThis->m3DBuffer->SetMinDistance(flMinDistance, dwApply);
}

// stub (not needed)
int32_t IDirectSoundBuffer_SetMixBinVolumes_8(int32_t keyBuffer, uint32_t pMixBins)
{
	return S_OK;
}

int32_t IDirectSoundBuffer_SetMaxDistance(int32_t keyBuffer, float flMaxDistance, int32_t dwApply)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis || !pThis->m3DBuffer)
	{
		return E_FAIL;
	}
	return pThis->m3DBuffer->SetMaxDistance(flMaxDistance, dwApply);
}

// stub (not needed)
int32_t IDirectSoundBuffer_SetMixBins(int32_t keyBuffer, uint32_t pMixBins)
{
	return S_OK;
}

// stub (not needed)
int32_t IDirectSoundBuffer_SetConeAngles(int32_t keyBuffer, int32_t dwInsideConeAngle, int32_t dwOutsideConeAngle, int32_t dwApply)
{
	return S_OK;
}

// stub (not needed)
int32_t IDirectSoundBuffer_SetPitch(int32_t keyBuffer, int32_t lPitch)
{
	return S_OK;
}

// stub (not needed)
int32_t IDirectSoundBuffer_SetHeadroom(int32_t keyBuffer, int32_t dwHeadroom)
{
	return S_OK;
}

int32_t IDirectSoundBuffer_SetFrequency(int32_t keyBuffer, int32_t dwFrequency)
{
	auto pThis = GetCurrentSoundBuffer(keyBuffer);
	if (!pThis)
	{
		return E_FAIL;
	}
	return pThis->SetFrequency(dwFrequency);
}

//====== IDIRECT SOUND FUNCTIONS ==========

// stub (not needed)
int32_t IDirectSound_SetI3DL2Listener(int32_t keyDevice, uint32_t pds3dlp, int32_t dwApply)
{
	return S_OK;
}

int32_t IDirectSound_SetDistanceFactor(int32_t keyDevice, float flDistanceFactor, int32_t dwApply)
{
	auto pDevice = ppDS;
	if (!pDevice)
	{
		return E_FAIL;
	}
	return pDevice->SetDistanceFactor(flDistanceFactor, dwApply);
}

int32_t IDirectSound_SetRolloffFactor(int32_t keyDevice, float flRolloffFactor, int32_t dwApply)
{
	auto pDevice = ppDS;
	if (!pDevice)
	{
		return E_FAIL;
	}
	return pDevice->SetRolloffFactor(flRolloffFactor, dwApply);
}

int32_t IDirectSound_SetPosition(int32_t keyDevice, float x, float y, float z, int32_t dwApply)
{
	auto pDevice = ppDS;
	if (!pDevice)
	{
		return E_FAIL;
	}
	return pDevice->SetPosition(x, y, z, dwApply);
}

// stub (not needed)
int32_t IDirectSound_SetOrientation(int32_t keyDevice, float xFront, float yFront, float zFront, float xTop, float yTop, float zTop, int32_t dwApply)
{
	return S_OK;
}

// stub (not needed)
int32_t CDirectSound_CommitDeferredSettings(uint32_t pThis)
{
	return S_OK;
}

// stub (not needed)
int32_t IDirectSound_CommitDeferredSettings(int32_t keyDevice)
{
	return S_OK;
}

void AudioConfigValueChangedCallback(X86Context* ctx, IConfigDef* config)
{
	if (config == &Config::VoiceLanguage)
	{
		auto base = g_memory.base;
#if 0 // HEROES-TODO: Shadow's voice-language switch (Shadow sound functions and variables); Heroes' equivalent unknown
		GuestToHostFunction<void>(*ctx, sub_25C750); // stop all sound

		const uint32_t newVoiceLang = (uint32_t)Config::VoiceLanguage.Value;
		X86_STORE_U32(0x54DD8C, newVoiceLang); // set voice language to new value (eng or jp)

		// Call ADXF_LoadPartitionNw to re load voice asf file
		if (newVoiceLang == (uint32_t)EVoiceLanguage::English)
		{
			GuestToHostFunction<void, CallConvention::Cdecl>(*ctx, sub_328BB0, 0, X86_LOAD_U32(0x51DDA0), 0, 0x54DED0);
		}
		else
		{
			GuestToHostFunction<void, CallConvention::Cdecl>(*ctx, sub_328BB0, 0, X86_LOAD_U32(0x51DD9C), 0, 0x54DED0);
		}

		// wait for the adx server to update its value properly (logic re used from IsndADX::CreateTask)
		int stat;
		int failSafeCount = 0; // I'm paranoid
		while ((stat = GuestToHostFunction<int>(*ctx, sub_328B90, 0)) != 3 && stat != 4)
		{
			GuestToHostFunction<void>(*ctx, sub_32AC10);
			GuestToHostFunction<void>(*ctx, sub_328F30);
			failSafeCount++;
			if (failSafeCount >= 5000)
			{
				break;
			}
		}
		X86_STORE_U32(0x51DD80, newVoiceLang);
#endif // HEROES-TODO
	}
}

GUEST_FUNCTION_HOOK_STDCALL(sub_340B9B, DirectSoundCreate_r);
GUEST_FUNCTION_HOOK_STDCALL(sub_3408CA, IDirectSound_CreateSoundBuffer);
GUEST_FUNCTION_HOOK_STDCALL(sub_34005D, IDirectSoundBuffer_SetBufferData);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F390, IDirectSoundBuffer_GetStatus);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F3E8, IDirectSoundBuffer_Lock);
#if 0 // HEROES-TODO: Shadow address(es) sub_3E34B8 not matched in Heroes yet (docs/shadow-matches.md)
GUEST_FUNCTION_HOOK_STDCALL(sub_3E34B8, IDirectSoundBuffer_Unlock);
#endif // HEROES-TODO
GUEST_FUNCTION_HOOK_STDCALL(sub_33E7D5, CDirectSound_DownloadEffectsImage);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F3AC, IDirectSoundBuffer_GetCurrentPosition);
#if 0 // HEROES-TODO: Shadow address(es) sub_3E4258 not matched in Heroes yet (docs/shadow-matches.md)
GUEST_FUNCTION_HOOK_STDCALL(sub_3E4258, IDirectSoundBuffer_SetCurrentPosition);
#endif // HEROES-TODO
GUEST_FUNCTION_HOOK_STDCALL(sub_33F310, IDirectSoundBuffer_Play);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F334, IDirectSoundBuffer_Stop);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F34C, IDirectSoundBuffer_StopEx);
GUEST_FUNCTION_HOOK_STDCALL(sub_33E6DD, IDirectSoundBuffer_Release);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F2BC, IDirectSoundBuffer_SetVolume);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F370, IDirectSoundBuffer_SetLoopRegion);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F2D8, IDirectSoundBuffer_SetPitch);
GUEST_FUNCTION_HOOK_STDCALL(sub_33F2F4, IDirectSoundBuffer_SetHeadroom);
GUEST_FUNCTION_HOOK_STDCALL(sub_33FA6E, IDirectSoundBuffer_SetFrequency);

//3D Sound
GUEST_FUNCTION_HOOK_STDCALL(sub_33FAEE, IDirectSoundBuffer_SetMinDistance);
#if 0 // HEROES-TODO: Shadow address(es) sub_3E4180 not matched in Heroes yet (docs/shadow-matches.md)
GUEST_FUNCTION_HOOK_STDCALL(sub_3E4180, IDirectSoundBuffer_SetMixBinVolumes_8);
#endif // HEROES-TODO
GUEST_FUNCTION_HOOK_STDCALL(sub_33FA8A, IDirectSoundBuffer_SetMixBins);
GUEST_FUNCTION_HOOK_STDCALL(sub_33FACA, IDirectSoundBuffer_SetMaxDistance);
GUEST_FUNCTION_HOOK_STDCALL(sub_33FAA6, IDirectSoundBuffer_SetConeAngles);
GUEST_FUNCTION_HOOK_STDCALL(sub_33FB12, IDirectSoundBuffer_SetPosition);
GUEST_FUNCTION_HOOK_STDCALL(sub_3408EE, IDirectSound_SetAllParameters);
GUEST_FUNCTION_HOOK_STDCALL(sub_3409D5, IDirectSound_SetI3DL2Listener);
GUEST_FUNCTION_HOOK_STDCALL(sub_34090E, IDirectSound_SetDistanceFactor);
GUEST_FUNCTION_HOOK_STDCALL(sub_3409B1, IDirectSound_SetRolloffFactor);
GUEST_FUNCTION_HOOK_STDCALL(sub_34097C, IDirectSound_SetPosition);
GUEST_FUNCTION_HOOK_STDCALL(sub_340932, IDirectSound_SetOrientation);
GUEST_FUNCTION_HOOK_STDCALL(sub_340450, IDirectSound_CommitDeferredSettings);
GUEST_FUNCTION_HOOK_STDCALL(sub_340128, CDirectSound_CommitDeferredSettings);

