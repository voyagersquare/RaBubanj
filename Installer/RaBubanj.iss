; Build with Installer/Build-Installer.ps1 after collecting the validated package.
#ifndef AppVersion
  #error "AppVersion must be supplied by the build script"
#endif
#ifndef PackageRoot
  #error "PackageRoot must be supplied by the build script"
#endif

[Setup]
; Keep this ID stable for future upgrades and the Windows uninstall entry.
AppId={{E9DCCBBC-9486-4ACF-A28A-7801A09472F2}
AppName=RaBubanj
AppVersion={#AppVersion}
AppPublisher=Voyagersquare
AppPublisherURL=https://github.com/voyagersquare/RaBubanj
AppSupportURL=https://github.com/voyagersquare/RaBubanj/issues
DefaultDirName={autopf}\Voyagersquare\RaBubanj
DefaultGroupName=RaBubanj
AllowNoIcons=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
PrivilegesRequired=admin
WizardStyle=modern
LicenseFile={#PackageRoot}\LICENSE
OutputBaseFilename=RaBubanj-{#AppVersion}-Setup-x64
Compression=lzma2
SolidCompression=yes
CloseApplications=yes
RestartApplications=no
SetupLogging=yes
UninstallDisplayName=RaBubanj {#AppVersion} (64-bit)
VersionInfoVersion={#AppVersion}.0
VersionInfoCompany=Voyagersquare
VersionInfoDescription=RaBubanj VST3 and Standalone Installer

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "VST3 + Standalone"
Name: "vst3only"; Description: "VST3 plugin only"
Name: "standaloneonly"; Description: "Standalone application only"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3"; Description: "RaBubanj VST3 plugin (64-bit)"; Types: full vst3only
Name: "standalone"; Description: "RaBubanj Standalone application (64-bit)"; Types: full standaloneonly

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"; Components: standalone; Flags: unchecked

[Files]
Source: "{#PackageRoot}\RaBubanj.vst3\*"; DestDir: "{commoncf64}\VST3\RaBubanj.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#PackageRoot}\RaBubanj.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion
Source: "{#PackageRoot}\PROCITAJ-ME.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageRoot}\INSTALLER-HR.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageRoot}\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageRoot}\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageRoot}\THIRD_PARTY.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageRoot}\ThirdParty\*"; DestDir: "{app}\ThirdParty"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#PackageRoot}\RaBubanj-source.zip"; DestDir: "{app}\Source"; Flags: ignoreversion
Source: "{#PackageRoot}\JUCE-source.tar.gz"; DestDir: "{app}\Source"; Flags: ignoreversion

[Icons]
Name: "{group}\RaBubanj"; Filename: "{app}\RaBubanj.exe"; Components: standalone
Name: "{group}\Upute za RaBubanj"; Filename: "{app}\PROCITAJ-ME.md"
Name: "{group}\Uninstall RaBubanj"; Filename: "{uninstallexe}"
Name: "{autodesktop}\RaBubanj"; Filename: "{app}\RaBubanj.exe"; Components: standalone; Tasks: desktopicon

[Run]
Filename: "{app}\RaBubanj.exe"; Description: "Launch RaBubanj Standalone"; Components: standalone; Flags: nowait postinstall skipifsilent unchecked

[Code]
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if not (WizardIsComponentSelected('vst3') or WizardIsComponentSelected('standalone')) then
    Result := 'Select at least one component: VST3 or Standalone.';
end;
