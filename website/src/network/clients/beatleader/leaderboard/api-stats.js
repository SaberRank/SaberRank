import queue from '../../../queues/queues';
import createClient from '../../generic';

const process = response => response;

const get = async ({leaderboardId, priority = queue.PRIORITY.FG_HIGH, ...queueOptions} = {}) =>
	queue.SABERRANK_API.leaderboardStats(leaderboardId, priority, queueOptions);

const client = createClient(get, process);

export default client;
