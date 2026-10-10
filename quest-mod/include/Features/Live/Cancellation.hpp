#pragma once

#include <atomic>
#include <memory>
#include <stdexcept>
#include <vector>

// stand-in for System.Threading cancellation; the quest api client is blocking,
// so live services run on worker threads and poll these flags between steps
namespace SnoreSaber::Features::Live
{
    struct OperationCanceledException : std::runtime_error
    {
        OperationCanceledException() : std::runtime_error("the operation was canceled") {}
    };

    class CancellationToken
    {
      public:
        CancellationToken() = default;

        bool IsCancellationRequested() const
        {
            for (const auto& flag : _flags)
            {
                if (flag && flag->load())
                {
                    return true;
                }
            }
            return false;
        }

        void ThrowIfCancellationRequested() const
        {
            if (IsCancellationRequested())
            {
                throw OperationCanceledException();
            }
        }

        // canceled when either token is; mirrors CreateLinkedTokenSource
        CancellationToken Linked(const CancellationToken& other) const
        {
            CancellationToken token = *this;
            token._flags.insert(token._flags.end(), other._flags.begin(), other._flags.end());
            return token;
        }

        // identity compare (same sources), used to match a stored token
        bool operator==(const CancellationToken& other) const = default;

      private:
        friend class CancellationSource;
        std::vector<std::shared_ptr<std::atomic<bool>>> _flags;
    };

    class CancellationSource
    {
      public:
        CancellationSource() : _flag(std::make_shared<std::atomic<bool>>(false)) {}

        CancellationToken Token() const
        {
            CancellationToken token;
            token._flags.push_back(_flag);
            return token;
        }

        void Cancel()
        {
            _flag->store(true);
        }

        bool IsCancellationRequested() const
        {
            return _flag->load();
        }

      private:
        std::shared_ptr<std::atomic<bool>> _flag;
    };
}
