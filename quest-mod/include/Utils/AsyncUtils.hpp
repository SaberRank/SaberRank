#pragma once

#include "Utils/GCUtil.hpp"

#include <beatsaber-hook/shared/utils/il2cpp-utils.hpp>
#include <bsml/shared/BSML/MainThreadScheduler.hpp>

#include <functional>
#include <thread>
#include <type_traits>
#include <utility>

namespace SnoreSaber::Utils::Async
{
    template <typename F>
    void Run(F&& work)
    {
        il2cpp_utils::il2cpp_aware_thread(gc_aware_function(std::forward<F>(work))).detach();
    }

    template <typename F>
    void RunCpp(F&& work)
    {
        std::thread(std::forward<F>(work)).detach();
    }

    template <typename F>
    void Main(F&& work)
    {
        BSML::MainThreadScheduler::Schedule(gc_aware_function(std::forward<F>(work)));
    }

    template <typename F>
    void After(float seconds, F&& work)
    {
        BSML::MainThreadScheduler::ScheduleAfterTime(seconds, gc_aware_function(std::forward<F>(work)));
    }

    template <typename Work, typename Callback>
    void RunThenMain(Work&& work, Callback&& callback)
    {
        using WorkT = std::decay_t<Work>;
        using CallbackT = std::decay_t<Callback>;
        using ResultT = std::invoke_result_t<WorkT&>;

        Run([work = WorkT(std::forward<Work>(work)), callback = CallbackT(std::forward<Callback>(callback))] {
            if constexpr (std::is_void_v<ResultT>)
            {
                std::invoke(work);
                Main([callback] {
                    std::invoke(callback);
                });
            }
            else
            {
                auto result = std::invoke(work);
                Main([callback, result] {
                    std::invoke(callback, result);
                });
            }
        });
    }
}
