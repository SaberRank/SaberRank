'use client';

import { type ReactNode } from 'react';

import { FaArrowDown, FaArrowUp } from 'react-icons/fa';
import { useTranslations } from 'use-intl';

import { usePersistedParams } from '@/shared/url-state/persisted/use-persisted-params';
import { DebouncedSearchInput } from '@/shared/components/debounced-search-input';
import { PaginationArrow } from '@/shared/components/pagination';
import { FilterPill } from '@/shared/components/filter-pill';
import { isMapSearchReady } from '@/modules/maps/shared/map-search';
import {
   MAP_CONTROLLER_GET_MAP_LISTINGS_SORT_BY,
   type MapControllerGetMapListingsSortBy,
   type MapControllerGetMapListingsSortDirection
} from '@/shared/api/generated/ApiParams';
import type { RouteLocationBuilder } from '@/shared/url-state/route-location';
import type { SearchParamsRecord } from '@/shared/url-state/search-params';
import { updateSearchParams } from '@/shared/url-state/update-search-params';
import { mapFilterPreferences } from '@/shared/url-state/persisted-filter-preferences';

const SORT_OPTIONS: { value: MapControllerGetMapListingsSortBy }[] = [
   { value: 'trending' },
   { value: 'createdAt' },
   { value: 'latestRankedAt' },
   { value: 'highestStars' },
   { value: 'totalScores' }
];

interface MapFiltersProps<TLocation> {
   currentPage: number;
   totalPages: number;
   search: MapsFilterSearch;
   buildLocation: RouteLocationBuilder<MapsFilterSearch, TLocation>;
   parseSearch: (search: SearchParamsRecord) => MapsFilterSearch | null;
   initialFiltersOpen: boolean;
   trailingAction?: ReactNode;
}

type MapsFilterSearch = SearchParamsRecord & {
   page?: number;
   search?: string;
   status?: string;
   verified?: 'true' | 'false';
   minStars?: number;
   maxStars?: number;
   sortBy?: MapControllerGetMapListingsSortBy;
   sortDirection?: MapControllerGetMapListingsSortDirection;
};

export function MapFilters<TLocation>({
   currentPage,
   totalPages,
   search,
   buildLocation,
   parseSearch,
   trailingAction
}: MapFiltersProps<TLocation>) {
   const t = useTranslations();
   const { navigate } = usePersistedParams({
      storageKey: mapFilterPreferences.storageKey,
      search,
      buildLocation,
      parseSearch,
      persistedKeys: []
   });

   const currentSearch = search.search;
   const currentSortBy = search.sortBy ?? 'trending';
   const currentSortDirection = search.sortDirection ?? 'desc';
   const showPagination = totalPages > 1;

   const getPageLocation = (page: number) =>
      buildLocation(updateSearchParams(search, { page: page > 1 ? page : undefined }));

   function handleSortChange(sortBy: MapControllerGetMapListingsSortBy) {
      navigate({
         sortBy,
         sortDirection: sortBy === currentSortBy && currentSortDirection === 'desc' ? 'asc' : 'desc',
         status: 'RANKED'
      });
   }

   return (
      <div className="flex flex-col gap-3">
         <div className="flex items-center gap-2 md:gap-3">
            {showPagination && (
               <PaginationArrow direction="left" page={currentPage - 1} disabled={currentPage <= 1} getPageLocation={getPageLocation} />
            )}

            <div className="min-w-0 flex-1">
               <DebouncedSearchInput
                  id="map-search"
                  initialValue={currentSearch ?? ''}
                  placeholder={t('map.searchPlaceholder')}
                  clearLabel={t('common.clearSearch')}
                  srLabel={t('map.searchMaps')}
                  isSearchReady={isMapSearchReady}
                  onSearchAction={(value) => navigate({ search: value || undefined, status: 'RANKED' })}
               />
            </div>

            {showPagination && (
               <PaginationArrow direction="right" page={currentPage + 1} disabled={currentPage >= totalPages} getPageLocation={getPageLocation} />
            )}
         </div>

         <div className="flex items-center justify-center gap-1.5 overflow-x-auto">
            {SORT_OPTIONS.map(({ value }) => {
               const active = currentSortBy === value;
               return (
                  <FilterPill
                     className="cursor-pointer"
                     key={value}
                     active={active}
                     onClick={() => handleSortChange(value)}
                  >
                     {value === 'trending'
                        ? t('map.sortTrending')
                        : value === 'createdAt'
                          ? t('map.sortDateAdded')
                          : value === 'latestRankedAt'
                            ? t('map.sortRecentlyRanked')
                            : value === 'highestStars'
                              ? t('map.sortStarRating')
                              : t('map.sortMostPlayed')}
                     {active && (currentSortDirection === 'desc' ? <FaArrowDown className="size-2.5" /> : <FaArrowUp className="size-2.5" />)}
                  </FilterPill>
               );
            })}
            {trailingAction}
         </div>

         <div className="text-center text-xs text-muted-foreground">
            Ranked maps only
         </div>
      </div>
   );
}
