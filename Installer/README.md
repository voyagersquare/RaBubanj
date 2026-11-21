# Windows installer

Inno Setup 6 builds an x64 per-machine installer from the validated `package/` payload. The Windows runner supplies the compiler; its compiler-engine banner is logged in CI. `BUILD-INFO.txt` records the Windows file-version field, which is `0.0.0.0` in the current runner image; the actual compiler-engine version for this release is **6.7.1**. Inno Setup license: `INNO-LICENSE.txt`, copied verbatim from https://github.com/jrsoftware/issrc/blob/main/license.txt on 21 November 2026.

```powershell
./Installer/Build-Installer.ps1 -PackageRoot ./package -OutputRoot ./installer-package
```

The version comes from `CMakeLists.txt`. Keep the `.iss` AppId stable for reinstall/upgrade detection. VST3 goes to the 64-bit Common Files VST3 directory; the standalone goes to the selected application directory. Source archives, licenses and instructions are installed for every component selection. The installer does not download runtime dependencies and is not Authenticode-signed.

`Test-Installer.ps1` is restricted to disposable Windows GitHub Actions runners, refuses existing RaBubanj paths/registration, and checks VST3-only, standalone-only, full install, repeat install/repair, Start menu target, payload/source/license hashes, installed pluginval strictness 5, standalone window launch and uninstall. An untracked user-data marker must survive reinstall and uninstall. Installation logs and installed pluginval logs are retained in the validation artifact.

The test overrides only the application directory to a unique runner temp folder; the plugin is tested at its real system Common Files location. No real DAW/audio-hardware playback is claimed. Native wizard click-through and UAC interaction are not automated; silent modes exercise the same installation/component/uninstall logic.
