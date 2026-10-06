import queue from '../../../queues/queues';
import createClient from '../../generic';
import {processClanRankings} from './utils/process';

const get = async ({leaderboardId, page = 1, priority = queue.PRIORITY.FG_HIGH, ...queueOptions} = {}) =>
	queue.SABERRANK_API.clanRanking(leaderboardId, page, priority, queueOptions);

const client = createClient(get, processClanRankings);

export default client;
