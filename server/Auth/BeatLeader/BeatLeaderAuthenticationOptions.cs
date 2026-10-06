/*
 * Licensed under the Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
 * See https://github.com/aspnet-contrib/AspNet.Security.OAuth.Providers
 * for more information concerning the license and the contributors participating to this project.
 */

using SaberRank_Server.Models;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Authentication.OAuth;
using System.Security.Claims;
using static AspNet.Security.OAuth.SaberRank.SaberRankAuthenticationConstants;

namespace AspNet.Security.OAuth.SaberRank;

/// <summary>
/// Defines a set of options used by <see cref="SaberRankAuthenticationHandler"/>.
/// </summary>
public class SaberRankAuthenticationOptions : OAuthOptions
{
    public SaberRankAuthenticationOptions()
    {
        ClaimsIssuer = SaberRankAuthenticationDefaults.Issuer;
        CallbackPath = SaberRankAuthenticationDefaults.CallbackPath;

        AuthorizationEndpoint = SaberRankAuthenticationDefaults.AuthorizationEndpoint;
        TokenEndpoint = SaberRankAuthenticationDefaults.TokenEndpoint;
        UserInformationEndpoint = SaberRankAuthenticationDefaults.UserInformationEndpoint;

        Scope.Add("profile");
        Scope.Add(CustomScopes.Clan);

        ClaimActions.MapJsonKey(ClaimTypes.NameIdentifier, "id");
        ClaimActions.MapJsonKey(ClaimTypes.Name, "name");
    }

    /// <summary>
    /// Gets the list of fields to retrieve from the user information endpoint.
    /// </summary>
    public ISet<string> Fields { get; } = new HashSet<string>();

    /// <summary>
    /// Gets the list of related data to include from the user information endpoint.
    /// </summary>
    public ISet<string> Includes { get; } = new HashSet<string>();
}
