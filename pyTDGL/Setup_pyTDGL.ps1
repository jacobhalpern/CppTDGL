<#
Setup_pyTDGL.ps1

Purpose:
  Set up the pyTDGL Python project on Windows and create one simple runner.

Recommended use:
  1. Save this file into the pyTDGL project folder.
  2. Open PowerShell.
  3. Run:
       powershell -ExecutionPolicy Bypass -File .\Setup_pyTDGL.ps1
  4. After setup, run:
       .\Run_pyTDGL.cmd

If the script is not inside the project folder, run:
  powershell -ExecutionPolicy Bypass -File .\Setup_pyTDGL.ps1 -ProjectRoot "C:\Users\Jacob Halpern\Documents\GitHub\CppTDGL\pyTDGL"
#>

[CmdletBinding()]
param(
    [string]$ProjectRoot = "",

    [ValidateSet("standard", "dev", "devdocs")]
    [string]$InstallProfile = "standard",

    [switch]$RecreateVenv,
    [switch]$RunTests
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Section {
    param([string]$Message)
    Write-Host ""
    Write-Host ("=" * 100) -ForegroundColor DarkGray
    Write-Host $Message -ForegroundColor Cyan
    Write-Host ("=" * 100) -ForegroundColor DarkGray
}

function Invoke-Checked {
    param(
        [Parameter(Mandatory=$true)][string]$FilePath,
        [Parameter(ValueFromRemainingArguments=$true)][string[]]$Arguments
    )

    Write-Host "> $FilePath $($Arguments -join ' ')" -ForegroundColor DarkGray
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $FilePath $($Arguments -join ' ')"
    }
}

function Test-ProjectRoot {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) { return $false }
    if (-not (Test-Path -LiteralPath (Join-Path $Path "setup.py") -PathType Leaf)) { return $false }
    if (-not (Test-Path -LiteralPath (Join-Path $Path "tdgl") -PathType Container)) { return $false }
    return $true
}

function Resolve-ProjectRoot {
    param([string]$ExplicitProjectRoot)

    $candidatePaths = New-Object System.Collections.Generic.List[string]

    if (-not [string]::IsNullOrWhiteSpace($ExplicitProjectRoot)) {
        $candidatePaths.Add($ExplicitProjectRoot)
    }

    $candidatePaths.Add((Get-Location).Path)

    if ($PSScriptRoot) {
        $candidatePaths.Add($PSScriptRoot)
    }

    $candidatePaths.Add("C:\Users\Jacob Halpern\Documents\GitHub\CppTDGL\pyTDGL")

    foreach ($candidate in $candidatePaths) {
        try {
            $resolved = Resolve-Path -LiteralPath $candidate -ErrorAction Stop | Select-Object -First 1 -ExpandProperty Path
            if (Test-ProjectRoot $resolved) {
                return $resolved
            }
        }
        catch {
            # Keep trying candidates.
        }
    }

    throw @"
Could not find the pyTDGL project folder.

Expected a folder containing both:
  - setup.py
  - tdgl\

Run this script from the project root, or pass the project path explicitly:
  powershell -ExecutionPolicy Bypass -File .\Setup_pyTDGL.ps1 -ProjectRoot "C:\Users\Jacob Halpern\Documents\GitHub\CppTDGL\pyTDGL"
"@
}

function Find-CompatiblePython {
    $probe = @"
import sys
ok = (3, 8) <= sys.version_info[:2] < (3, 15)
print(sys.executable)
print(f'{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}')
sys.exit(0 if ok else 2)
"@

    $candidates = @(
        @{ Exe = "py"; Args = @("-3.12") },
        @{ Exe = "py"; Args = @("-3.13") },
        @{ Exe = "py"; Args = @("-3.11") },
        @{ Exe = "py"; Args = @("-3.10") },
        @{ Exe = "py"; Args = @("-3.9") },
        @{ Exe = "python"; Args = @() },
        @{ Exe = "python3"; Args = @() }
    )

    foreach ($candidate in $candidates) {
        $exe = [string]$candidate.Exe
        $prefixArgs = [string[]]$candidate.Args

        try {
            $output = & $exe @prefixArgs -c $probe 2>$null
            if ($LASTEXITCODE -eq 0 -and $output.Count -ge 2) {
                return [PSCustomObject]@{
                    Exe = $exe
                    PrefixArgs = $prefixArgs
                    ExecutablePath = [string]$output[0]
                    Version = [string]$output[1]
                }
            }
        }
        catch {
            # Keep trying candidates.
        }
    }

    throw @"
Could not find a compatible Python installation.

pyTDGL supports Python >=3.8 and <3.15. Python 3.12 is recommended.
Install Python from https://www.python.org/downloads/windows/ and make sure the Python launcher is installed.
"@
}

function Invoke-PythonLauncherChecked {
    param(
        [Parameter(Mandatory=$true)]$PythonInfo,
        [Parameter(ValueFromRemainingArguments=$true)][string[]]$Arguments
    )

    $allArgs = @($PythonInfo.PrefixArgs) + @($Arguments)
    Invoke-Checked -FilePath $PythonInfo.Exe -Arguments $allArgs
}

Write-Section "Resolving pyTDGL project folder"
$ResolvedProjectRoot = Resolve-ProjectRoot -ExplicitProjectRoot $ProjectRoot
Write-Host "Project root: $ResolvedProjectRoot" -ForegroundColor Green
Set-Location -LiteralPath $ResolvedProjectRoot

Write-Section "Checking Python"
$PythonInfo = Find-CompatiblePython
Write-Host "Python command: $($PythonInfo.Exe) $($PythonInfo.PrefixArgs -join ' ')" -ForegroundColor Green
Write-Host "Python executable: $($PythonInfo.ExecutablePath)" -ForegroundColor Green
Write-Host "Python version: $($PythonInfo.Version)" -ForegroundColor Green

Write-Section "Creating or reusing virtual environment"
$VenvDir = Join-Path $ResolvedProjectRoot ".venv"
$VenvPython = Join-Path $VenvDir "Scripts\python.exe"

if ($RecreateVenv -and (Test-Path -LiteralPath $VenvDir)) {
    Write-Host "Removing existing virtual environment: $VenvDir" -ForegroundColor Yellow
    Remove-Item -LiteralPath $VenvDir -Recurse -Force
}

if (-not (Test-Path -LiteralPath $VenvPython -PathType Leaf)) {
    Invoke-PythonLauncherChecked -PythonInfo $PythonInfo -Arguments @("-m", "venv", $VenvDir)
}
else {
    Write-Host "Virtual environment already exists: $VenvDir" -ForegroundColor Green
}

Write-Section "Upgrading package tooling"
Invoke-Checked -FilePath $VenvPython -Arguments @("-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel")

Write-Section "Installing pyTDGL"
if ($InstallProfile -eq "standard") {
    Invoke-Checked -FilePath $VenvPython -Arguments @("-m", "pip", "install", "-e", ".")
}
elseif ($InstallProfile -eq "dev") {
    Invoke-Checked -FilePath $VenvPython -Arguments @("-m", "pip", "install", "-e", ".[dev]")
}
elseif ($InstallProfile -eq "devdocs") {
    Invoke-Checked -FilePath $VenvPython -Arguments @("-m", "pip", "install", "-e", ".[dev,docs]")
}

Write-Section "Verifying tdgl import"
$verifyCode = @"
import sys
import tdgl
print('Python executable:', sys.executable)
print('tdgl module:', tdgl.__file__)
print('tdgl version:', getattr(tdgl, '__version__', 'unknown'))
"@
Invoke-Checked -FilePath $VenvPython -Arguments @("-c", $verifyCode)

Write-Section "Verifying tdgl.visualize CLI"
Invoke-Checked -FilePath $VenvPython -Arguments @("-m", "tdgl.visualize", "--help")

if ($RunTests) {
    Write-Section "Running tests"
    Invoke-Checked -FilePath $VenvPython -Arguments @("-m", "pytest", "tdgl\test", "-q")
}

Write-Section "Creating one-command runner"
$RunPs1Path = Join-Path $ResolvedProjectRoot "Run_pyTDGL.ps1"
$RunCmdPath = Join-Path $ResolvedProjectRoot "Run_pyTDGL.cmd"

$runPs1 = @'
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$VenvPython = Join-Path $ProjectRoot ".venv\Scripts\python.exe"
$QuickstartNotebook = Join-Path $ProjectRoot "docs\notebooks\quickstart.ipynb"

if (-not (Test-Path -LiteralPath $VenvPython -PathType Leaf)) {
    throw "Virtual environment not found. Run Setup_pyTDGL.ps1 first. Missing: $VenvPython"
}

Set-Location -LiteralPath $ProjectRoot

if (Test-Path -LiteralPath $QuickstartNotebook -PathType Leaf) {
    Write-Host "Opening pyTDGL quickstart notebook..." -ForegroundColor Cyan
    Write-Host "Notebook: $QuickstartNotebook" -ForegroundColor DarkGray
    & $VenvPython -m jupyter notebook $QuickstartNotebook
    exit $LASTEXITCODE
}
else {
    Write-Host "Quickstart notebook not found. Showing tdgl.visualize help instead." -ForegroundColor Yellow
    & $VenvPython -m tdgl.visualize --help
    exit $LASTEXITCODE
}
'@

$runCmd = @'
@echo off
setlocal
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Run_pyTDGL.ps1"
if errorlevel 1 (
    echo.
    echo pyTDGL runner failed. Review the error above.
    pause
)
endlocal
'@

Set-Content -LiteralPath $RunPs1Path -Value $runPs1 -Encoding UTF8
Set-Content -LiteralPath $RunCmdPath -Value $runCmd -Encoding ASCII

Write-Host "Created: $RunPs1Path" -ForegroundColor Green
Write-Host "Created: $RunCmdPath" -ForegroundColor Green

Write-Section "Setup complete"
Write-Host "To run pyTDGL after setup, use one command from the project folder:" -ForegroundColor Green
Write-Host "  .\Run_pyTDGL.cmd" -ForegroundColor White
Write-Host ""
Write-Host "Or double-click this file:" -ForegroundColor Green
Write-Host "  $RunCmdPath" -ForegroundColor White
