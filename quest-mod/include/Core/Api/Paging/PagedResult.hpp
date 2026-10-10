#pragma once

#include <vector>

namespace SnoreSaber::Core::Api::Paging
{
    struct PageMetadata
    {
        int page = 0;
        int itemsPerPage = 0;
        int totalItems = 0;
        int totalPages = 0;
    };

    template <typename T>
    struct PagedResult
    {
        std::vector<T> items;
        PageMetadata metadata;
    };
}
