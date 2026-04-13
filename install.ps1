# LPP (Lua++) Installation Script for Windows
# Copyright (C) 2024-2025 Klinti Karaj, Albania

$ErrorActionPreference = "Stop"

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "This script requires administrator privileges to add LPP to system PATH." -ForegroundColor Yellow
    Write-Host "Please run PowerShell as Administrator and try again." -ForegroundColor Yellow
    exit 1
}

$ScriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$BinPath = Join-Path $ScriptPath "bin"

# Check if executables exist
if (-not (Test-Path (Join-Path $BinPath "lpp.exe"))) {
    Write-Host "Error: lpp.exe not found in $BinPath" -ForegroundColor Red
    Write-Host "Please build the project first by running: cd src && make mingw" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path (Join-Path $BinPath "lppac.exe"))) {
    Write-Host "Error: lppac.exe not found in $BinPath" -ForegroundColor Red
    Write-Host "Please build the project first by running: cd src && make mingw" -ForegroundColor Yellow
    exit 1
}

# Get current system PATH
$SystemPath = [Environment]::GetEnvironmentVariable("Path", "Machine")
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")

# Check if bin directory is already in PATH
if ($SystemPath -like "*$BinPath*" -or $UserPath -like "*$BinPath*") {
    Write-Host "LPP is already in PATH." -ForegroundColor Green
} else {
    # Add to user PATH
    [Environment]::SetEnvironmentVariable("Path", $UserPath + ";$BinPath", "User")
    Write-Host "Added $BinPath to user PATH." -ForegroundColor Green
    Write-Host "You may need to restart your terminal for changes to take effect." -ForegroundColor Yellow
}

# Verify installation
Write-Host "`nInstallation complete!" -ForegroundColor Green
Write-Host "LPP version:" -ForegroundColor Cyan
& (Join-Path $BinPath "lpp.exe") -v

Write-Host "`nTo use LPP in a new terminal, close and reopen your terminal." -ForegroundColor Yellow
Write-Host "Or refresh your PATH by running: `$env:Path = [System.Environment]::GetEnvironmentVariable(`"Path`",`"User`")" -ForegroundColor Yellow
