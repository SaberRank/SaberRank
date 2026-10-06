import createClanRankingService from '../../../services/saberrank/clan-ranking';
import queue from '../../../network/queues/queues';

let clanRankingService = null;

export default () => {
	if (!clanRankingService) clanRankingService = createClanRankingService();

	const getProcessed = async ({
		leaderboardId,
		clanRankingId,
		page = 1,
		priority = queue.PRIORITY.FG_HIGH,
		signal = null,
		force = false,
	} = {}) => clanRankingService.fetchClanRankingScores(leaderboardId, clanRankingId, page, priority, signal, force);

	return {
		getProcessed,
		getCached: getProcessed,
	};
};
