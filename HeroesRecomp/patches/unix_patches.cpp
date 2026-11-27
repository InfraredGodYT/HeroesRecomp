#include <kernel/function.h>

// The game mixes lowercase and uppercase across files and code references. 
// This works fine on Windows / Xbox as casing is ignored. Unix-like systems however care...

enum ASCIITables : uint8_t
{
	c = 0x63, 
	s = 0x73,
	d = 0x64,
	K = 0x4B, 
	X = 0x58,
	p = 0x70,
};

const uint32_t enemyBkChaosDataAddr = 0x458234;
const uint32_t BkChaosDataAddr = 0x4529A8;
const uint32_t csd2PCommandAddr = 0x477318;
const uint32_t Csdfiles2pCommAddr = 0x477334;
const uint32_t CsdfilesCommanAddr = 0x477350;
const uint32_t Csdfiles2pRingAddr = 0x4773D0;
const uint32_t CsdfilesInfowiAddr = 0x4773B0;
const uint32_t CsdfilesBossgaAddr = 0x478000;
const uint32_t CsdfilesDarkgAddr = 0x4774BC;
const uint32_t CsdfilesHerogaAddr = 0x4774D0;
const uint32_t CsdfilesHissatAddr = 0x477554; //dark
const uint32_t CsdfilesHissat2Addr = 0x4775A0; //hero
const uint32_t CsdfilesHpbarAddr = 0x47805C; 
const uint32_t CsdfilesTwindoAddr = 0x477670;
const uint32_t CsdfilesPauseP1Addr = 0x477620;
const uint32_t CsdfilesPauseP2Addr = 0x47763C;
const uint32_t CsdfilesPauseP0Addr = 0x477654;
const uint32_t CsdfilesResultAddr = 0x478168;
const uint32_t CsdfilesRoutedAddr = 0x464ACC;
const uint32_t CsdfilesBcharaAddr = 0x454C7C;
const uint32_t CsdfilesVharroAddr = 0x478504;
const uint32_t CsdfilesEggbosAddr = 0x45696C;

const uint32_t CsdfilesDoomEyeAddr = 0x47818C;
const uint32_t CsdfilesEggmanAddr = 0x4781A4;
const uint32_t CsdfilesCharmyAddr = 0x4781BC;
const uint32_t CsdfilesMariaAddr = 0x4781D4;
const uint32_t CsdfilesEspioAddr = 0x4781EC;
const uint32_t CsdfilesVectorAddr = 0x478204;
const uint32_t CsdfilesOmegaAddr = 0x47821C;
const uint32_t CsdfilesRougeAddr = 0x478234;
const uint32_t CsdfilesAmyAddr = 0x47824C;
const uint32_t CsdfilesKnucklesAddr = 0x478260;
const uint32_t CsdfilesMilesAddr = 0x47827C;
const uint32_t CsdfilesSonicAddr = 0x478294;

const uint32_t CsdfilesBcharaBbullAddr = 0x45461C;
const uint32_t CsdfilesBcharaFalconAddr = 0x454EC0;
const uint32_t CsdfilesBcharaDoomAddr = 0x455244;
const uint32_t CsdfilesBcharaEbreakerAddr = 0x4554BC;
const uint32_t CsdfilesBcharaSanddAddr = 0x455928;
const uint32_t CsdfilesBcharaDevdoomAddr = 0x456F60;
const uint32_t CsdfilesMissionFailedAddr = 0x46EF9C;
const uint32_t Csdfiles2pStartAddr = 0x470940;
const uint32_t Csdfiles2pStarAddr = 0x477478;
const uint32_t CsdfilesMediamenuAddr = 0x4788C8;
const uint32_t CsdfilesMediaromTitleAddr = 0x4788EC;
const uint32_t CsdfilesTestromStgselAddr = 0x47892C;
const uint32_t CsdfilesTestromStgselTxdAddr = 0x478944;
const uint32_t CsdfilesTestromTitleAddr = 0x478984;
const uint32_t CsdfilesTestromTitleTxdAddr = 0x47899C;
const uint32_t CsdfilesTgsTitleAddr = 0x478A28;
const uint32_t OpSfdCreditsAddr = 0x479970;
const uint32_t SoundBankFormatAddr = 0x473555;




void PatchUnixStrings()
{
	auto base = g_memory.base;

	X86_STORE_U8(enemyBkChaosDataAddr + 7, K);
	X86_STORE_U8(BkChaosDataAddr + 1, K);
	X86_STORE_U8(csd2PCommandAddr, c);
	X86_STORE_U8(Csdfiles2pCommAddr, c);
	X86_STORE_U8(CsdfilesCommanAddr, c);
	X86_STORE_U8(Csdfiles2pRingAddr, c);
	X86_STORE_U8(CsdfilesInfowiAddr, c);
	X86_STORE_U8(CsdfilesBossgaAddr, c);
	X86_STORE_U8(CsdfilesDarkgAddr, c);
	X86_STORE_U8(CsdfilesHerogaAddr, c);
	X86_STORE_U8(CsdfilesHissatAddr, c);
	X86_STORE_U8(CsdfilesHissat2Addr, c);
	X86_STORE_U8(CsdfilesHpbarAddr, c);
	X86_STORE_U8(CsdfilesTwindoAddr, c);
	X86_STORE_U8(CsdfilesPauseP0Addr, c);
	X86_STORE_U8(CsdfilesPauseP1Addr, c);
	X86_STORE_U8(CsdfilesPauseP2Addr, c);
	X86_STORE_U8(CsdfilesResultAddr, c);
	X86_STORE_U8(CsdfilesRoutedAddr, c);
	X86_STORE_U8(CsdfilesBcharaAddr, c);
	X86_STORE_U8(CsdfilesVharroAddr, c);
	X86_STORE_U8(CsdfilesEggbosAddr, c);

	X86_STORE_U8(CsdfilesDoomEyeAddr, c);
	X86_STORE_U8(CsdfilesEggmanAddr, c);
	X86_STORE_U8(CsdfilesCharmyAddr, c);
	X86_STORE_U8(CsdfilesMariaAddr, c);
	X86_STORE_U8(CsdfilesEspioAddr, c);
	X86_STORE_U8(CsdfilesVectorAddr, c);
	X86_STORE_U8(CsdfilesOmegaAddr, c);
	X86_STORE_U8(CsdfilesRougeAddr, c);
	X86_STORE_U8(CsdfilesAmyAddr, c);
	X86_STORE_U8(CsdfilesKnucklesAddr, c);
	X86_STORE_U8(CsdfilesMilesAddr, c);
	X86_STORE_U8(CsdfilesSonicAddr, c);

	X86_STORE_U8(CsdfilesBcharaBbullAddr, c);
	X86_STORE_U8(CsdfilesBcharaFalconAddr, c);
	X86_STORE_U8(CsdfilesBcharaDoomAddr, c);
	X86_STORE_U8(CsdfilesBcharaEbreakerAddr, c);
	X86_STORE_U8(CsdfilesBcharaSanddAddr, c);
	X86_STORE_U8(CsdfilesBcharaSanddAddr + 1, s);
	X86_STORE_U8(CsdfilesBcharaSanddAddr + 2, d);
	X86_STORE_U8(CsdfilesBcharaDevdoomAddr, c);
	X86_STORE_U8(CsdfilesMissionFailedAddr, c);
	X86_STORE_U8(Csdfiles2pStartAddr, c);
	X86_STORE_U8(Csdfiles2pStarAddr, c);
	X86_STORE_U8(CsdfilesMediamenuAddr, c);
	X86_STORE_U8(CsdfilesMediaromTitleAddr, c);
	X86_STORE_U8(CsdfilesTestromStgselAddr, c);
	X86_STORE_U8(CsdfilesTestromStgselTxdAddr, c);
	X86_STORE_U8(CsdfilesTestromTitleAddr, c);
	X86_STORE_U8(CsdfilesTestromTitleTxdAddr, c);
	X86_STORE_U8(CsdfilesTgsTitleAddr, c);
	X86_STORE_U8(SoundBankFormatAddr, X);
	X86_STORE_U8(OpSfdCreditsAddr + 1, p);
}