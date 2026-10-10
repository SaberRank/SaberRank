import { createFileRoute, linkOptions } from '@tanstack/react-router';
import { createServerFn } from '@tanstack/react-start';
import { z } from 'zod';

import { MapCard } from '@/modules/maps/listing/map-card';
import { SeasonalCountdown } from '@/modules/maps/listing/seasonal-countdown';
import { MapDownloadActions } from '@/modules/maps/listing/map-download-actions';
import { MapFilters } from '@/modules/maps/listing/map-filters';
import { isMapIdentifierSearch } from '@/modules/maps/shared/map-search';
import {
   MAP_CONTROLLER_GET_MAP_LISTINGS_SORT_BY,
   MAP_CONTROLLER_GET_MAP_LISTINGS_SORT_DIRECTION,
} from '@/shared/api/generated/ApiParams';
import type { MapControllerGetMapListingsResponse } from '@/shared/api/generated/ApiParams';
import { localApiPageData } from '@/shared/api/local-action.server';
import { PageError } from '@/shared/components/error/page-error';
import { Pagination } from '@/shared/components/pagination';
import { buildSeoHead } from '@/shared/seo/metadata';
import { isPageNumber } from '@/shared/url-state/params';
import { mapFilterPreferences } from '@/shared/url-state/persisted-filter-preferences';
import { applyPersistedSearchParams, readPersistedSearchStorage } from '@/shared/url-state/persisted-search';
import type { SearchParamsRecord } from '@/shared/url-state/search-params';
import { normalizeSearchRecord } from '@/shared/url-state/search-serializer';
import { updateSearchParams } from '@/shared/url-state/update-search-params';
import { SetPageBackground } from '@/shell/background/page-background-provider';

const isOptionalNumber = z.preprocess((val) => (val === '' ? undefined : val), z.coerce.number().min(0).optional());

const mapsSearchSchema = z.object({
   page: isPageNumber.optional(),
   search: z.string().min(1).max(64).optional(),
   status: z.string().optional(),
   verified: z.enum(['true', 'false']).optional(),
   minStars: isOptionalNumber,
   maxStars: isOptionalNumber,
   sortBy: z.enum(MAP_CONTROLLER_GET_MAP_LISTINGS_SORT_BY).optional(),
   sortDirection: z.enum(MAP_CONTROLLER_GET_MAP_LISTINGS_SORT_DIRECTION).optional()
});

type MapsSearchParams = z.output<typeof mapsSearchSchema>;

type MapsRouteInput = {
   search: MapsSearchParams;
   rawSearch: SearchParamsRecord;
};

const getMapsPageData = createServerFn({ method: 'GET' })
   .validator((data: MapsRouteInput) => data)
   .handler(async ({ data }) => {
      const rawSearchParams = normalizeSearchRecord(data.rawSearch);
      const effectiveSearchParams = await applyPersistedSearchParams<MapsSearchParams>({
         searchParams: rawSearchParams,
         parseSearch: parseMapsSearch,
         storageKey: mapFilterPreferences.storageKey,
         persistedKeys: mapFilterPreferences.persistedKeys
      });
      const searchParams = mapsSearchSchema.parse({ ...data.search, ...effectiveSearchParams });
      const persistedStorage = await readPersistedSearchStorage(mapFilterPreferences.storageKey);
      const search = searchParams.search?.trim();
      const identifierSearch = search ? isMapIdentifierSearch(search) : false;
      const apiSearch = new URLSearchParams({
         page: String(searchParams.page ?? 1),
         sortBy: searchParams.sortBy ?? 'trending',
         sortDirection: searchParams.sortDirection ?? 'desc'
      });
      if (search) apiSearch.set('search', search);
      if (!identifierSearch) {
         apiSearch.set('status', 'RANKED');
         if (searchParams.verified) apiSearch.set('verified', searchParams.verified);
         if (searchParams.minStars != null) apiSearch.set('minStars', String(searchParams.minStars));
         if (searchParams.maxStars != null) apiSearch.set('maxStars', String(searchParams.maxStars));
      }
      const result = await localApiPageData<MapControllerGetMapListingsResponse>(`/maps?${apiSearch.toString()}`);

      return { result, searchParams, persistedStorage };
   });

export const Route = createFileRoute('/maps')({
   validateSearch: (search) => mapsSearchSchema.parse(search),
   loaderDeps: ({ search }) => search,
   loader: ({ deps, location }) => getMapsPageData({ data: { search: deps, rawSearch: location.search } }),
   head: () =>
      buildSeoHead({
         title: 'Seasonal Maps',
         description: 'Browse the current SnoreSaber seasonal ranked map pool',
         path: '/maps'
      }),
   component: MapsRoute
});

function MapsRoute() {
   const data = Route.useLoaderData();
   const { result, searchParams, persistedStorage } = data;

   if (!result.ok) return <PageError status={result.status} />;

   const response = result.data;
   const maps = response.data;
   const meta = response.metadata;
   const expandLowest = searchParams.sortBy === 'highestStars' && (searchParams.sortDirection ?? 'desc') === 'asc';
   const currentPage = searchParams.page ?? 1;
   const bgCandidates = maps.filter((m) => m.coverUrl).map((m) => m.coverUrl);
   const getPageLocation = (page: number) => buildMapsLocation(updateSearchParams(searchParams, { page: page > 1 ? page : undefined }));

   return (
      <div className="relative flex-1 overflow-hidden">
         {bgCandidates.length > 0 && <SetPageBackground src={bgCandidates[0]} candidates={bgCandidates} />}

         <div className="app-container relative z-10 flex flex-col gap-4 p-4 md:p-8">
            <div className="flex flex-col gap-3 sm:flex-row sm:items-center sm:justify-between"><div><h1 className="text-2xl font-semibold">Seasonal Maps</h1><p className="text-muted-foreground text-sm">Ranked maps for the current calendar-quarter season.</p></div><SeasonalCountdown /></div>
            <MapFilters
               currentPage={currentPage}
               totalPages={meta.totalPages}
               search={searchParams}
               buildLocation={buildMapsLocation}
               parseSearch={parseMapsSearch}
               initialFiltersOpen={persistedStorage.filtersOpen === 'true'}
               trailingAction={<MapDownloadActions search={searchParams} />}
            />

            <div className="grid grid-cols-1 gap-2 md:grid-cols-2">
               {maps.map((map, index) => (
                  <MapCard key={map.id} map={map} expandLowest={expandLowest} coverPriority={index === 0} />
               ))}
            </div>

            {meta.totalPages > 1 && (
               <div className="flex justify-center">
                  <Pagination
                     currentPage={currentPage}
                     totalItems={meta.totalItems}
                     pageSize={meta.itemsPerPage}
                     getPageLocation={getPageLocation}
                     scroll={false}
                  />
               </div>
            )}
         </div>
      </div>
   );
}

function buildMapsLocation(search?: MapsSearchParams) {
   return linkOptions({ to: '/maps', search: normalizeMapsLocationSearch(search) });
}

function normalizeMapsLocationSearch(search?: MapsSearchParams) {
   const { page = 1, verified = 'true', ...rest } = search ?? {};
   return {
      page: page > 1 ? page : undefined,
      verified: verified === 'false' ? verified : undefined,
      ...rest
   };
}

function parseMapsSearch(search: SearchParamsRecord) {
   return mapsSearchSchema.safeParse({ page: 1, ...search }).data ?? null;
}

function parseMapListingStatuses(status?: string) {
   return mapStatusListSchema.parse(status?.split(',').filter(Boolean) ?? []);
}
