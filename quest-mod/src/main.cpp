#include "main.hpp"
#include "hooks.hpp"
#include "logging.hpp"

#include <GlobalNamespace/GameScenesManager.hpp>
#include <GlobalNamespace/HealthWarningFlowCoordinator.hpp>
#include <GlobalNamespace/MultiplayerLocalActivePlayerInstaller.hpp>
#include <GlobalNamespace/StandardGameplayInstaller.hpp>

#include "Features/Replays/Format/ReplayReader.hpp"
#include "Data/Private/Settings.hpp"
#include "Core/AppInstaller.hpp"
#include "Core/SnoreSaberRuntimeInfo.hpp"
#include "MainInstaller.hpp"
#include "Features/Replays/ReplayFeatureInstaller.hpp"
#include "Features/Replays/Installers/ImberInstaller.hpp"
#include "Features/Replays/Installers/PlaybackInstaller.hpp"
#include "Features/Replays/Installers/RecordInstaller.hpp"
#include "Features/Live/LiveGameplayInstaller.hpp"
#include "Features/Replays/Playback/ReplayPlaybackRegistry.hpp"
#include "Features/Replays/ReplayStateRegistry.hpp"
#include "Services/FileService.hpp"
#include "Features/Players/Services/PlayerService.hpp"
#include "Services/ReplayService.hpp"
#include "Features/Players/Profile/ProfilePictureView.hpp"
#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"
#include "Utils/TeamUtils.hpp"
#include <bsml/shared/BSML.hpp>
#include <bsml/shared/BSMLDataCache.hpp>
#include <lapiz/shared/zenject/Zenjector.hpp>
#include "assets.hpp"
#include "static.hpp"

static modloader::ModInfo modInfo = {MOD_ID, VERSION, 0};

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig()
{
    static Configuration config(modInfo);
    return config;
}

// Called at the early stages of game loading
extern "C" __attribute((visibility("default"))) void setup(CModInfo *info) noexcept
{
    *info = modInfo.to_c();

    getConfig().Load();
    getConfig().Reload();
    getConfig().Write();
    INFO("Completed setup!");
}

void soft_restart()
{
    SnoreSaber::ReplaySystem::ReplayStateRegistry::Current.Reset();
    SnoreSaber::ReplaySystem::Playback::ReplayPlaybackRegistry::Clear();
    SnoreSaber::Services::PlayerService::OnSoftRestart();
    SnoreSaber::Services::ReplayService::OnSoftRestart();
    SnoreSaber::UI::Other::SnoreSaberLeaderboardView::OnSoftRestart();
    SnoreSaber::UI::Other::ProfilePictureView::OnSoftRestart();
}

// Called later on in the game loading - a good time to install function hooks
extern "C" __attribute((visibility("default"))) void late_load() noexcept
{
    il2cpp_functions::Init();
    BSML::Init();
    custom_types::Register::AutoRegister();
    Hooks::InstallHooks();
    SnoreSaber::Core::SnoreSaberRuntimeInfo::InitializeGameVersion();
    SnoreSaber::Data::Private::Settings::LoadSettings();
    TeamUtils::Download();
    
    SnoreSaber::Services::FileService::EnsurePaths();

    auto zenjector = Lapiz::Zenject::Zenjector::Get();
    zenjector->Install<SnoreSaber::Core::AppInstaller*>(Lapiz::Zenject::Location::App);
    zenjector->Install<SnoreSaber::MainInstaller*>(Lapiz::Zenject::Location::Menu);
    zenjector->Install<SnoreSaber::ReplaySystem::Installers::PlaybackInstaller*>(Lapiz::Zenject::Location::StandardPlayer);
    zenjector->Install<SnoreSaber::ReplaySystem::Installers::ImberInstaller*>(Lapiz::Zenject::Location::StandardPlayer);
    zenjector->Install<SnoreSaber::ReplaySystem::Installers::RecordInstaller*, GlobalNamespace::StandardGameplayInstaller*>();
    zenjector->Install<SnoreSaber::ReplaySystem::Installers::RecordInstaller*, GlobalNamespace::MultiplayerLocalActivePlayerInstaller*>();
    zenjector->Install<SnoreSaber::Features::Live::LiveGameplayInstaller*, GlobalNamespace::StandardGameplayInstaller*>();
    zenjector->Install<SnoreSaber::Features::Live::LiveGameplayInstaller*, GlobalNamespace::MultiplayerLocalActivePlayerInstaller*>();

    BSML::Events::onGameDidRestart.addCallback(soft_restart);
}

BSML_DATACACHE(replay_png) {
    return IncludedAssets::replay_png;
}
