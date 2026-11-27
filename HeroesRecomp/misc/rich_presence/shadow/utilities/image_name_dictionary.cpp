#include "image_name_dictionary.h"

namespace RichPresence::ImageNameDictionary
{
    const char* TryGetValue(StageID stageId)
    {
        switch (stageId)
        {
            case StageID::TestLevel:               return "stg0000";
            case StageID::TestLevelSkatePark:      return "stg0001";
            case StageID::TestLevelUVTextureMap:   return "stg0003";
            case StageID::TestLevelSurfaceTest:    return "stg0002";
            case StageID::TestLevelCutsceneTest:   return "stg0005";

            case StageID::Westopolis:              return "westopolis";

            case StageID::DigitalCircuit:          return "digitalcircuit";
            case StageID::GlyphicCanyon:           return "glyphiccanyon";
            case StageID::LethalHighway:           return "lethalhighway";

            case StageID::LethalHighwayBlackBull:  return "lethalhighwayblackbull";

            case StageID::CrypticCastle:           return "crypticcastle";
            case StageID::PrisonIsland:            return "prisonisland";
            case StageID::CircusPark:              return "circuspark";

            case StageID::CrypticCastleEggBreaker: return "crypticcastleeggdealer";

            case StageID::CentralCity:             return "centralcity";
            case StageID::TheDoom:                 return "thedoom";
            case StageID::SkyTroops:               return "skytroops";
            case StageID::MadMatrix:               return "madmatrix";
            case StageID::DeathRuins:              return "deathruins";

            case StageID::HeavyDog:                return "thedoombluefalcon";
            case StageID::MadMatrixEggBreaker:     return "madmatrixeggbreaker";
            case StageID::DeathRuinsBlackBull:     return "deathruinsblackbull";

            case StageID::TheARK:                  return "theark";
            case StageID::AirFleet:                return "airfleet";
            case StageID::IronJungle:              return "ironjungle";
            case StageID::SpaceGadget:             return "spacegadget";
            case StageID::LostImpact:              return "lostimpact";

            case StageID::BlueFalcon:              return "thearkbluefalcon";
            case StageID::IronJungleEggBreaker:    return "ironfleeteggbreaker";

            case StageID::GUNFortress:             return "gunfortress";
            case StageID::BlackComet:              return "blackcomet";
            case StageID::LavaShelter:             return "lavashelter";
            case StageID::CosmicFall:              return "cosmicfall";
            case StageID::FinalHaunt:              return "finalhaunt";

            case StageID::GUNFortressBlackDoom:    return "gunfortressdoom";
            case StageID::GUNFortressDiablon:      return "gunfortressdiablon";
            case StageID::BlackCometEggDealer:     return "blackcometeggdealer";
            case StageID::BlackCometDiablon:       return "blackcometdiablon";
            case StageID::LavaShelterEggDealer:    return "lavasheltereggdealer";
            case StageID::CosmicFallEggDealer:     return "cosmicfalleggdealer";
            case StageID::CosmicFallBlackDoom:     return "cosmicfallblackdoom";
            case StageID::FinalHauntBlackDoom:     return "finalhauntdoom";
            case StageID::FinalHauntDiablon:       return "finalhauntdiablon";

            case StageID::TheLastWay:              return "thelastway";
            case StageID::DevilDoom:               return "devildoom";
            case StageID::HeroesAssetsLevel:       return "stg9900";

            default:                               return nullptr;
        }
    }
}
