#include "Features/Leaderboards/Services/MaxScoreCache.hpp"
#include <custom-types/shared/delegate.hpp>
#include <GlobalNamespace/IBeatmapLevelData.hpp>
#include <GlobalNamespace/ScoreModel.hpp>
#include <GlobalNamespace/LoadBeatmapLevelDataResult.hpp>
#include <GlobalNamespace/zzzz__BeatmapLevelDataVersion_def.hpp>
#include <GlobalNamespace/zzzz__BeatmapLevelsEntitlementModel_def.hpp>
#include <System/Threading/zzzz__CancellationToken_def.hpp>
#include "Utils/DelegateUtils.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/SafePtr.hpp"
#include "Utils/GCUtil.hpp"
#include "logging.hpp"

using namespace GlobalNamespace;
using namespace System::Threading;
using namespace std;

DEFINE_TYPE(SnoreSaber::Utils, MaxScoreCache);

namespace SnoreSaber::Utils {
    void MaxScoreCache::ctor(BeatmapDataLoader* beatmapDataLoader, BeatmapLevelsModel* beatmapLevelsModel, BeatmapLevelsEntitlementModel* beatmapLevelsEntitlementModel) {
        INVOKE_CTOR();

        _beatmapDataLoader = beatmapDataLoader;
        _beatmapLevelsModel = beatmapLevelsModel;
        _beatmapLevelsEntitlementModel = beatmapLevelsEntitlementModel;
    }

    void MaxScoreCache::GetMaxScore(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, const function<void(int)> &callback) {
        if (!beatmapLevel ||
            !beatmapKey.beatmapCharacteristic ||
            !beatmapKey.levelId ||
            !_beatmapDataLoader ||
            !_beatmapLevelsModel ||
            !_beatmapLevelsEntitlementModel) {
            ERROR("Unable to compute max score due to missing beatmap data");
            callback(0);
            return;
        }

        SafeValueType<BeatmapKey> beatmapKeySafe(beatmapKey);
        {
            std::lock_guard lock(cacheLock);
            auto cachedMaxScore = cache.find(beatmapKeySafe);
            if(cachedMaxScore != cache.end()) {
                callback(cachedMaxScore->second);
                return;
            }
        }

        SafePtr<MaxScoreCache> self(this);
        SafePtr<BeatmapLevel> beatmapLevelSafe(beatmapLevel);

        SnoreSaber::Utils::Async::Main([self, beatmapLevelSafe, beatmapKeySafe, callback] {
            if (!self)
            {
                callback(0);
                return;
            }

            DelegateHelper::ContinueWith(self->_beatmapLevelsEntitlementModel->GetLevelDataVersionAsync(beatmapKeySafe->levelId, CancellationToken::get_None()), std::function(gc_aware_function(
                [self, beatmapLevelSafe, beatmapKeySafe, callback](BeatmapLevelDataVersion beatmapLevelDataVersion) {
                    if (!self)
                    {
                        callback(0);
                        return;
                    }

                    DelegateHelper::ContinueWith(self->_beatmapLevelsModel->LoadBeatmapLevelDataAsync(beatmapKeySafe->levelId, beatmapLevelDataVersion, CancellationToken::get_None()), std::function(gc_aware_function(
                        [self, beatmapLevelSafe, beatmapKeySafe, callback, beatmapLevelDataVersion](LoadBeatmapLevelDataResult beatmapLevelDataResult) {
                            SnoreSaber::Utils::Async::Main([self, beatmapLevelSafe, beatmapKeySafe, callback, beatmapLevelDataVersion, beatmapLevelDataResult]() {
                                if (!self || !beatmapLevelDataResult.beatmapLevelData)
                                {
                                    ERROR("Unable to load beatmap level data for max score");
                                    callback(0);
                                    return;
                                }

                                DelegateHelper::ContinueWith(self->_beatmapDataLoader->LoadBeatmapDataAsync(
                                    beatmapLevelDataResult.beatmapLevelData, // ::GlobalNamespace::IBeatmapLevelData *beatmapLevelData,
                                    *beatmapKeySafe, // ::GlobalNamespace::BeatmapKey beatmapKey,
                                    beatmapLevelSafe->beatsPerMinute, // float_t startBpm,
                                    false, // bool loadingForDesignatedEnvironment,
                                    nullptr, // ::GlobalNamespace::IEnvironmentInfo *targetEnvironmentInfo
                                    nullptr, // ::GlobalNamespace::IEnvironmentInfo *originalEnvironmentInfo
                                    beatmapLevelDataVersion,// ::GlobalNamespace::BeatmapLevelDataVersion beatmapLevelDataVersion
                                    nullptr, // ::GlobalNamespace::GameplayModifiers *gameplayModifiers,
                                    nullptr, // ::GlobalNamespace::PlayerSpecificSettings *playerSpecificSettings,
                                    false// bool enableBeatmapDataCaching
                                ), std::function(gc_aware_function(
                                    [self, beatmapKeySafe, callback](IReadonlyBeatmapData *beatmapData) {
                                        if (!self || !beatmapData)
                                        {
                                            ERROR("Unable to load beatmap data for max score");
                                            callback(0);
                                            return;
                                        }

                                        int maxScore = ScoreModel::ComputeMaxMultipliedScoreForBeatmap(beatmapData);
                                        {
                                            std::lock_guard lock(self->cacheLock);
                                            self->cache[beatmapKeySafe] = maxScore;
                                        }
                                        callback(maxScore);
                                    }
                                )));
                            });
                        }
                    )));
                }
            )));
        });
    }
}
