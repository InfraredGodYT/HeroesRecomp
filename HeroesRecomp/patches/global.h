#pragma once

typedef float RwReal;
#define RwUInt32 uint32_t
#define RwUInt16 uint16_t
#define RwInt16 int16_t
#define RwInt32 int32_t
#define RwUInt8 uint8_t

struct RwV3d
{
	float x;
	float y;
	float z;
};

enum RwMatrixFlag
{
	rwMATRIXINTERNALIDENTITY = 0x20000,
};

struct RwMatrixTag
{
	RwV3d right;
	RwMatrixFlag flags;
	RwV3d up;
	RwUInt32 pad1;
	RwV3d at;
	RwUInt32 pad2;
	RwV3d pos;
	RwUInt32 pad3;
};


struct GeoMesh
{
	float x, y, z;
	uint8_t r, g, b, a;
	float u, v;
};


struct WeightedMesh
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};

typedef RwMatrixTag RwMatrix;

extern bool isAwakeAboutToInvoke;
extern bool isAwakeAboutToRevoke;
extern thread_local bool g_isStartingEventAudio;
extern uint32_t g_multiplayerPlayerCount;
uint32_t GetAwakeDrainDivisor(uint32_t awake);
extern uint32_t g_playerStructAddrs[4];
RwV3d* GetPlayerPos(uint32_t slot);

struct RwObject
{
	RwUInt8 type;
	RwUInt8 subType;
	RwUInt8 flags;
	RwUInt8 privateFlags;
	xpointer<void> parent;
};

struct RwMemory
{
	xpointer<RwUInt8> start;
	RwUInt32 length;
};

struct RpMorphTarget;
struct RpGeometry
{
	RwObject object;
	RwUInt32 flags;
	RwUInt16 lockedSinceLastInst;
	RwInt16 refCount;
	RwInt32 numTriangles;
	RwInt32 numVertices;
	RwInt32 numMorphTargets;
	RwInt32 numTexCoordSets;
	uint8_t matList[0xC];
	uint32_t triangles; //ptr
	uint32_t preLitLum; //RwRGBA* ptr
	uint32_t texCoords[8]; // RwTexCoords* ptr
	uint32_t mesh; // RpMeshHeader* ptr
	uint32_t repEntry; // RwResEntry* ptr
	xpointer<RpMorphTarget> morphTarget; 
};

enum RwStreamType
{
	rwNASTREAM = 0x0,
	rwSTREAMFILE = 0x1,
	rwSTREAMFILENAME = 0x2,
	rwSTREAMMEMORY = 0x3,
	rwSTREAMCUSTOM = 0x4,
};

enum RwStreamAccessType
{
	rwNASTREAMACCESS = 0x0,
	rwSTREAMREAD = 0x1,
	rwSTREAMWRITE = 0x2,
	rwSTREAMAPPEND = 0x3,
};

struct RwStreamMemory
{
	RwUInt32 position;
	RwUInt32 nSize;
	xpointer<RwUInt8> memBlock;
};

union RwStreamFile
{
	xpointer<void> fpFile;
	xpointer<const void> constfpFile;
};

union RwStreamUnion
{
	RwStreamMemory memory;
	RwStreamFile file;
	uint8_t custom[0x14]; //RwStreamCustom
};

struct RwStream
{
	RwStreamType type;
	RwStreamAccessType accessType;
	RwInt32 position;
	RwStreamUnion Type;
	bool rwOwned;
};

struct TOneFile
{
	char fname[38];
	BYTE gap26[6];
	int32_t field_2C;
	BYTE gap30[4];
	int32_t assetStreamData;
	BYTE gap38[4];
	int32_t size;
	int32_t field_40;
	char field_44;
	int32_t pASyncFunc_HHC;
	int32_t field_4C;
	BYTE gap50[4];
	int32_t field_54;
};

struct TOneFileAsync
{
	uint8_t task[0x28];
	int32_t mode;
	int32_t threadFunc;
	int32_t pAsyncManager;
	int32_t fid;
	xpointer<void> dest;
	uint32_t clump; //xpointer<RpClump>
	xpointer<TOneFile> pOneFile;
	RwMemory _mem;
	xpointer<RwStream> stream;
	char field_50;
	int32_t texDict;
	char field_58;
	int32_t field_5C;
};

struct RwSphere
{
	RwV3d center;
	RwReal radius;
};

struct RpMorphTarget
{
	xpointer<RpGeometry> parentGeom;
	RwSphere boundingSphere;
	xpointer<RwV3d> verts;
	xpointer<RwV3d> normals;
};

struct X86Context;
char* TOneFileCheckFileName(TOneFile* this_, int32_t index);
