# RaBubanj 0.16.1 installer validation

Verified 21 November 2026. Tested source: `69af5ab51c0d105b80ea3b272136f9e1713c9116`.

[Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44883314452) · [Installer ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44883314452/artifacts/14699097022) · [Portable ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44883314452/artifacts/14699106880) · [Validation and installation logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44883314452/artifacts/14699405799)

## Windows build

- MSVC Release x64 VST3 and Standalone: passed. All **nine** CTest suites passed.
- pluginval 1.0.4 strictness **5: SUCCESS** for both the build output and the installed plugin at **Common Files/VST3/RaBubanj.vst3**.
- PowerShell installer script syntax checked on Windows before building.
- Inno Setup compiler-engine banner reports **6.7.1**. Its Windows ProductVersion metadata is `0.0.0.0`, which is the field recorded in the installer ZIP's BUILD-INFO sidecar; use the CI compiler-engine banner for the actual compiler version. The compiler banner also states "Non-commercial use only"; tooling terms are separate from the application's GPL license.

## Actual install, repair and uninstall

All native Windows smoke checks passed in a disposable GitHub Actions runner:

1. Fresh **VST3 only** install: plugin bundle installed, Standalone executable/launch shortcut absent; uninstall passed.
2. Fresh **Standalone only** install: executable and correct Start menu shortcut installed, plugin absent; uninstall passed.
3. Full install: both components, correct version/publisher registry entry and one uninstaller.
4. Full reinstall/repair restored deliberately modified installed EXE and VST3 bytes to the validated payload hashes, retaining an untracked user-data file.
5. Installed plugin passed pluginval strictness 5 at its real system Common Files path, including spaces in the path.
6. Installed Standalone started, became responsive and created a visible window belonging to its own process; the test then closed it. The smoke harness waits for asynchronous startup rather than immediately relying on Process.MainWindowHandle.
7. Full uninstall removed registered installed files, plugin bundle, Start menu group and Windows uninstall entry, while the untracked **UserData/keep-me.txt** marker retained its hash.

Payload hashes were checked for both binaries, every VST3 bundle file, licenses, instructions and both source archives in all applicable install modes. The test overrides the application folder to a unique runner temp path; VST3 uses its real system location. Existing installations/paths are refused by the test.

## Installer behavior

- Per-machine Windows 10/11 x64 installation, administrator privileges, stable AppId, VST3/Standalone component selection, Start menu integration and optional desktop shortcut. Run-after-install is optional and skipped in silent tests.
- Standard VST3 folder: `C:\Program Files\Common Files\VST3\RaBubanj.vst3`; default application folder: `C:\Program Files\Voyagersquare\RaBubanj`.
- Installed components are removed through Windows Apps/Uninstall. Selecting components on a later setup run installs/refreshes those components; removing an existing component requires uninstall then reinstall of the desired selection.
- Exact RaBubanj source ZIP, corresponding JUCE source and licenses are included for every component selection. No runtime download is performed. No wildcard uninstall deletion of user-data folders is configured.
- App/installer are not Authenticode-signed; no signing certificate has been supplied. Native interactive wizard/UAC click-through, desktop shortcut selection and real audio/MIDI hardware playback are not automated here.

## Download verification

- Installer ZIP: **66,113,727 bytes**, 4 files, SHA256 matches the GitHub digest; ZIP integrity passed.
- Inner Setup EXE: **66,659,365 bytes**, PE signature verified. Its SHA256 matches the installer ZIP's SHA256SUMS.txt. BUILD-INFO names the tested source commit and package version **0.16.1**.
- Portable ZIP: **68,997,052 bytes**, 14 files, integrity and GitHub SHA256 passed. Both payload PEs are AMD64; VST3 manifest is **0.16.1**.
- All **39** bundled Source/test/Installer/CMake/workflow files match the tested commit after line-ending normalization. Original wood and embedded font bytes are verified in both binaries and the bundled source.
- Audio/UI source is unchanged from 0.16.0; this patch adds packaging and installer tests. [Previous functional validation](docs/VALIDATION-0.16.md).

```text
282d291e5ba5fbe6a4a0278076f7c6b9116d6d5565cf3df15e66d5f265e6efdb  RaBubanj-0.16.1-Windows-x64-Setup.zip
a36a2b942f4a64b572612af1516427e3a1186eed272ad856a9aec0b0c39c112f  RaBubanj-0.16.1-Setup-x64.exe
62c33e39e757a4a167821b15e0a904649de796c684c9a5889e760222bdbad1e5  RaBubanj-0.16.1-Windows-x64.zip
```

Artifacts are retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**. [Installer instructions](docs/INSTALLER-HR.md).
