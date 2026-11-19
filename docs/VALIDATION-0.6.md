# RaBubanj 0.6 validation

Verified 19 November 2026. Tested source: `aa6640eca343a14b7f6fe0c13f1d72ad55de4dec`.

[Native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44504345108) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44504345108/artifacts/14518203461) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44504345108/artifacts/14519620564)

## Build and package

- Native MSVC Release x64 VST3 and standalone: passed. All **seven** CTest suites passed, zero failures.
- pluginval 1.0.4 strictness **5: SUCCESS**. The validator process is explicitly awaited before packaging. The optional separate Steinberg SDK validator was not configured and was skipped.
- Downloaded ZIP integrity and SHA256 match GitHub's artifact digest. Both PE binaries are AMD64; the VST3 manifest reports 0.6.0. All bundled project source files match the tested commit after line-ending normalization. Corresponding JUCE source and licenses are included.
- Portable Windows archive: 29,098,416 bytes, with no installer.
- Linux Release VST3/standalone and the same seven suites pass; the UI suite was rerun after final fader geometry and right-double-click handling changes.

## EQ node menus

- Both channel and Master EQ filter dropdowns are absent. Right-clicking a node opens the themed context menu, rather than changing its enabled state immediately. Right-clicking outside a node does not open a different band's menu.
- Menus contain all six **Curve shape** choices with response icons and a current-choice check mark: Bell, Low shelf, High shelf, Low cut, High cut and Notch.
- **Gain (dB)** offers −24/−18/−12/−6/−3/0/+3/+6/+12/+18/+24 dB. Invert gain and gain choices are disabled for cut/notch; the action handler also rejects those gain actions while such a filter is selected.
- Band enabled toggles the clicked band; Alt-click retains direct toggling. Double-click resets gain; right-double-click cannot reset it. Existing drag, wheel, bypass and reset remain covered.
- Tests verify all shapes and gain choices, menu inversion, enable/disable, channel/Master isolation, retaining the original bus/band when selection changes, and dismissal without parameter changes.
- Async callbacks use a safe component reference, retain the clicked bus/band, and keep the menu look-and-feel alive through completion. Commands write the existing parameter IDs with host gestures, so saved state and automation formats remain compatible.

## Visual checks

- The procedural rust/scratch texture and its cached image generator are removed. Main surfaces use solid ochre fills with bevel/shadow depth at panel edges.
- Command legends are white. EQ scales and the main response curve use warm gold tones; spectrum displays retain their stereo-aware analysis. Context menus have a dark brown background, warm text, ochre highlights and curve icons.
- Rotary knobs are drawn as ribbed light-plastic bodies with coloured top caps and position marks, based on the supplied studio-console reference. Dark fader caps with white reference stripes and raised switches complete the console styling. No manufacturer graphics or logos are embedded.
- EQ panels use deeper bevels and shadows; the graph gains the space formerly occupied by the dropdown.
- Existing bounds checks pass at 1120×740, 1280×820 and 1800×1160. Actual JUCE editor snapshots were rendered at each size, and minimum/default views were visually inspected. [UI preview](preview.png).
- [Menu styling preview](eq-menu.png) is rendered from the actual menu items and look-and-feel; it is a styling reference rather than an OS-window screenshot. Its dark background is checked during export.

The audio DSP, per-channel Pitch/Drive/Comp, sample loading and embedded recall, MIDI mapping, metering and analysis algorithms retain their 0.5 implementation. Existing engine, legacy-source, EQ, processor, sample-import and channel-colour/spectrum suites continue to pass. Actual Ableton sessions, native popup placement in a Windows host, and physical MIDI/audio hardware have not been exercised here.

## Checksum

```text
2c8c0ab14d7454c5bfc34d2d920e557cffedf36042abaebef1dff71eb77d8991  RaBubanj-0.6.0-Windows-x64.zip
```

Artifacts are retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**. [Previous validation](VALIDATION-0.5.md).
