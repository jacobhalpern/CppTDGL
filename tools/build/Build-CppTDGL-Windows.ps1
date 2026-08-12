<#
Build-CppTDGL-Windows.ps1

Clean/configure/build/test/install/deploy helper for the current CppTDGL Windows/MSVC build.

Default behavior:
  - builds Release
  - enables Qt if available
  - uses a separate Qt build folder
  - installs into cppTDGL/dist/CppTDGL-windows-x64-<Configuration>
  - runs windeployqt so Qt DLLs/plugins are copied beside CppTDGLQtApp.exe
  - writes a distributable zip unless -NoZip is passed

Examples:
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Build-CppTDGL-Windows.ps1 -Clean
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Build-CppTDGL-Windows.ps1 -Configuration Debug -Clean
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Build-CppTDGL-Windows.ps1 -NoQt -Clean
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Build-CppTDGL-Windows.ps1 -QtPrefix C:\Qt\6.10.2\msvc2022_64 -Clean
#>

[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [string]$SourceDir = "",

    [string]$QtPrefix = "",

    [switch]$Clean,

    [switch]$NoQt,

    [switch]$SkipTests,

    [switch]$NoZip,

    [switch]$LaunchQt
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-CheckedProcess {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    Write-Host ""
    Write-Host "> $FilePath $($Arguments -join ' ')" -ForegroundColor Cyan
    & $FilePath @Arguments
    $exitCode = $LASTEXITCODE
    if ($null -ne $exitCode -and $exitCode -ne 0) {
        throw "Command failed with exit code ${exitCode}: $FilePath $($Arguments -join ' ')"
    }
}

function Resolve-SourceDir {
    param([string]$RequestedSourceDir)

    if ($RequestedSourceDir) {
        $resolved = (Resolve-Path $RequestedSourceDir).Path
        if (-not (Test-Path (Join-Path $resolved "CMakeLists.txt"))) {
            throw "SourceDir does not contain CMakeLists.txt: $resolved"
        }
        return $resolved
    }

    $cwd = (Get-Location).Path
    $candidateNested = Join-Path $cwd "cppTDGL"
    if (Test-Path (Join-Path $candidateNested "CMakeLists.txt")) {
        return (Resolve-Path $candidateNested).Path
    }

    if (Test-Path (Join-Path $cwd "CMakeLists.txt")) {
        return (Resolve-Path $cwd).Path
    }

    throw "Could not find cppTDGL/CMakeLists.txt or CMakeLists.txt from current directory: $cwd"
}

function Convert-ToCMakePath {
    param([string]$PathValue)
    return (($PathValue -replace "\\", "/") -replace "//", "/")
}

function Find-MsvcQtPrefix {
    $qtRoot = "C:\Qt"
    if (-not (Test-Path $qtRoot)) {
        return ""
    }

    $configs = Get-ChildItem -Path $qtRoot -Recurse -Filter "Qt6Config.cmake" -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match "\\msvc2022_64\\lib\\cmake\\Qt6\\Qt6Config\.cmake$" }

    if (-not $configs) {
        return ""
    }

    $ranked = foreach ($config in $configs) {
        $prefix = $config.FullName -replace "\\lib\\cmake\\Qt6\\Qt6Config\.cmake$", ""
        $versionText = Split-Path (Split-Path $prefix -Parent) -Leaf
        $versionValue = $null
        try {
            $versionValue = [version]$versionText
        } catch {
            $versionValue = [version]"0.0.0"
        }
        [pscustomobject]@{
            Prefix = $prefix
            Version = $versionValue
        }
    }

    return ($ranked | Sort-Object Version -Descending | Select-Object -First 1).Prefix
}

$withQt = -not $NoQt.IsPresent
$source = Resolve-SourceDir -RequestedSourceDir $SourceDir
$sourceName = Split-Path $source -Leaf
$repoRoot = Split-Path $source -Parent

if ($withQt) {
    if (-not $QtPrefix) {
        $QtPrefix = Find-MsvcQtPrefix
    }

    if (-not $QtPrefix) {
        throw "Qt was requested, but no C:\Qt\<version>\msvc2022_64 Qt6Config.cmake installation was found. Pass -QtPrefix C:\Qt\6.10.2\msvc2022_64 or run with -NoQt."
    }

    $qtConfig = Join-Path $QtPrefix "lib\cmake\Qt6\Qt6Config.cmake"
    if (-not (Test-Path $qtConfig)) {
        throw "QtPrefix does not contain lib\cmake\Qt6\Qt6Config.cmake: $QtPrefix"
    }
}

$configurationLower = $Configuration.ToLowerInvariant()
$buildLeaf = if ($withQt) { "vs2022-x64-qt-$configurationLower" } else { "vs2022-x64-core-$configurationLower" }
$buildDir = Join-Path $source "build\$buildLeaf"
$distRoot = Join-Path $source "dist"
$installDir = Join-Path $distRoot "CppTDGL-windows-x64-$Configuration"
$zipPath = Join-Path $distRoot "CppTDGL-windows-x64-$Configuration.zip"

Write-Host "CppTDGL Windows build/deploy" -ForegroundColor Green
Write-Host "SourceDir:     $source"
Write-Host "BuildDir:      $buildDir"
Write-Host "InstallDir:    $installDir"
Write-Host "Configuration: $Configuration"
Write-Host "Qt enabled:    $withQt"
if ($withQt) {
    Write-Host "QtPrefix:      $QtPrefix"
}

if ($Clean) {
    Write-Host ""
    Write-Host "Cleaning build/install/package output..." -ForegroundColor Yellow
    foreach ($path in @($buildDir, $installDir, $zipPath)) {
        if (Test-Path $path) {
            Remove-Item -Recurse -Force $path
            Write-Host "Removed: $path"
        }
    }
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
New-Item -ItemType Directory -Force -Path $distRoot | Out-Null

$configureArgs = @(
    "-S", (Convert-ToCMakePath $source),
    "-B", (Convert-ToCMakePath $buildDir),
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCPPTDGL_ENABLE_PACKAGING=ON"
)

if ($withQt) {
    $configureArgs += "-DCPPTDGL_ENABLE_QT=ON"
    $configureArgs += "-DCMAKE_PREFIX_PATH=$(Convert-ToCMakePath $QtPrefix)"
} else {
    $configureArgs += "-DCPPTDGL_ENABLE_QT=OFF"
}

Invoke-CheckedProcess -FilePath "cmake" -Arguments $configureArgs

Invoke-CheckedProcess -FilePath "cmake" -Arguments @(
    "--build", (Convert-ToCMakePath $buildDir),
    "--config", $Configuration
)

if (-not $SkipTests) {
    Invoke-CheckedProcess -FilePath "ctest" -Arguments @(
        "--test-dir", (Convert-ToCMakePath $buildDir),
        "-C", $Configuration,
        "--output-on-failure"
    )
}

if (Test-Path $installDir) {
    Remove-Item -Recurse -Force $installDir
}

Invoke-CheckedProcess -FilePath "cmake" -Arguments @(
    "--install", (Convert-ToCMakePath $buildDir),
    "--config", $Configuration,
    "--prefix", (Convert-ToCMakePath $installDir)
)

$binDir = Join-Path $installDir "bin"
$cliExe = Join-Path $binDir "CppTDGLSmokeCli.exe"
$qtExe = Join-Path $binDir "CppTDGLQtApp.exe"

if (-not (Test-Path $cliExe)) {
    throw "Expected CLI executable was not installed: $cliExe"
}

if ($withQt) {
    if (-not (Test-Path $qtExe)) {
        throw "Expected Qt executable was not installed: $qtExe"
    }

    $windeployqt = Join-Path $QtPrefix "bin\windeployqt.exe"
    if (-not (Test-Path $windeployqt)) {
        throw "windeployqt.exe not found: $windeployqt"
    }

    $deployMode = if ($Configuration -eq "Debug") { "--debug" } else { "--release" }
    Invoke-CheckedProcess -FilePath $windeployqt -Arguments @(
        $deployMode,
        "--compiler-runtime",
        "--dir", $binDir,
        $qtExe
    )

    $qtCoreDll = if ($Configuration -eq "Debug") { Join-Path $binDir "Qt6Cored.dll" } else { Join-Path $binDir "Qt6Core.dll" }
    $platformDll = if ($Configuration -eq "Debug") { Join-Path $binDir "platforms\qwindowsd.dll" } else { Join-Path $binDir "platforms\qwindows.dll" }

    if (-not (Test-Path $qtCoreDll)) {
        throw "Qt deployment did not copy expected Qt Core DLL: $qtCoreDll"
    }
    if (-not (Test-Path $platformDll)) {
        throw "Qt deployment did not copy expected Windows platform plugin: $platformDll"
    }
}

$manifestPath = Join-Path $installDir "BUILD_MANIFEST.txt"
$manifest = @()
$manifest += "CppTDGL build manifest"
$manifest += "Generated:      $(Get-Date -Format o)"
$manifest += "SourceDir:      $source"
$manifest += "BuildDir:       $buildDir"
$manifest += "Configuration:  $Configuration"
$manifest += "Qt enabled:     $withQt"
if ($withQt) { $manifest += "QtPrefix:       $QtPrefix" }
$manifest += ""
$manifest += "Primary executables:"
$manifest += "  bin\CppTDGLSmokeCli.exe"
if ($withQt) { $manifest += "  bin\CppTDGLQtApp.exe" }
$manifest += ""
$manifest += "Run commands from this folder:"
$manifest += "  .\bin\CppTDGLSmokeCli.exe --keep-output"
if ($withQt) { $manifest += "  .\bin\CppTDGLQtApp.exe" }
$manifest | Set-Content -Path $manifestPath -Encoding UTF8

if (-not $NoZip) {
    if (Test-Path $zipPath) {
        Remove-Item -Force $zipPath
    }
    Compress-Archive -Path (Join-Path $installDir "*") -DestinationPath $zipPath -Force
}

Write-Host ""
Write-Host "Build/deploy completed." -ForegroundColor Green
Write-Host "Install folder: $installDir"
if (-not $NoZip) {
    Write-Host "Zip package:    $zipPath"
}
Write-Host "CLI exe:        $cliExe"
if ($withQt) {
    Write-Host "Qt exe:         $qtExe"
}

if ($LaunchQt -and $withQt) {
    Write-Host ""
    Write-Host "Launching Qt app..." -ForegroundColor Yellow
    Start-Process -FilePath $qtExe -WorkingDirectory $binDir
}
