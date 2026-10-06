using Microsoft.AspNetCore.Mvc;

namespace SaberRank_Server.Controllers {
    public class MiscController : Controller {
        [HttpGet("~/")]
        public async Task<ActionResult> Index() {
            return Redirect("/swagger/index.html");
        }
    }
}
