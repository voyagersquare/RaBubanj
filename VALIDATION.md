# RaBubanj 0.3 validation

Verified 17 November 2026. Tested source: `11de82443b684a30a6c66bce38805369e7e5aa92`.

[Successful native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44171520880) · [Windows ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44171520880/artifacts/14367593804) · [80 WAV previews](https://github.com/voyagersquare/RaBubanj/actions/runs/44171520880/artifacts/14367948141) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44171520880/artifacts/14367324082)

## Windows results

- Native MSVC Release VST3 and standalone builds: passed.
- All **five** CTest suites: passed, zero failures. Drum engine, parametric EQ, factory presets, processor, and mixer/editor integration.
- pluginval 1.0.4 strictness 5: **SUCCESS**, with its process explicitly awaited before packaging. The optional separate Steinberg SDK validator was not configured and was skipped.
- Native factory exporter produced all 80 dry previews successfully.
- Both downloaded archives passed ZIP integrity and SHA256 checks against GitHub artifact digests.
- Windows ZIP: 29,532,353 bytes; expected VST3 bundle, manifest version 0.3.0, standalone, instructions, corresponding source and licenses verified.
- Factory audio ZIP: 4,603,197 bytes; all 80 WAVs verified as mono 32 kHz, 16-bit PCM with nonempty audio.

## Factory and compatibility coverage

- All 80 choices render audible, distinct, finite, bounded audio at 44.1/48/96 kHz. Each retains velocity dynamics, and each one-shot ends within four seconds.
- Closed-hat presets choke open-hat presets. Switching a preset leaves an already-playing tail bit-identical to an unchanged reference voice; future triggers use the new source. Invalid indices clamp safely.
- Factory recipes have three velocity layers and three round robins. The 72 new choices share a 32 kHz immutable cache; slot 01 uses the original native-rate generator. Shared new PCM storage is approximately 106 MiB and is created outside the audio callback.
- Choices recall independently through DAW project state. Pre-0.3 state restores slot 01; 0.2 sends and EQ edits are retained. The existing 0.1 room-feed migration still passes.
- Channel-name clicks select the EQ and enqueue exactly one preview hit. Factory selection updates only its channel parameter without an unsolicited hit. All eight selectors expose ten choices.
- Duplicate per-strip EQ/HIT buttons, EQ number selectors, numeric EQ sliders and Band On checkbox are absent. Every graph node selects its band; drag, wheel, double-click and right-click perform the retained EQ actions.
- Existing send, fader, pan, mute/solo, Reverb/master EQ, MIDI timing, panic and state tests continue to pass.

## Local and visual checks

- Linux Release VST3/standalone and the same five suites pass. The UI suite was rerun after correcting a test that accidentally mistook the Open Hat control names for obsolete Open-EQ buttons.
- Factory tests under AddressSanitizer and UndefinedBehaviorSanitizer pass, including all presets, resampling and preserved playing-source pointers.
- Rendered the actual JUCE editor at minimum/default/maximum sizes; bounds checks pass and minimum/default views were visually inspected. The worn finish, layout, Reverb knobs and reserved panel remain, with selectors replacing HIT. [Preview](docs/preview.png).
- Local 80 WAV previews were checked for format, audible RMS and absence of full-scale PCM clipping; combined duration approximately 107 seconds.

The presets are original analogue-style digital synthesis informed by the [research references](docs/FACTORY-PRESETS.md). No third-party sample library is bundled. Actual Ableton sessions and physical Windows MIDI/audio hardware have not been exercised here.

## Checksums

```text
555d62429a92b71d504be215cc52a1582a0af82607170342232df97c0cb401fd  RaBubanj-0.3.0-Windows-x64.zip
90eadd772fd308bdc267edba42b7afb6b34c7a30973b13756db86410309348e3  RaBubanj-0.3.0-Factory-Audio.zip
```

Artifacts are retained for 30 days and can require GitHub sign-in. Manual rebuild: **Actions → Windows VST3 → Run workflow**. [Previous validation](docs/VALIDATION-0.2.1.md).
