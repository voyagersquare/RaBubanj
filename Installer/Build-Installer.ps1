[CmdletBinding()]
param(
    [string]$PackageRoot = (Join-Path $PSScriptRoot '..\package'),
    [string]$OutputRoot = (Join-Path $PSScriptRoot '..\installer-package')
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repository = Split-Path $PSScriptRoot -Parent
$versionMatch = [regex]::Match((Get-Content (Join-Path $repository 'CMakeLists.txt') -Raw), 'project\(RaBubanj VERSION ([0-9]+\.[0-9]+\.[0-9]+)')
if (-not $versionMatch.Success) { throw 'Cannot read RaBubanj version from CMakeLists.txt' }
$version = $versionMatch.Groups[1].Value
$package = (Resolve-Path -LiteralPath $PackageRoot).Path
$compiler = Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'
if (-not (Test-Path -LiteralPath $compiler -PathType Leaf)) { throw "Inno Setup 6 compiler not found: $compiler" }
$compilerVersion = (Get-Item -LiteralPath $compiler).VersionInfo.ProductVersion
Write-Output "Inno Setup compiler: $compilerVersion"
foreach ($required in @('RaBubanj.exe', 'RaBubanj.vst3\Contents\x86_64-win\RaBubanj.vst3', 'RaBubanj.vst3\Contents\Resources\moduleinfo.json', 'LICENSE', 'THIRD_PARTY.md', 'INSTALLER-HR.md', 'RaBubanj-source.zip', 'JUCE-source.tar.gz', 'ThirdParty\INNO-LICENSE.txt')) {
    if (-not (Test-Path -LiteralPath (Join-Path $package $required) -PathType Leaf)) { throw "Missing installer input: $required" }
}
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
$output = (Resolve-Path -LiteralPath $OutputRoot).Path
& $compiler "/DAppVersion=$version" "/DPackageRoot=$package" "/O$output" (Join-Path $PSScriptRoot 'RaBubanj.iss')
if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed: $LASTEXITCODE" }
$setupName = "RaBubanj-$version-Setup-x64.exe"
$setup = Join-Path $output $setupName
if (-not (Test-Path -LiteralPath $setup -PathType Leaf)) { throw 'Installer was not produced' }
$digest = (Get-FileHash -LiteralPath $setup -Algorithm SHA256).Hash.ToLowerInvariant()
"$digest  $setupName" | Set-Content -LiteralPath (Join-Path $output 'SHA256SUMS.txt') -Encoding ascii
Copy-Item -LiteralPath (Join-Path $repository 'docs\INSTALLER-HR.md') -Destination (Join-Path $output 'PROCITAJ-ME.md')
"RaBubanj $version`nInno Setup $compilerVersion`nSource commit: $(git -C $repository rev-parse HEAD)`n" | Set-Content -LiteralPath (Join-Path $output 'BUILD-INFO.txt') -Encoding utf8
Write-Output "Built $setupName; SHA256 $digest"
