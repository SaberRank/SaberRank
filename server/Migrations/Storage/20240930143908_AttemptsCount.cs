using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace SaberRank_Server.Migrations.Storage
{
    /// <inheritdoc />
    public partial class AttemptsCount : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<int>(
                name: "AttemptsCount",
                table: "PlayerLeaderboardStats",
                type: "int",
                nullable: false,
                defaultValue: 0);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "AttemptsCount",
                table: "PlayerLeaderboardStats");
        }
    }
}
