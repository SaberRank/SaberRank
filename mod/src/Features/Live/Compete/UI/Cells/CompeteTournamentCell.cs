using SnoreSaber.Features.Live.Compete.Domain;

namespace SnoreSaber.Features.Live.Compete.UI.Cells {
    internal class CompeteTournamentCell : CompeteListRowCell {
        internal CompeteTournament Tournament { get; }

        internal CompeteTournamentCell(CompeteTournament tournament)
            : base(tournament.Name, tournament.RoomSummary, string.Empty) {
            Tournament = tournament;
        }
    }
}
