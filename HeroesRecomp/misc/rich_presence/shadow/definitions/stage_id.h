#pragma once

#include <cstdint>

namespace RichPresence
{
    enum class StageID : int32_t
    {
        TestLevel               = 0,
        TestLevelSkatePark      = 1,
        TestLevelUVTextureMap   = 2,
        TestLevelSurfaceTest    = 3,
        TestLevelCutsceneTest   = 4,

        Westopolis              = 100,
        DigitalCircuit          = 200,

        GlyphicCanyon           = 201,
        LethalHighway           = 202,

        LethalHighwayBlackBull  = 210,

        CrypticCastle           = 300,
        PrisonIsland            = 301,
        CircusPark              = 302,

        CrypticCastleEggBreaker = 310,

        CentralCity             = 400,
        TheDoom                 = 401,
        SkyTroops               = 402,
        MadMatrix               = 403,
        DeathRuins              = 404,

        HeavyDog                = 410,
        MadMatrixEggBreaker     = 411,
        DeathRuinsBlackBull     = 412,

        TheARK                  = 500,
        AirFleet                = 501,
        IronJungle              = 502,
        SpaceGadget             = 503,
        LostImpact              = 504,

        BlueFalcon              = 510,
        IronJungleEggBreaker    = 511,

        GUNFortress             = 600,
        BlackComet              = 601,
        LavaShelter             = 602,
        CosmicFall              = 603,
        FinalHaunt              = 604,

        GUNFortressBlackDoom    = 610,
        GUNFortressDiablon      = 611,
        BlackCometEggDealer     = 612,
        BlackCometDiablon       = 613,
        LavaShelterEggDealer    = 614,
        CosmicFallEggDealer     = 615,
        CosmicFallBlackDoom     = 616,
        FinalHauntBlackDoom     = 617,
        FinalHauntDiablon       = 618,

        TheLastWay              = 700,
        DevilDoom               = 710,

        TwoPlayerStageOne       = 800,
        TwoPlayerStageTwo       = 801,
        TwoPlayerStageThree     = 802,

        RemovedTestLevelSTG6000 = 6000,
        HeroesAssetsLevel       = 9900,
        ProtoShadowCreationRoom = 9901,
    };
}
