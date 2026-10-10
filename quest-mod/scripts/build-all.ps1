[CmdletBinding()]
param(
    [string]$Version = "3.4.1",
    [string]$OutputDirectory = "./dist",
    [switch]$SkipRestore,
    [switch]$KeepBuildFiles
)

$ErrorActionPreference = 'Stop'

# Windows PowerShell 5.1's `Set-Content -Encoding utf8` emits a UTF-8 BOM.
# QPM's Rust JSON parser rejects that BOM, so write JSON/config files as UTF-8 without BOM.
$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
function Write-Utf8NoBom([string]$Path, [string]$Content) {
    [System.IO.File]::WriteAllText($Path, $Content, $Utf8NoBom)
}

# Use PowerShell 7 if available; otherwise support the Windows PowerShell 5.1
# that ships with Windows. This prevents the build from failing just because
# the optional `pwsh` command is not installed.
$PowerShellExe = (Get-Command pwsh -ErrorAction SilentlyContinue | Select-Object -First 1).Source
if ([string]::IsNullOrWhiteSpace($PowerShellExe)) {
    $PowerShellExe = (Get-Command powershell.exe -ErrorAction SilentlyContinue | Select-Object -First 1).Source
}
if ([string]::IsNullOrWhiteSpace($PowerShellExe)) {
    throw 'PowerShell was not found. Install PowerShell 7 from https://aka.ms/powershell or enable Windows PowerShell.'
}
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Push-Location $root

$qpmPath = Join-Path $root 'qpm.json'
$qpmSharedPath = Join-Path $root 'qpm.shared.json'
$templatePath = Join-Path $root 'mod.template.json'
$originalQpm = Get-Content $qpmPath -Raw
$originalQpmShared = if (Test-Path $qpmSharedPath) { Get-Content $qpmSharedPath -Raw } else { $null }
$originalTemplate = Get-Content $templatePath -Raw
# QPM executes workspace script strings itself. Rewrite any remaining `pwsh` calls
# for this Windows run, otherwise QPM fails even though this wrapper supports powershell.exe.
foreach ($configPath in @($qpmPath, $qpmSharedPath)) {
    if (Test-Path $configPath) {
        $configText = Get-Content $configPath -Raw
        $configText = $configText -replace 'pwsh\s+(\./scripts/[^"\r\n]+)', 'powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File $1'
        Write-Utf8NoBom -Path $configPath -Content $configText
    }
}
$outPath = [System.IO.Path]::GetFullPath((Join-Path $root $OutputDirectory))
New-Item -ItemType Directory -Force -Path $outPath | Out-Null

# These are the requested Quest game targets, not PC game versions.
$targets = @(
    @{ GameVersion = '1.29.0'; CordlVersion = '2900.0.0'; PackageVersion = '1.29.0' },
    @{ GameVersion = '1.37.0'; CordlVersion = '3700.0.0'; PackageVersion = '1.37.0_9064817954' },
    @{ GameVersion = '1.38.0'; CordlVersion = '3800.0.0'; PackageVersion = '1.38.0' },
    @{ GameVersion = '1.40.8'; CordlVersion = '4008.0.0'; PackageVersion = '1.40.8_7379' },
    @{ GameVersion = '1.42.0'; CordlVersion = '4200.0.0'; PackageVersion = '1.42.0' }
)

function Invoke-Checked([string]$Command, [string[]]$Arguments) {
    Write-Host "`n> $Command $($Arguments -join ' ')" -ForegroundColor Cyan
    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Command failed with exit code $LASTEXITCODE`: $Command $($Arguments -join ' ')" }
}

$results = @()
try {
    foreach ($target in $targets) {
        Write-Host "`n============================================================" -ForegroundColor Yellow
        Write-Host "Building SnoreSaber Quest for Beat Saber $($target.GameVersion)" -ForegroundColor Yellow
        Write-Host "============================================================" -ForegroundColor Yellow

        $qpm = Get-Content $qpmPath -Raw | ConvertFrom-Json
        $cordl = @($qpm.dependencies | Where-Object { $_.id -eq 'bs-cordl' }) | Select-Object -First 1
        if (-not $cordl) { throw 'qpm.json does not contain a bs-cordl dependency.' }
        $cordl.versionRange = '^' + $target.CordlVersion
        Write-Utf8NoBom -Path $qpmPath -Content ($qpm | ConvertTo-Json -Depth 100)

        $template = Get-Content $templatePath -Raw | ConvertFrom-Json
        $template.packageVersion = $target.PackageVersion
        Write-Utf8NoBom -Path $templatePath -Content ($template | ConvertTo-Json -Depth 100)

        if (-not $SkipRestore) {
            Invoke-Checked 'qpm' @('restore')
        }

        # Force a clean CMake configure for each game's bindings.
        Invoke-Checked $PowerShellExe @('-NoLogo', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $root 'scripts/build.ps1'), '-clean', '-configure')
        Invoke-Checked 'qpm' @('qmod', 'manifest')
        Invoke-Checked $PowerShellExe @('-NoLogo', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $root 'scripts/createqmod.ps1'), 'SnoreSaber')

        $builtQmod = Join-Path $root 'SnoreSaber.qmod'
        if (-not (Test-Path $builtQmod)) { throw "Expected package was not created: $builtQmod" }
        $destination = Join-Path $outPath ("SnoreSaber-Quest-{0}-v{1}.qmod" -f $target.GameVersion, $Version)
        Copy-Item $builtQmod $destination -Force
        $results += [pscustomobject]@{ GameVersion = $target.GameVersion; PackageVersion = $target.PackageVersion; File = $destination; Status = 'Built' }
        Write-Host "SUCCESS: $destination" -ForegroundColor Green

        if (-not $KeepBuildFiles) {
            if (Test-Path (Join-Path $root 'build')) { Remove-Item (Join-Path $root 'build') -Recurse -Force }
            if (Test-Path $builtQmod) { Remove-Item $builtQmod -Force }
            if (Test-Path (Join-Path $root 'SnoreSaber.zip')) { Remove-Item (Join-Path $root 'SnoreSaber.zip') -Force }
        }
    }

    $manifest = [pscustomobject]@{
        pluginVersion = $Version
        generatedAt = (Get-Date).ToString('o')
        note = 'A successful build is not the same as runtime validation on a headset.'
        builds = $results
    }
    $manifest | ConvertTo-Json -Depth 10 | Set-Content (Join-Path $outPath 'build-summary.json') -Encoding utf8
    Write-Host "`nAll requested build jobs completed." -ForegroundColor Green
    $results | Format-Table GameVersion, PackageVersion, Status, File -AutoSize
}
catch {
    Write-Error $_
    exit 1
}
finally {
    # Restore the user's original project config even if a target fails.
    Write-Utf8NoBom -Path $qpmPath -Content $originalQpm
    Write-Utf8NoBom -Path $templatePath -Content $originalTemplate
    Pop-Location
}
