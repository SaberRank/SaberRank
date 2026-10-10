#include "Features/Replays/Installers/ImberInstaller.hpp"

#include "Features/Replays/UI/ImberManager.hpp"
#include "Features/Replays/UI/ImberScrubber.hpp"
#include "Features/Replays/UI/ImberSpecsReporter.hpp"
#include "Features/Replays/UI/ImberUIPositionController.hpp"
#include "Features/Replays/UI/MainImberPanelView.hpp"
#include "Features/Replays/UI/SpectateAreaController.hpp"
#include "Features/Replays/UI/VRControllerAccessor.hpp"
#include "Features/Replays/ReplayStateRegistry.hpp"
#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/ConcreteIdBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>
#include <lapiz/shared/utilities/ZenjectExtensions.hpp>
#include "logging.hpp"

DEFINE_TYPE(SnoreSaber::ReplaySystem::Installers, ImberInstaller);

using namespace Lapiz::Zenject::ZenjectExtensions;

namespace SnoreSaber::ReplaySystem::Installers
{
    void ImberInstaller::InstallBindings()
    {
        if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsModernPlaybackEnabled()) {
            auto container = Container;
            container->Bind<UI::VRControllerAccessor*>()->AsSingle();
            container->BindInterfacesTo<UI::ImberManager*>()->AsSingle();
            container->BindInterfacesAndSelfTo<UI::ImberScrubber*>()->AsSingle();
            container->BindInterfacesAndSelfTo<UI::ImberSpecsReporter*>()->AsSingle();
            container->BindInterfacesAndSelfTo<UI::ImberUIPositionController*>()->AsSingle();
            FromNewComponentAsViewController(container->Bind<UI::MainImberPanelView*>())->AsSingle();
            container->BindInterfacesAndSelfTo<UI::SpectateAreaController*>()->AsSingle();
        }
    }
} // namespace SnoreSaber::ReplaySystem::Installers
