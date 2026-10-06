import queue from '../../../queues/queues';
import process from './utils/process';
import createClient from '../../generic';

const get = async ({page = 1, eventId = 1, filters, priority = queue.PRIORITY.FG_HIGH, ...queueOptions} = {}) =>
	queue.SABERRANK_API.rankingEventFollowed(page, eventId, filters, priority, queueOptions);

const client = createClient(get, process);

export default client;
