using Amazon.S3;
using saberrank_parser;
using SaberRank_Server.Enums;
using SaberRank_Server.Extensions;
using SaberRank_Server.Models;
using SaberRank_Server.Services;
using SaberRank_Server.Utils;
using Lib.ServerTiming;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using ReplayDecoder;
using Swashbuckle.AspNetCore.Annotations;
using System.Buffers;
using Parser.Utils;
using System.IO.Compression;
using static SaberRank_Server.Utils.ResponseUtils;
using Microsoft.AspNetCore.StaticFiles;

namespace SaberRank_Server.Controllers
{
    public class ReplaysProxyController : Controller
    {
        private readonly AppContext _context;
        private readonly StorageContext _storageContext;

        private readonly IServerTiming _serverTiming;
        private readonly IConfiguration _configuration;
        private readonly IAmazonS3 _s3Client;

        public ReplaysProxyController(
            AppContext context,
            StorageContext storageContext,
            IWebHostEnvironment env,
            IServerTiming serverTiming,
            IConfiguration configuration)
        {
            _context = context;
            _storageContext = storageContext;

            _serverTiming = serverTiming;
            _configuration = configuration;
            _s3Client = configuration.GetS3Client();
        }

        

        [HttpGet("~/otherreplays/{name}")]
        public async Task<ActionResult> GetOtherReplay(string name) {

            string? currentID = HttpContext.CurrentUserID(_context);
            bool admin = currentID != null ? ((await _context
                .Players
                .Where(p => p.Id == currentID)
                .Select(p => p.Role)
                .FirstOrDefaultAsync())
                ?.Contains("admin") ?? false) : false;

            var replayFile = $"https://api.saberrank.com/otherreplays/{name}";
            var stats = await _storageContext.PlayerLeaderboardStats.Where(p => p.Replay == replayFile)
                .AsNoTracking()
                .TagWithCaller()
                .Select(s => new { PinnedContexts = s.Metadata != null ? s.Metadata.PinnedContexts : LeaderboardContexts.None, s.PlayerId })
                .ToListAsync();

            if (stats.Count == 0) return NotFound();
            
            if (!stats.Any(s => s.PinnedContexts != LeaderboardContexts.None)) {
                string? playerId = await _context.PlayerIdToMain(stats.First().PlayerId);

                if (playerId != currentID) {
                    var features = await _context
                        .Players
                        .Where(p => p.Id == playerId)
                        .Select(p => p.ProfileSettings)
                        .FirstOrDefaultAsync();

                    if (!(currentID == playerId || admin || (features != null && features.ShowStatsPublic))) {
                        return Unauthorized();
                    }
                }
            }

            var stream = await _s3Client.DownloadOtherReplay(name);
            if (stream == null) {
                return NotFound();
            }

            return Ok(stream);
        }

        [HttpGet("~/replays-storage/{name}")]
        public ActionResult GetColdReplay(string name) {

            var root = Path.GetFullPath(ReplaysColdStorage.ColdStorageRoot);
            var contentTypes = new FileExtensionContentTypeProvider();

            var fullPath = Path.GetFullPath(Path.Combine(root, name));

            if (!fullPath.StartsWith(root + Path.DirectorySeparatorChar, StringComparison.Ordinal))
                return BadRequest();

            if (!System.IO.File.Exists(fullPath))
                return NotFound();

            if (!contentTypes.TryGetContentType(fullPath, out var contentType))
                contentType = "application/octet-stream";

            HttpContext.Response.Headers.CacheControl = "public, max-age=86400";

            return PhysicalFile(fullPath, contentType, enableRangeProcessing: true);
        }
    }
}
