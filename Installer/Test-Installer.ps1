# Runs only on the disposable Windows CI runner. It never removes an existing installation.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$SetupFile,
    [Parameter(Mandatory)][string]$PackageRoot,
    [Parameter(Mandatory)][string]$ValidatorPath,
    [string]$LogRoot = (Join-Path $PSScriptRoot '..\validation\installer')
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if (-not $IsWindows -or -not [Environment]::Is64BitProcess -or $env:GITHUB_ACTIONS -ne 'true') {
    throw 'This installation test is restricted to a disposable 64-bit Windows GitHub Actions runner.'
}
$setup = (Resolve-Path -LiteralPath $SetupFile).Path
$package = (Resolve-Path -LiteralPath $PackageRoot).Path
$validator = (Resolve-Path -LiteralPath $ValidatorPath).Path
New-Item -ItemType Directory -Force -Path $LogRoot | Out-Null
$logs = (Resolve-Path -LiteralPath $LogRoot).Path
$appRoot = Join-Path $env:RUNNER_TEMP ('RaBubanj-Installer-' + [guid]::NewGuid().ToString('N'))
$vstRoot = Join-Path $env:CommonProgramFiles 'VST3\RaBubanj.vst3'
$menuRoot = Join-Path ([Environment]::GetFolderPath('CommonStartMenu')) 'Programs\RaBubanj'
$registry = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{E9DCCBBC-9486-4ACF-A28A-7801A09472F2}_is1'
foreach ($path in @($appRoot, $vstRoot, $menuRoot, $registry)) {
    if (Test-Path -LiteralPath $path) { throw "Refusing to overwrite an existing installation: $path" }
}
function Assert-True([bool]$Condition, [string]$Message) { if (-not $Condition) { throw $Message } }
function Assert-SameFile([string]$Expected, [string]$Actual) {
    Assert-True (Test-Path -LiteralPath $Actual -PathType Leaf) "Installed file missing: $Actual"
    Assert-True ((Get-FileHash -LiteralPath $Expected -Algorithm SHA256).Hash -eq (Get-FileHash -LiteralPath $Actual -Algorithm SHA256).Hash) "Installed file differs: $Actual"
}
function Assert-Payload([bool]$Vst3, [bool]$Standalone) {
    Assert-True (Test-Path -LiteralPath $registry) 'Windows uninstall registration missing'
    if ($Vst3) {
        $sourceVst = Join-Path $package 'RaBubanj.vst3'
        $sourceFiles = @(Get-ChildItem -LiteralPath $sourceVst -Recurse -File)
        $actualFiles = @(Get-ChildItem -LiteralPath $vstRoot -Recurse -File)
        Assert-True ($sourceFiles.Count -eq $actualFiles.Count) 'Installed VST3 bundle has the wrong file count'
        foreach ($file in $sourceFiles) {
            Assert-SameFile $file.FullName (Join-Path $vstRoot ([IO.Path]::GetRelativePath($sourceVst, $file.FullName)))
        }
    } else { Assert-True (-not (Test-Path -LiteralPath $vstRoot)) 'VST3-only files were installed without selecting VST3' }
    $exe = Join-Path $appRoot 'RaBubanj.exe'
    $shortcut = Join-Path $menuRoot 'RaBubanj.lnk'
    if ($Standalone) {
        Assert-SameFile (Join-Path $package 'RaBubanj.exe') $exe
        Assert-True (Test-Path -LiteralPath $shortcut -PathType Leaf) 'Standalone Start menu shortcut missing'
        $shell = New-Object -ComObject WScript.Shell
        Assert-True ($shell.CreateShortcut($shortcut).TargetPath -eq $exe) 'Standalone shortcut points to the wrong application'
    } else {
        Assert-True (-not (Test-Path -LiteralPath $exe)) 'Standalone was installed without selecting it'
        Assert-True (-not (Test-Path -LiteralPath $shortcut)) 'A nonfunctional Standalone shortcut was installed'
    }
    foreach ($file in @('LICENSE', 'THIRD_PARTY.md', 'PROCITAJ-ME.md', 'INSTALLER-HR.md')) {
        Assert-SameFile (Join-Path $package $file) (Join-Path $appRoot $file)
    }
    foreach ($file in @('RaBubanj-source.zip', 'JUCE-source.tar.gz')) {
        Assert-SameFile (Join-Path $package $file) (Join-Path $appRoot "Source\$file")
    }
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $package 'ThirdParty') -File) {
        Assert-SameFile $file.FullName (Join-Path $appRoot "ThirdParty\$($file.Name)")
    }
    Assert-True (@(Get-ChildItem -LiteralPath $appRoot -Filter 'unins*.exe' -File).Count -eq 1) 'There must be one uninstaller'
}
function Install-Components([string]$Components, [string]$Name) {
    $arguments = @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/SP-', '/TYPE=custom', "/DIR=`"$appRoot`"", '/GROUP=RaBubanj', "/COMPONENTS=$Components", '/TASKS=', "/LOG=`"$logs\$Name.log`"")
    $process = Start-Process -FilePath $setup -ArgumentList $arguments -Wait -PassThru
    Assert-True ($process.ExitCode -eq 0) "Installer failed ($Name): $($process.ExitCode)"
    Write-Output "Installed: $Components"
}
function Uninstall-Application([string]$Name) {
    $uninstaller = @(Get-ChildItem -LiteralPath $appRoot -Filter 'unins*.exe' -File)
    Assert-True ($uninstaller.Count -eq 1) 'Cannot select the uninstaller'
    $process = Start-Process -FilePath ($uninstaller[0].FullName) -ArgumentList @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', "/LOG=`"$logs\$Name.log`"") -Wait -PassThru
    Assert-True ($process.ExitCode -eq 0) "Uninstaller failed ($Name): $($process.ExitCode)"
    Assert-True (-not (Test-Path -LiteralPath $registry)) 'Uninstall entry was not removed'
    Assert-True (-not (Test-Path -LiteralPath $vstRoot)) 'Installed VST3 bundle was not removed'
    Assert-True (-not (Test-Path -LiteralPath (Join-Path $appRoot 'RaBubanj.exe'))) 'Standalone was not removed'
    Assert-True (-not (Test-Path -LiteralPath $menuRoot)) 'Start menu shortcuts were not removed'
    # The uninstaller deletes its own temporary executable after its main process ends.
    $timer = [Diagnostics.Stopwatch]::StartNew()
    while ((Test-Path -LiteralPath $appRoot) -and @(Get-ChildItem -LiteralPath $appRoot -Filter 'unins*' -File).Count -gt 0 -and $timer.Elapsed.TotalSeconds -lt 15) {
        Start-Sleep -Milliseconds 100
    }
    if (Test-Path -LiteralPath $appRoot) {
        foreach ($file in Get-ChildItem -LiteralPath $appRoot -Recurse -File) {
            Assert-True ([IO.Path]::GetRelativePath($appRoot, $file.FullName) -eq 'UserData\keep-me.txt') "Installed file left after uninstall: $($file.FullName)"
        }
    }
    Write-Output "Uninstalled: $Name"
}

Install-Components 'vst3' '01-vst3-only-install'
Assert-Payload $true $false
Uninstall-Application '02-vst3-only-uninstall'

Install-Components 'standalone' '03-standalone-only-install'
Assert-Payload $false $true
Uninstall-Application '04-standalone-only-uninstall'

Install-Components 'vst3,standalone' '05-full-install'
Assert-Payload $true $true
$versionMatch = [regex]::Match((Get-Content (Join-Path $PSScriptRoot '..\CMakeLists.txt') -Raw), 'project\(RaBubanj VERSION ([0-9]+\.[0-9]+\.[0-9]+)')
$entry = Get-ItemProperty -LiteralPath $registry
Assert-True ($entry.DisplayVersion -eq $versionMatch.Groups[1].Value -and $entry.Publisher -eq 'Voyagersquare') 'Wrong installed version or publisher'

# Repair a changed installed executable and plugin, without deleting user-created files.
$marker = Join-Path $appRoot 'UserData\keep-me.txt'
New-Item -ItemType Directory -Force -Path (Split-Path $marker -Parent) | Out-Null
'RaBubanj user data must survive reinstall and uninstall.' | Set-Content -LiteralPath $marker -Encoding utf8
$markerHash = (Get-FileHash -LiteralPath $marker -Algorithm SHA256).Hash
foreach ($file in @((Join-Path $appRoot 'RaBubanj.exe'), (Join-Path $vstRoot 'Contents\x86_64-win\RaBubanj.vst3'))) {
    $stream = [IO.File]::Open($file, [IO.FileMode]::Append)
    try { $stream.WriteByte(0) } finally { $stream.Dispose() }
}
Install-Components 'vst3,standalone' '06-full-reinstall'
Assert-Payload $true $true
Assert-True ((Get-FileHash -LiteralPath $marker -Algorithm SHA256).Hash -eq $markerHash) 'Reinstall changed user-created data'

# Validate the plugin at its actual installed Common Files path, including spaces.
$validationArgs = @('--strictness-level', '5', '--validate', "`"$vstRoot`"", '--output-dir', "`"$logs`"", '--timeout-ms', '120000')
$result = Start-Process -FilePath $validator -ArgumentList $validationArgs -Wait -PassThru -NoNewWindow -RedirectStandardOutput "$logs\pluginval-stdout.txt" -RedirectStandardError "$logs\pluginval-stderr.txt"
Get-Content -LiteralPath "$logs\pluginval-stdout.txt"
Get-Content -LiteralPath "$logs\pluginval-stderr.txt"
Assert-True ($result.ExitCode -eq 0) 'Installed VST3 failed pluginval strictness 5'

# Launch only the installed standalone owned by this test; no audio hardware is assumed.
# Process.MainWindowHandle omits owned windows. Enumerate visible windows of this PID
# instead, and allow asynchronous JUCE startup to finish after WaitForInputIdle.
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class RaBubanjSmokeWindow {
    private delegate bool EnumWindowsProc(IntPtr window, IntPtr parameter);
    [DllImport("user32.dll")] private static extern bool EnumWindows(EnumWindowsProc callback, IntPtr parameter);
    [DllImport("user32.dll")] private static extern bool IsWindowVisible(IntPtr window);
    [DllImport("user32.dll")] private static extern uint GetWindowThreadProcessId(IntPtr window, out uint processId);
    public static IntPtr Find(int processId) {
        IntPtr found = IntPtr.Zero;
        EnumWindows((window, parameter) => {
            uint owner; GetWindowThreadProcessId(window, out owner);
            if (owner == (uint)processId && IsWindowVisible(window)) { found = window; return false; }
            return true;
        }, IntPtr.Zero);
        return found;
    }
}
'@
$application = Start-Process -FilePath (Join-Path $appRoot 'RaBubanj.exe') -PassThru
try {
    Assert-True ($application.WaitForInputIdle(30000)) 'Standalone did not become responsive'
    $startup = [Diagnostics.Stopwatch]::StartNew()
    $window = [IntPtr]::Zero
    while ($startup.Elapsed.TotalSeconds -lt 30) {
        $application.Refresh()
        if ($application.HasExited) { throw "Standalone exited during startup: $($application.ExitCode)" }
        $window = [RaBubanjSmokeWindow]::Find($application.Id)
        if ($window -ne [IntPtr]::Zero) { break }
        Start-Sleep -Milliseconds 200
    }
    Assert-True ($window -ne [IntPtr]::Zero) 'Installed Standalone did not create a visible window within 30 seconds'
    Write-Output "Installed Standalone launch: PASS (PID $($application.Id), window $window)"
} finally {
    $application.Refresh()
    if (-not $application.HasExited) {
        $application.CloseMainWindow() | Out-Null
        if (-not $application.WaitForExit(5000)) { $application.Kill(); $application.WaitForExit() }
    }
    $application.Dispose()
}
Uninstall-Application '07-full-uninstall'
Assert-True ((Get-FileHash -LiteralPath $marker -Algorithm SHA256).Hash -eq $markerHash) 'Uninstall removed or changed user-created data'
Write-Output 'INSTALLER SMOKE TESTS: PASS (VST3 only, Standalone only, full, reinstall, installed pluginval, Standalone launch, uninstall, retained user data)'
