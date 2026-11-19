# RaBubanj 0.5 validation

Verified 19 November 2026. Tested source: `035a5b39aebaf74c024b36d9a5f560037dbb7c0a`.

[Native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44499031182) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44499031182/artifacts/14517505191) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44499031182/artifacts/14516632661)

## Results

- Native MSVC Release VST3 and standalone: passed. All **seven** CTest suites passed, zero failures.
- pluginval 1.0.4 strictness **5: SUCCESS**. Its process is explicitly awaited before packaging. The optional separate Steinberg SDK validator was not configured and was skipped.
- Downloaded ZIP integrity and SHA256 match GitHub's artifact digest. VST3 and standalone PE machine types are AMD64, and the VST3 manifest reports 0.5.0. All bundled project source files match the tested commit after Windows line-ending normalization. Corresponding JUCE source and licenses are included.
- Portable Windows ZIP: 30,101,768 bytes. No installer and no factory-preview artifact are produced for 0.5.
- Linux Release builds and the same seven suites pass, with targeted reruns after the final UI assertions. The new processing/spectrum suite passes with project sources under AddressSanitizer and UndefinedBehaviorSanitizer, linked against the native JUCE library.

## Channel controls and audio

- Channel-name clicks select the EQ without triggering. Headers receive live post-fader level values. All eight bottom PLAY controls trigger their current source exactly once and illuminate yellow, including a short pulse for quick clicks; illumination expires after release.
- Factory menus/arrows and the public preset-selection API are removed. Legacy source parameter IDs and recipes are retained for old-project recall. Imported filenames, per-channel file drops and embedded sample state remain covered by the import suite.
- Pitch/Drive/Comp knobs rebind to the selected drum, retain separate channel values and recall through state. Drum-only processing controls are disabled for Reverb. Old states with absent new parameters restore neutral settings.
- At zero Drive/Comp, the new processing is bit-exact neutral at 44.1/48/96 kHz. Saturation adds measured third-harmonic content and stays finite/bounded. First-order antiderivative antialiasing softens the nonlinear high end.
- A steady −6.02 dBFS input yields approximately 7.99 dB gain reduction at full compression, matching the fixed −18 dBFS threshold and 3:1 ratio. Stereo balance is retained; release decays gradually. Settings: 6 dB knee, 10 ms attack, 150–350 ms programme-dependent release, no auto makeup; knob strength blends the compressed signal.
- Exact custom-source interpolation verifies that +12 semitones doubles playback rate. Changes during playing notes stay finite, and pitched voices release their sources correctly. Range is ±24 semitones with a 5 ms pitch glide for existing voices; new voices start at the target.
- Signal path is Pitch → Drive → Compressor → channel EQ → level/pan, with the existing post-fader sends, wet return and master processing. MIDI C1–G1, legacy GM, sample offsets, panic, hat choke, state migration, mute/solo and independent EQ routing continue to pass.

A local 512-sample-block stress benchmark with all eight Drive/Comp controls at 100% processed 12.8 seconds of 48 kHz audio in 1.29 seconds, and 6.4 seconds of 96 kHz audio in 1.31 seconds. These are measurements on this Linux environment with the GUI closed.

## Spectrum and UI

- Both EQ graphs render a live, stereo-aware spectrum under the response curve. Channel tap is post-EQ/pre-fader; Reverb tap is after its return level; master tap is final stereo output.
- A calibrated 996.09 Hz, −6.02 dBFS test tone is detected at the correct FFT bin and amplitude. Opposite-phase stereo does not cancel the display, silence decays below −80 dBFS, and channel/master taps carry actual processor audio.
- FFT size 4096, Hann window, at least 1024 new samples per update. Spectrum uses the right-side −90 to 0 dBFS scale; the left-side ±24 dB scale remains EQ gain. FFTs run on the GUI thread; the audio producer writes only to fixed-capacity SPSC buffers for visible buses. No audio latency is added by analysis.
- Concurrent producer/consumer buffer activity passes the new suite and sanitizer run. Source switching/recall stress remains covered separately.
- Bounds checks and actual editor snapshots pass at 1120×740, 1280×820 and 1800×1160; minimum/default views were visually inspected, including live spectra, header levels, yellow PLAY and the three new rotary controls. The worn finish and existing Reverb arrangement remain. [Preview](preview.png).

Actual Ableton sessions, physical MIDI/audio hardware and native host keyboard forwarding have not been exercised here. Automated tests cover plugin-side incoming MIDI, key handling, file/URI targets, playback and state recall.

## Checksum

```text
76a5778731ed4275a43e6d7af3c8ddc0e6e22935ba8ee11be78cdfb735bd1bb6  RaBubanj-0.5.0-Windows-x64.zip
```

Artifacts are retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**. [Previous validation](VALIDATION-0.4.md).
