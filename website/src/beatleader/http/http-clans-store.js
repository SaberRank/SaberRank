import createApiClansProvider from './providers/api-clans';
import paggedStoreWithFilters from './http-store-pagged-with-filters';

export default (page = 1, filters = {}, initialState = null, initialStateType = 'initial', force = true) =>
	paggedStoreWithFilters(createApiClansProvider(), page, filters, initialState, initialStateType, force);
