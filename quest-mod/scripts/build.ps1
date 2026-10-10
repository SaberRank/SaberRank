Param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean,

    [Parameter(Mandatory=$false)]
    [Switch] $configure,

    [Parameter(Mandatory=$false)]
    [Switch] $noCache,

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    Write-Output "`"Build`" - Copiles your mod into a `".so`" or a `".a`" library"
    Write-Output "`n-- Arguments --`n"

    Write-Output "-Clean `t`t Deletes the `"build`" folder, so that the entire library is rebuilt"
    Write-Output "-Configure `t Forces CMake configure before building"
    Write-Output "-NoCache `t Disables sccache/ccache for this build"

    exit
}

# if user specified clean, remove all build files
if ($clean.IsPresent) {
    if (Test-Path -Path "build") {
        remove-item build -R -Force 
    }
}


if (($clean.IsPresent) -or (-not (Test-Path -Path "build"))) {
    new-item -Path build -ItemType Directory
} 

$cmakeArgs = @("-G", "Ninja", "-DCMAKE_BUILD_TYPE=RelWithDebInfo", "-B", "build")
$forceConfigure = $configure.IsPresent
if ($noCache.IsPresent) {
    $env:SNORESABER_COMPILER_CACHE = "off"
    $forceConfigure = $true
}

if (-not [string]::IsNullOrWhiteSpace($env:SNORESABER_COMPILER_CACHE)) {
    $cmakeArgs += "-DSNORESABER_COMPILER_CACHE=$($env:SNORESABER_COMPILER_CACHE)"
    $forceConfigure = $true
}

$needsConfigure = $clean.IsPresent -or $forceConfigure -or (-not (Test-Path -Path "build/build.ninja"))
if ($needsConfigure) {
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

& cmake --build ./build
exit $LASTEXITCODE
