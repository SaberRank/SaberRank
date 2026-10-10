#include "Utils/NumberFormatter.hpp"

namespace SnoreSaber::Utils
{
    std::string FormatInteger(int64_t number)
    {
        bool negative = number < 0;
        uint64_t magnitude = negative ? static_cast<uint64_t>(-(number + 1)) + 1 : static_cast<uint64_t>(number);
        std::string value = std::to_string(magnitude);

        int insertIndex = static_cast<int>(value.length()) - 3;
        while (insertIndex > 0)
        {
            value.insert(insertIndex, ",");
            insertIndex -= 3;
        }

        return negative ? "-" + value : value;
    }
}
