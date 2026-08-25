[CmdletBinding()]
param(
    [string]$DownloadDirectory = (Join-Path $env:LOCALAPPDATA "RoboCup\Ubuntu1804"),
    [switch]$CheckOnly
)

$ErrorActionPreference = "Stop"

$DistroName = "Ubuntu-18.04"
$AppxUri = "https://wslstorestorage.blob.core.windows.net/wslblob/Ubuntu_1804.2019.522.0_x64.appx"
$AppxFileName = "Ubuntu_1804.2019.522.0_x64.appx"
$ExpectedSha256 = "0b1abe8d5dc3ff416a06c9524e4f61a1fdf6ced583cb9b297ee72df1732ff403"

function Get-WslListText {
    $text = (& wsl.exe --list --verbose 2>$null | Out-String) -replace "`0", ""
    return $text
}

function Get-RegisteredDistros {
    $lines = & wsl.exe --list --quiet 2>$null
    return @($lines | ForEach-Object { ($_ -replace "`0", "").Trim() } | Where-Object { $_ })
}

function Test-DistroRegistered {
    return (Get-RegisteredDistros) -contains $DistroName
}

function Get-DefaultDistro {
    foreach ($line in (Get-WslListText) -split "`r?`n") {
        if ($line -match '^\s*\*\s+(\S+)') {
            return $Matches[1]
        }
    }
    return $null
}

function Get-DistroVersion {
    foreach ($line in (Get-WslListText) -split "`r?`n") {
        if ($line -match ('^\s*\*?\s*' + [regex]::Escape($DistroName) + '\s+.+?\s+([12])\s*$')) {
            return [int]$Matches[1]
        }
    }
    return $null
}

function Assert-DistroIdentity {
    if (-not (Test-DistroRegistered)) {
        throw "$DistroName is not registered. Finish its first-launch initialization and retry."
    }

    $osReleaseOutput = & wsl.exe -d $DistroName -- cat /etc/os-release
    $osReleaseExit = $LASTEXITCODE
    $osRelease = (($osReleaseOutput | Out-String) -replace "`0", "")
    if ($osReleaseExit -ne 0 -or $osRelease -notmatch '(?m)^VERSION_ID="?18\.04"?\s*$') {
        throw "$DistroName is registered but is not Ubuntu 18.04."
    }

    $architectureOutput = & wsl.exe -d $DistroName -- uname -m
    $architectureExit = $LASTEXITCODE
    $architecture = (($architectureOutput | Out-String) -replace "`0", "").Trim()
    if ($architectureExit -ne 0 -or $architecture -ne "x86_64") {
        throw "$DistroName must use the x86_64 architecture."
    }
}

if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT) {
    throw "This installer must run in Windows PowerShell."
}
if (-not [Environment]::Is64BitOperatingSystem) {
    throw "The RoboCup platform requires 64-bit Windows and x86_64 WSL."
}
if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
    throw "WSL is unavailable. Enable WSL and Virtual Machine Platform, reboot, then rerun this script."
}

$previousDefault = Get-DefaultDistro

if (Test-DistroRegistered) {
    Assert-DistroIdentity
    $wslVersion = Get-DistroVersion
    if ($wslVersion -ne 2) {
        if ($CheckOnly) {
            throw "$DistroName is registered as WSL$wslVersion; WSL2 is required."
        }
        & wsl.exe --set-version $DistroName 2
        if ($LASTEXITCODE -ne 0) {
            throw "Failed to convert $DistroName to WSL2."
        }
    }
    Write-Host "OK: $DistroName is Ubuntu 18.04, x86_64, WSL2."
    exit 0
}

if ($CheckOnly) {
    throw "$DistroName is not installed. Run this script without -CheckOnly."
}

New-Item -ItemType Directory -Path $DownloadDirectory -Force | Out-Null
$appxPath = Join-Path $DownloadDirectory $AppxFileName

if (Test-Path -LiteralPath $appxPath) {
    $actualSha256 = (Get-FileHash -LiteralPath $appxPath -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actualSha256 -ne $ExpectedSha256) {
        throw "Existing APPX SHA256 mismatch: $appxPath"
    }
    Write-Host "Using verified APPX: $appxPath"
} else {
    Write-Host "Downloading Ubuntu 18.04 APPX to $appxPath"
    $oldProgressPreference = $ProgressPreference
    try {
        $ProgressPreference = "SilentlyContinue"
        Invoke-WebRequest -Uri $AppxUri -OutFile $appxPath -UseBasicParsing
    } finally {
        $ProgressPreference = $oldProgressPreference
    }
    $actualSha256 = (Get-FileHash -LiteralPath $appxPath -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actualSha256 -ne $ExpectedSha256) {
        throw "Downloaded APPX SHA256 mismatch. Remove $appxPath and retry."
    }
}

$package = Get-AppxPackage | Where-Object { $_.Name -match 'Ubuntu18' } | Select-Object -First 1
if (-not $package) {
    Add-AppxPackage -Path $appxPath
    $package = Get-AppxPackage | Where-Object { $_.Name -match 'Ubuntu18' } | Select-Object -First 1
}
if (-not $package) {
    throw "Ubuntu 18.04 APPX installation completed but the package cannot be found."
}

$launcher = Join-Path $package.InstallLocation "ubuntu1804.exe"
if (-not (Test-Path -LiteralPath $launcher)) {
    throw "Ubuntu 18.04 launcher not found: $launcher"
}

Write-Host "A Ubuntu 18.04 window will open. Create your Linux username and password, then run 'exit'."
Start-Process -FilePath $launcher -Wait

Assert-DistroIdentity
if ((Get-DistroVersion) -ne 2) {
    & wsl.exe --set-version $DistroName 2
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to convert $DistroName to WSL2."
    }
}

if ($previousDefault -and ((Get-RegisteredDistros) -contains $previousDefault)) {
    & wsl.exe --set-default $previousDefault
    if ($LASTEXITCODE -ne 0) {
        throw "Ubuntu 18.04 is ready, but the previous default distro could not be restored."
    }
}

Write-Host "Ubuntu 18.04 is ready. Next, enter it with: wsl -d $DistroName"
