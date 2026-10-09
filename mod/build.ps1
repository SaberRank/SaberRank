param(
    [ValidateSet('All','1.29.0','1.37.1','1.38.0','1.40.0','1.42.0')]
    [string]$TargetVersion = '1.40.0',
    [string]$BeatSaberRoot = ''
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Solution = Join-Path $Root 'SnoreSaber.sln'

$targets = @('1.29.0','1.37.1','1.38.0','1.40.0','1.42.0')
if ($TargetVersion -ne 'All') { $targets = @($TargetVersion) }

if ([string]::IsNullOrWhiteSpace($BeatSaberRoot)) {
    $BeatSaberRoot = Join-Path $env:USERPROFILE 'BSManager\BSInstances'
}

function Get-GameDirectory([string]$base) {
    $mapping = @{
        '1.29.0' = '^1\.29\.[01]($|\s|_)'
        '1.37.1' = '^1\.37\.[12]($|\s|_)'
        '1.38.0' = '^1\.3(8\.0|9\.[01])($|\s|_)'
        '1.40.0' = '^1\.40\.[0-8]($|\s|_)'
        '1.42.0' = '^1\.(42\.[0-3]|43\.0|44\.[01])($|\s|_)'
    }
    if (-not (Test-Path $BeatSaberRoot)) { return $null }
    $dirs = Get-ChildItem -LiteralPath $BeatSaberRoot -Directory -ErrorAction SilentlyContinue
    foreach ($d in $dirs) {
        $name = $d.Name
        $candidate = $name
        if ($name -match '^([^_ ]+)_') { $candidate = $Matches[1] }
        if ($candidate -match $mapping[$base]) {
            if ((Test-Path (Join-Path $d.FullName 'Beat Saber_Data\Managed')) -and (Test-Path (Join-Path $d.FullName 'IPA'))) {
                return $d.FullName
            }
        }
    }
    return $null
}

function Get-RefsDirectory([string]$base) {
    foreach ($p in @(
        (Join-Path $Root "refs\$base"),
        (Join-Path $Root "Refs\$base")
    )) {
        if ((Test-Path (Join-Path $p 'Beat Saber_Data\Managed')) -and (Test-Path (Join-Path $p 'Plugins\LeaderboardCore.dll'))) {
            return $p
        }
    }
    return $null
}

foreach ($base in $targets) {
    $game = Get-GameDirectory $base
    $refs = Get-RefsDirectory $base
    $source = if ($game) { $game } elseif ($refs) { $refs } else { $null }

    if (-not $source) {
        Write-Warning "No Beat Saber installation or stripped refs found for target $base. Install a compatible version or place refs in refs\$base, then rerun."
        continue
    }

    Write-Host "=== Building Beat Saber target $base ===" -ForegroundColor Cyan
    Write-Host "References: $source"

    # The protobuf-net mod is normally installed under Plugins; older/manual
    # reference layouts may keep it under Libs. The project now supports both.
    $protobufPlugins = Join-Path $source 'Plugins\protobuf-net.dll'
    $protobufLibs = Join-Path $source 'Libs\protobuf-net.dll'
    if (Test-Path $protobufPlugins) {
        Write-Host "protobuf-net: $protobufPlugins"
    } elseif (Test-Path $protobufLibs) {
        Write-Host "protobuf-net: $protobufLibs"
    } else {
        Write-Warning "protobuf-net.dll was not found under Plugins or Libs for $base"
    }

    # Build with dotnet MSBuild directly. Use interpolation so PowerShell cannot
    # split the target version/property values into positional MSBuild arguments.
    $msbuildArgs = @(
        'msbuild',
        $Solution,
        '/t:Build',
        '/p:Configuration=Release',
        "/p:Platform=BS$base",
        "/p:SnoreSaberTargetVersion=$base",
        '/p:SnoreSaberUseLocalRefs=true',
        "/p:LocalRefsDir=$source",
        "/p:GameReferences=$source",
        '/p:DisableCopyToGame=True',
        '/p:DisableZipRelease=False'
    )


    & dotnet @msbuildArgs
    if ($LASTEXITCODE -ne 0) { throw "Build failed for Beat Saber target $base (exit code $LASTEXITCODE)." }
}
