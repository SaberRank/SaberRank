#include "Core/Gameplay/SnoreSaberGameplayModifiers.hpp"

namespace SnoreSaber::Core::Gameplay::SnoreSaberGameplayModifiers
{
    GameplayModifiersMap FromCodes(const std::vector<std::string>& modifiers, bool isPositiveModifiersEnabled)
    {
        double totalMultiplier = 1.0;
        auto energyType = GlobalNamespace::GameplayModifiers::EnergyType::Bar;
        auto obstacleType = GlobalNamespace::GameplayModifiers::EnabledObstacleType::All;
        auto songSpeed = GlobalNamespace::GameplayModifiers::SongSpeed::Normal;

        bool NF = false;
        bool IF = false;
        bool NB = false;
        bool DA = false;
        bool GN = false;
        bool NA = false;
        bool PM = false;
        bool SC = false;
        bool SA = false;

        for (const auto& modifier : modifiers)
        {
            if (modifier == "BE")
            {
                energyType = GlobalNamespace::GameplayModifiers::EnergyType::Battery;
            }
            else if (modifier == "NF")
            {
                totalMultiplier += -0.5;
                NF = true;
            }
            else if (modifier == "IF")
            {
                IF = true;
            }
            else if (modifier == "NO")
            {
                totalMultiplier += -0.05;
                obstacleType = GlobalNamespace::GameplayModifiers::EnabledObstacleType::NoObstacles;
            }
            else if (modifier == "NB")
            {
                totalMultiplier += -0.10;
                NB = true;
            }
            else if (modifier == "DA")
            {
                if (isPositiveModifiersEnabled)
                {
                    totalMultiplier += 0.02;
                }
                DA = true;
            }
            else if (modifier == "GN")
            {
                if (isPositiveModifiersEnabled)
                {
                    totalMultiplier += 0.04;
                }
                GN = true;
            }
            else if (modifier == "NA")
            {
                NA = true;
            }
            else if (modifier == "SS")
            {
                totalMultiplier += -0.3;
                songSpeed = GlobalNamespace::GameplayModifiers::SongSpeed::Slower;
            }
            else if (modifier == "FS")
            {
                if (isPositiveModifiersEnabled)
                {
                    totalMultiplier += 0.08;
                }
                songSpeed = GlobalNamespace::GameplayModifiers::SongSpeed::Faster;
            }
            else if (modifier == "SF")
            {
                songSpeed = GlobalNamespace::GameplayModifiers::SongSpeed::SuperFast;
            }
            else if (modifier == "PM")
            {
                PM = true;
            }
            else if (modifier == "SC")
            {
                SC = true;
            }
            else if (modifier == "SA")
            {
                SA = true;
            }
        }

        GameplayModifiersMap result;
        result.gameplayModifiers = GlobalNamespace::GameplayModifiers::New_ctor(energyType, NF, IF, false, obstacleType, NB, false, SA, DA, songSpeed, NA, GN, PM, false, SC);
        result.totalMultiplier = totalMultiplier;
        return result;
    }

    std::vector<std::string> ToCodeList(GlobalNamespace::GameplayModifiers* gameplayModifiers, float energy)
    {
        std::vector<std::string> results;
        if (gameplayModifiers->energyType == GlobalNamespace::GameplayModifiers::EnergyType::Battery)
        {
            results.push_back("BE");
        }
        if (gameplayModifiers->noFailOn0Energy && energy == 0)
        {
            results.push_back("NF");
        }
        if (gameplayModifiers->noFailOn0Energy && energy == -1)
        {
            results.push_back("NF");
        }
        if (gameplayModifiers->instaFail)
        {
            results.push_back("IF");
        }
        if (gameplayModifiers->failOnSaberClash)
        {
            results.push_back("SC");
        }
        if (gameplayModifiers->enabledObstacleType == GlobalNamespace::GameplayModifiers::EnabledObstacleType::NoObstacles)
        {
            results.push_back("NO");
        }
        if (gameplayModifiers->noBombs)
        {
            results.push_back("NB");
        }
        if (gameplayModifiers->strictAngles)
        {
            results.push_back("SA");
        }
        if (gameplayModifiers->disappearingArrows)
        {
            results.push_back("DA");
        }
        if (gameplayModifiers->ghostNotes)
        {
            results.push_back("GN");
        }
        if (gameplayModifiers->songSpeed == GlobalNamespace::GameplayModifiers::SongSpeed::Slower)
        {
            results.push_back("SS");
        }
        if (gameplayModifiers->songSpeed == GlobalNamespace::GameplayModifiers::SongSpeed::Faster)
        {
            results.push_back("FS");
        }
        if (gameplayModifiers->songSpeed == GlobalNamespace::GameplayModifiers::SongSpeed::SuperFast)
        {
            results.push_back("SF");
        }
        if (gameplayModifiers->smallCubes)
        {
            results.push_back("SC");
        }
        if (gameplayModifiers->proMode)
        {
            results.push_back("PM");
        }
        if (gameplayModifiers->noArrows)
        {
            results.push_back("NA");
        }
        return results;
    }
}
