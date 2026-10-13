using RatingAPI;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddControllers();
builder.Services.AddCors(options =>
{
    options.AddDefaultPolicy(policy => policy.AllowAnyOrigin().AllowAnyMethod().AllowAnyHeader());
});

var app = builder.Build();

// The RatingAPI is a public HTTP service. Protect the endpoint used by SnoreSaber
// with a shared secret so third parties cannot freely consume the hosted analyzer.
app.Use(async (context, next) =>
{
    if (context.Request.Path.StartsWithSegments("/ppai2/link"))
    {
        var configuredKey = app.Configuration["AutorankerApiKey"];
        if (string.IsNullOrWhiteSpace(configuredKey))
        {
            context.Response.StatusCode = StatusCodes.Status503ServiceUnavailable;
            await context.Response.WriteAsJsonAsync(new { error = "AutorankerApiKey is not configured on this service." });
            return;
        }

        var suppliedKey = context.Request.Headers["X-SnoreSaber-Autoranker-Key"].ToString();
        if (!System.Security.Cryptography.CryptographicOperations.FixedTimeEquals(
                System.Text.Encoding.UTF8.GetBytes(configuredKey),
                System.Text.Encoding.UTF8.GetBytes(suppliedKey.Length == configuredKey.Length ? suppliedKey : new string('\0', configuredKey.Length))))
        {
            context.Response.StatusCode = StatusCodes.Status401Unauthorized;
            await context.Response.WriteAsJsonAsync(new { error = "Invalid autoranker service key." });
            return;
        }
    }

    await next();
});

app.UseMiddleware<ErrorLoggingMiddleware>();
app.UseHttpsRedirection();
app.UseCors();
app.UseAuthorization();
app.MapControllers();
app.Run();
