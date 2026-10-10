#include "Features/ScoreSubmission/ScoreSubmissionFeatureInstaller.hpp"

#include "Features/ScoreSubmission/ScoreSubmissionController.hpp"
#include "Features/ScoreSubmission/Services/ScoreSubmissionRegistry.hpp"
#include "Features/ScoreSubmission/Services/ScoreSubmissionService.hpp"
#include "Features/ScoreSubmission/Services/ScoreSubmissionWorkflow.hpp"
#include "Features/ScoreSubmission/Services/ScoreUploadPayloadBuilder.hpp"
#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/ConcreteIdBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>

DEFINE_TYPE(SnoreSaber::Features::ScoreSubmission, ScoreSubmissionFeatureInstaller);

namespace SnoreSaber::Features::ScoreSubmission
{
    void ScoreSubmissionFeatureInstaller::InstallBindings()
    {
        auto container = Container;
        container->Bind<Services::ScoreSubmissionRegistry*>()->AsSingle();
        container->Bind<Services::ScoreUploadPayloadBuilder*>()->AsSingle();
        container->Bind<Services::ScoreSubmissionWorkflow*>()->AsSingle();
        container->Bind<Services::ScoreSubmissionService*>()->AsSingle();
        container->BindInterfacesAndSelfTo<ScoreSubmissionController*>()->AsSingle()->NonLazy();
    }
}
