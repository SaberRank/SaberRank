#include "Features/Live/Ludus/Services/LiveChatSongNavigator.hpp"

#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <GlobalNamespace/LevelCollectionNavigationController.hpp>
#include <GlobalNamespace/LevelCollectionViewController.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Resources.hpp>

#include <future>
#include <vector>

using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::Features::Live::Ludus::Services, LiveChatSongNavigator);

namespace SnoreSaber::Features::Live::Ludus::Services
{
    namespace
    {
        template <typename T>
        std::vector<T> Active()
        {
            std::vector<T> active;
            for (auto item : UnityEngine::Resources::FindObjectsOfTypeAll<T>())
            {
                if (item && item->get_gameObject() && item->get_gameObject()->get_activeInHierarchy())
                {
                    active.push_back(item);
                }
            }

            return active;
        }

        bool TryFocusSongOnMainThread(const Compete::Domain::CompeteSongSelection& selection)
        {
            bool focused = false;
            BeatmapLevel* beatmapLevel = selection.beatmapLevel.ptr();

            for (auto controller : Active<LevelCollectionNavigationController*>())
            {
                controller->SelectLevel(beatmapLevel);
                focused = true;
            }

            for (auto controller : Active<LevelCollectionViewController*>())
            {
                controller->SelectLevel(beatmapLevel);
                focused = true;
            }

            if (focused)
            {
                INFO("Live chat selected linked map: {:s}", selection.name.c_str());
            }

            return focused;
        }
    }

    void LiveChatSongNavigator::ctor()
    {
        INVOKE_CTOR();
    }

    bool LiveChatSongNavigator::TryFocusSong(const std::shared_ptr<Compete::Domain::CompeteSongSelection>& selection, const CancellationToken& cancellationToken)
    {
        if (!selection || !selection->beatmapLevel)
        {
            return false;
        }

        // pc runs the focus on the unity scheduler and awaits it
        auto done = std::make_shared<std::promise<bool>>();
        Utils::Async::Main([selection, done, cancellationToken] {
            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                done->set_value(TryFocusSongOnMainThread(*selection));
            }
            catch (...)
            {
                done->set_exception(std::current_exception());
            }
        });

        return done->get_future().get();
    }
}
