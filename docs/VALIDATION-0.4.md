# RaBubanj 0.4 validation

Verified 17 November 2026. Tested source: `bd0326a8f11bb8a3c7e9dad4a14632a20cc03a9b`.

[Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44183161228) · [Windows ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44183161228/artifacts/14373781111) · [160 dry WAVs](https://github.com/voyagersquare/RaBubanj/actions/runs/44183161228/artifacts/14371589929) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44183161228/artifacts/14372468972)

## Native Windows

- MSVC Release x64 VST3 and standalone: passed.
- All **six CTest suites**: passed, zero failures. Drum engine, parametric EQ, factory presets, processor MIDI/state, mixer/editor, and sample import/recall.
- pluginval 1.0.4 at strictness **5**: **SUCCESS**. The validator process is awaited before packaging. The optional separate Steinberg SDK validator was not configured and was skipped.
- Native exporter produced 160 dry WAV previews.
- Both downloaded archives passed ZIP integrity and SHA256 verification against GitHub artifact digests. VST3 and standalone PE headers are AMD64; the VST3 manifest is version 0.4.0. Bundled project sources match the tested commit, and corresponding JUCE source and licenses are present.
- Windows archive: 29,996,880 bytes. Factory audio archive: 10,021,582 bytes, 160 mono 32 kHz / 16-bit PCM files, approximately 213.82 seconds total. Every WAV has audible RMS with no full-scale PCM clipping.

## Input, presets and UI

- Chromatic 36–43 triggers Kick, Snare, Closed Hat, Open Hat, Low Tom, High Tom, Crash and Ride on all MIDI channels. Sample-offset timing and CC120/123 panic remain covered. Optional GM mapping retains the original aliases.
- DAW keyboard mode leaves keystrokes unhandled for the host. Local A/W/S/E/D/F/T/G audition triggers once per held key, passes shortcuts/text editing through, and forwards keys during host recording. Keyboard mode recalls in state.
- All 160 factory choices render audible, distinct, finite, bounded, velocity-sensitive audio at 44.1/48/96 kHz, with tails finishing within four seconds. Hat choke, source-rate interpolation and preservation of already-playing factory notes pass.
- Shared new factory PCM: 233,479,332 bytes (approximately 223 MiB), with three velocity layers and three round robins. Slot 01 retains native-rate legacy generation. The factory bank is created outside processing and shared between instances.
- Arrows reach all 20 sounds and wrap both directions without unsolicited hits. Old 0–9 preset parameter domains are retained; an additional bank flag selects sounds 11–20, preserving old normalized automation values.
- Send defaults and double-click reset are zero. Pre-0.4 saved values equal to the former 25% default migrate to zero; other custom sends and EQ edits are retained. The original 0.1 unity-send migration remains covered.
- Channel-name audition, graph EQ controls, independent channel/return/master audio routing, mute/solo and state recall remain covered. Dropdowns and the earlier duplicate EQ/HIT controls are absent.
- Actual JUCE editor snapshots and bounds checks at 1120×740, 1280×820 and 1800×1160 pass. Minimum and default views were visually inspected. [Preview](preview.png).

## Sample import and lifetime

- Per-drum local file and file-URI drops target only their own channel. Multiple-file and non-file payloads are rejected, as are drops onto Reverb/Master.
- Mono/stereo WAV imports at 32 kHz play in a 48 kHz host. A separate exact-interpolation test checks source/host rate conversion and both stereo channels. Imported voices retain their original source while it is replaced; completed voices release it.
- Imported original bytes are embedded in project state. Restored audio matches after original files are deleted. Factory arrows/clear affect only the chosen channel. Failed new imports preserve existing audio; missing recalled content stays explicitly silent until repaired or cleared.
- Invalid audio, missing files, files larger than 32 MB and files longer than 30 seconds are rejected. Decoded PCM is limited to 32 MB, stereo and supported sample rates.
- Background loads, clears and stale requests are exercised while a concurrent audio thread plays and steals voices. Clear invalidates pending loads. Decoding, publication and reclamation take place on the worker; processing uses a protected immutable snapshot and atomic voice references.
- Factory tests pass under AddressSanitizer and UndefinedBehaviorSanitizer. Project processor/editor/loader sources also pass the imported-audio stress and recall tests with both sanitizers, linked against the native JUCE library.

The same six suites pass locally on Linux, with targeted reruns after the final source-lifetime and host-state notification changes. Presets are original analogue-style synthesis informed by the [documented references](FACTORY-PRESETS.md). Actual Ableton browser drag payloads, native host keyboard forwarding and physical Windows MIDI/audio hardware have not been exercised here; the automated tests cover the plugin's file targets, key handlers and incoming MIDI processing.

## Checksums

```text
d2934c41ff275265cec457d5197a1e3fd30a8a4756c7f380f269e93fcc9fbb5c  RaBubanj-0.4.0-Windows-x64.zip
1622f851f7745f9f54db2bffdfc0dd4b7230406615b8209841f36bb093033f6e  RaBubanj-0.4.0-Factory-Audio.zip
```

Artifacts are retained for 30 days and can require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**. [Previous validation](VALIDATION-0.3.md).
