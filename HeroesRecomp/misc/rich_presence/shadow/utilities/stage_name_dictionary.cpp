#include "stage_name_dictionary.h"

namespace RichPresence::StageNameDictionary
{
    const char* TryGetValue(StageID stageId)
    {
        switch (stageId)
        {
            case StageID::TestLevel:               return "Test Level";
            case StageID::TestLevelSkatePark:      return "Test Level (Skate Park)";
            case StageID::TestLevelUVTextureMap:   return "Test Level (UV Test)";
            case StageID::TestLevelSurfaceTest:    return "Test Level (Surface Test)";
            case StageID::TestLevelCutsceneTest:   return "Test Level (Cutscene Test)";

            case StageID::Westopolis:              return "Westopolis";

            case StageID::DigitalCircuit:          return "Digital Circuit";
            case StageID::GlyphicCanyon:           return "Glyphic Canyon";
            case StageID::LethalHighway:           return "Lethal Highway";

            case StageID::LethalHighwayBlackBull:  return "Black Bull (Lethal Highway)";

            case StageID::CrypticCastle:           return "Cryptic Castle";
            case StageID::PrisonIsland:            return "Prison Island";
            case StageID::CircusPark:              return "Circus Park";

            case StageID::CrypticCastleEggBreaker: return "Egg Breaker (Cryptic Castle)";

            case StageID::CentralCity:             return "Central City";
            case StageID::TheDoom:                 return "The Doom";
            case StageID::SkyTroops:               return "Sky Troops";
            case StageID::MadMatrix:               return "Mad Matrix";
            case StageID::DeathRuins:              return "Death Ruins";

            case StageID::HeavyDog:                return "Heavy Dog";
            case StageID::MadMatrixEggBreaker:     return "Egg Breaker (Mad Matrix)";
            case StageID::DeathRuinsBlackBull:     return "Black Bull (Death Ruins)";

            case StageID::TheARK:                  return "The ARK";
            case StageID::AirFleet:                return "Air Fleet";
            case StageID::IronJungle:              return "Iron Jungle";
            case StageID::SpaceGadget:             return "Space Gadget";
            case StageID::LostImpact:              return "Lost Impact";

            case StageID::BlueFalcon:              return "Blue Falcon";
            case StageID::IronJungleEggBreaker:    return "Egg Breaker (Iron Jungle)";

            case StageID::GUNFortress:             return "GUN Fortress";
            case StageID::BlackComet:              return "Black Comet";
            case StageID::LavaShelter:             return "Lava Shelter";
            case StageID::CosmicFall:              return "Cosmic Fall";
            case StageID::FinalHaunt:              return "Final Haunt";

            case StageID::GUNFortressBlackDoom:    return "Black Doom (GUN Fortress)";
            case StageID::GUNFortressDiablon:      return "Diablon (GUN Fortress)";
            case StageID::BlackCometEggDealer:     return "Egg Dealer (Black Comet)";
            case StageID::BlackCometDiablon:       return "Diablon (Black Comet)";
            case StageID::LavaShelterEggDealer:    return "Egg Dealer (Lava Shelter)";
            case StageID::CosmicFallEggDealer:     return "Egg Dealer (Cosmic Fall)";
            case StageID::CosmicFallBlackDoom:     return "Black Doom (Cosmic Fall)";
            case StageID::FinalHauntBlackDoom:     return "Black Doom (Final Haunt)";
            case StageID::FinalHauntDiablon:       return "Diablon (Final Haunt)";

            case StageID::TheLastWay:              return "The Last Way";
            case StageID::DevilDoom:               return "Devil Doom";

            case StageID::TwoPlayerStageOne:       return "2P GUN Fortress A";
            case StageID::TwoPlayerStageTwo:       return "2P GUN Fortress B";
            case StageID::TwoPlayerStageThree:     return "2P Lava Shelter";

            case StageID::RemovedTestLevelSTG6000: return "Removed/Empty Test Level [STG6000]";
            case StageID::HeroesAssetsLevel:       return "Heroes Assets Test Level [STG9900] (GameCube Only)";
            case StageID::ProtoShadowCreationRoom: return "Prototype Shadow Creation Room [STG9901]";
        }

        return nullptr;
    }
}
