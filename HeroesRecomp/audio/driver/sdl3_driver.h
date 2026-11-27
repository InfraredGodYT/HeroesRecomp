
struct Vec3
{
	float x, y, z;
};

struct Listener3D
{
	uint32_t dwSize;

	Vec3 position;
	Vec3 velocity;
	Vec3 front;
	Vec3 top;

	float distanceFactor;
	float rolloffFactor;
	float dopplerFactor;
};

class Interface
{
public:
	virtual void Release();
};

class SHTHR_SOUND3DBUFFER : public Interface
{
public:
	virtual ~SHTHR_SOUND3DBUFFER() = default;
	virtual int GetMaxDistance(float* pflMaxDistance) PURE;
	virtual int GetMinDistance(float* pflMinDistance) PURE;
	virtual int GetPosition(float* pvPosition) PURE;
	virtual int SetMaxDistance(float flMaxDistance, int dwApply) PURE;
	virtual int SetMinDistance(float flMaxDistance, int dwApply) PURE;
	virtual int SetPosition(float x, float y, float z, int dwApply) PURE;
};


struct PullResult {
    int32_t bytesRead = 0;
    float mixVolume = 0.0f;
};

class SHTHR_SOUNDBUFFER : public Interface
{
public:

	SHTHR_SOUND3DBUFFER* m3DBuffer = nullptr;
	virtual ~SHTHR_SOUNDBUFFER() = default;
	virtual int GetStatus(uint32_t* pdwStatus) PURE;
	virtual int GetCurrentPosition(int* pdwCurrentPlayCursor, int* pdwCurrentWriteCursor) PURE;
	virtual int SetCurrentPosition(DWORD dwNewPosition) PURE;
	virtual int Lock(int dwOffset, int dwBytes, void** ppvAudioPtr1, int* pdwAudioBytes1, void** ppvAudioPtr2, int* pdwAudioBytes2, int dwFlags) PURE;
	virtual int Unlock(void* pvAudioPtr1, int dwAudioBytes1, void* pvAudioPtr2, int dwAudioBytes2) PURE;
	virtual int Play(int dwReserved1, int dwPriority, uint32_t dwFlags) PURE;
	virtual int Stop() PURE;
	virtual int StopEx(int64_t rtTimeStamp, uint32_t dwFlags) PURE;
	virtual int SetVolume(int lVolume) PURE;
	virtual void SetPan(float pan) PURE;
	virtual void CalcVolumeLinear() PURE;
	virtual int SetFrequency(int lFrequency) PURE;
	virtual int SetBufferData(void* pvBufferData, int dwBufferBytes) PURE;
	virtual int SetLoopRegion(int dwLoopStartOffset, int dwLoopLength) PURE;
};


class SHTHR_SOUNDDEVICE : public Interface
{
public:
	virtual ~SHTHR_SOUNDDEVICE() = default;
	virtual int32_t CreateSoundBuffer(X_DSBUFFERDESC* ppDesc, SHTHR_SOUNDBUFFER** ppBuffer) PURE;
	virtual bool ReleaseSoundBuffer(SHTHR_SOUNDBUFFER* ppBuffer) PURE;
	virtual int SetAllParameters(Listener3D* params) PURE;
	virtual int SetDistanceFactor(float flDistanceFactor, DWORD dwApply) PURE;
	virtual int SetRolloffFactor(float flRolloffFactor, DWORD dwApply) PURE;
	virtual int SetPosition(float x, float y, float z, DWORD dwApply) PURE;

};



int32_t SHTHRSoundCreate(SHTHR_SOUNDDEVICE** ppDevice);
extern SHTHR_SOUNDDEVICE* ppDS;
SHTHR_SOUNDBUFFER* GetCurrentSoundBuffer(const int32_t key);
void AudioShutDown();