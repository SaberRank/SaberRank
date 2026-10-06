using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace SaberRank_Server.Migrations
{
    /// <inheritdoc />
    public partial class FeaturedPlaylistDescription : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<string>(
                name: "Description",
                table: "FeaturedPlaylist",
                type: "nvarchar(max)",
                nullable: true);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "Description",
                table: "FeaturedPlaylist");
        }
    }
}
