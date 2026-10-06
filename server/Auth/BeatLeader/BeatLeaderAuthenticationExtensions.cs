/*
 * Licensed under the Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
 * See https://github.com/aspnet-contrib/AspNet.Security.OAuth.Providers
 * for more information concerning the license and the contributors participating to this project.
 */

using AspNet.Security.OAuth.SaberRank;
using Microsoft.AspNetCore.Authentication;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.Extensions.DependencyInjection;

/// <summary>
/// Extension methods to add Patreon authentication capabilities to an HTTP application pipeline.
/// </summary>
public static class SaberRankAuthenticationExtensions
{
    /// <summary>
    /// Adds <see cref="SaberRankAuthenticationHandler"/> to the specified
    /// <see cref="AuthenticationBuilder"/>, which enables Patreon authentication capabilities.
    /// </summary>
    /// <param name="builder">The authentication builder.</param>
    /// <returns>The <see cref="AuthenticationBuilder"/>.</returns>
    public static AuthenticationBuilder AddSaberRank([NotNull] this AuthenticationBuilder builder)
    {
        return builder.AddSaberRank(SaberRankAuthenticationDefaults.AuthenticationScheme, options => { });
    }

    /// <summary>
    /// Adds <see cref="SaberRankAuthenticationHandler"/> to the specified
    /// <see cref="AuthenticationBuilder"/>, which enables Patreon authentication capabilities.
    /// </summary>
    /// <param name="builder">The authentication builder.</param>
    /// <param name="configuration">The delegate used to configure the OpenID 2.0 options.</param>
    /// <returns>The <see cref="AuthenticationBuilder"/>.</returns>
    public static AuthenticationBuilder AddSaberRank(
        [NotNull] this AuthenticationBuilder builder,
        [NotNull] Action<SaberRankAuthenticationOptions> configuration)
    {
        return builder.AddSaberRank(SaberRankAuthenticationDefaults.AuthenticationScheme, configuration);
    }

    /// <summary>
    /// Adds <see cref="SaberRankAuthenticationHandler"/> to the specified
    /// <see cref="AuthenticationBuilder"/>, which enables Patreon authentication capabilities.
    /// </summary>
    /// <param name="builder">The authentication builder.</param>
    /// <param name="scheme">The authentication scheme associated with this instance.</param>
    /// <param name="configuration">The delegate used to configure the Patreon options.</param>
    /// <returns>The <see cref="AuthenticationBuilder"/>.</returns>
    public static AuthenticationBuilder AddSaberRank(
        [NotNull] this AuthenticationBuilder builder,
        [NotNull] string scheme,
        [NotNull] Action<SaberRankAuthenticationOptions> configuration)
    {
        return builder.AddSaberRank(scheme, SaberRankAuthenticationDefaults.DisplayName, configuration);
    }

    /// <summary>
    /// Adds <see cref="SaberRankAuthenticationHandler"/> to the specified
    /// <see cref="AuthenticationBuilder"/>, which enables Patreon authentication capabilities.
    /// </summary>
    /// <param name="builder">The authentication builder.</param>
    /// <param name="scheme">The authentication scheme associated with this instance.</param>
    /// <param name="caption">The optional display name associated with this instance.</param>
    /// <param name="configuration">The delegate used to configure the Patreon options.</param>
    /// <returns>The <see cref="AuthenticationBuilder"/>.</returns>
    public static AuthenticationBuilder AddSaberRank(
        [NotNull] this AuthenticationBuilder builder,
        [NotNull] string scheme,
        [MaybeNull] string caption,
        [NotNull] Action<SaberRankAuthenticationOptions> configuration)
    {
        return builder.AddOAuth<SaberRankAuthenticationOptions, SaberRankAuthenticationHandler>(scheme, caption, configuration);
    }
}
