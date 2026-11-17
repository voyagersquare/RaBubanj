# RaBubanj 0.2

Windows x64 VST3 drum instrument by **Voyagersquare**, with a compact studio mixer, independent parametric EQs and a shared stereo Reverb bus.

**Windows ZIP without an installer:** download `RaBubanj-0.2.0-Windows-x64` from a successful **Actions → Windows VST3** run. [Windows instructions in Croatian](docs/WINDOWS-HR.md) · [Validation](VALIDATION.md)

Extract the ZIP, copy the entire `RaBubanj.vst3` folder into `C:\Program Files\Common Files\VST3\`, then rescan instruments in your DAW. The included `RaBubanj.exe` is a standalone application, not an installer.

![RaBubanj 0.2 studio mixer, with an example Kick EQ edit](docs/preview.png)

## New in 0.2

- Compact grey mixer with cyan channel headers, silver faders, channel meters, pan, mute, solo and small HIT preview buttons. Eight drum strips, one Reverb return and a stereo master leave room for further mixer controls.
- Each drum has its own **post-fader Reverb Send next to the level fader**. Send values are displayed as percentages; the Reverb return has its own dB fader, mute and wet-only solo.
- Click a channel name, its EQ button or its controls to display that channel's EQ in the large upper panel. Reverb has its own return EQ. The independent global master EQ stays visible on the upper right.
- **Six parametric bands per EQ**, each with frequency (20 Hz–20 kHz), gain (±24 dB), Q (0.15–18), type and enable. Types: Bell, Low shelf, High shelf, Low cut, High cut and Notch. Cuts are 12 dB/octave. Gain is disabled for cuts and notch.
- Drag graph nodes for frequency/gain, scroll over a node for Q, and double-click a node to reset its gain. Band buttons and numeric controls provide precise editing. Each EQ has bypass and reset. The response graph uses the same filter formulas as the audio processing.
- Reverb Size, Damping and Width controls. All mixer, send, return and EQ parameters support DAW automation and project recall. The selected channel is remembered.
- Retains plugin identity and the existing level/pan/mute/master/room parameter IDs. Version 0.1 project states load with flat EQs and unity sends, preserving their original shared room feed and dry calibration. New kits start with 25% sends and the original 0.12 linear return level (about -18.4 dB).

### Signal path

```text
Drum → channel EQ → level / pan ──────────────→ dry mix ─┐
                         └→ post-fader Send → Reverb    │
                            → return EQ → return fader ├→ global EQ → master → stereo out
                                                       ┘
```

The Reverb return is wet-only. A drum mute also mutes its send; previously generated reverb tails decay naturally. Reverb mute or its return fader controls the entire return. Solo on Reverb auditions the wet return without the dry mix. The Reverb bus is internal; the VST3 still exposes one stereo output.

## Drum engine

Eight original generated starter instruments: kick, snare, closed/open hi-hat, low/high tom, crash and ride. Each has three velocity layers and three round-robin variations, with 64 shared voices and hi-hat choke. MIDI note-ons use their audio-buffer timestamps. Note-offs do not cut one-shots; MIDI CC120/123 resets voices, EQ state and the reverb at its exact event offset.

The starter kit remains synthetic. Recorded acoustic libraries, WAV import, multi-microphone routing, grooves and an internal sequencer are future work. The EQ is a six-band minimum-phase processor with a frequency-response graph; it does not implement dynamic EQ, linear-phase processing or a spectrum analyser. The master retains gentle saturation for stacked hits.

## MIDI map

| Instrument | Main note | Accepted aliases |
| --- | ---: | --- |
| Kick | 36 | 35 |
| Snare | 38 | 37, 40 |
| Closed hi-hat | 42 | 44 |
| Open hi-hat | 46 | — |
| Low tom | 45 | 41, 43 |
| High tom | 48 | 47, 50 |
| Crash | 49 | 52, 55, 57 |
| Ride | 51 | 53, 59 |

Aliases share the corresponding sound; they are not independent articulations. MIDI note numbers are authoritative because octave labels differ between hosts.

## Build on Windows

Use Visual Studio 2022 with **Desktop development with C++**, Windows SDK, CMake 3.22+ and Git. Internet access is needed for the first JUCE fetch; its revision is pinned in CMake.

```powershell
cmake -S . -B build -A x64 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Outputs:

```text
build/RaBubanj_artefacts/Release/VST3/RaBubanj.vst3/
build/RaBubanj_artefacts/Release/Standalone/RaBubanj.exe
```

GitHub Actions builds both targets, runs the DSP tests, and validates the VST3 with pluginval at strictness 5 before uploading the portable package. Binaries are unsigned. See [Croatian Windows instructions](docs/WINDOWS-HR.md) and [development roadmap](docs/ROADMAP.md).

## Tests

```sh
cmake -S . -B build-tests -DRABUBANJ_BUILD_PLUGIN=OFF
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

A full plugin build also runs processor and mixer/editor integration suites. They check send routing and fader/mute/solo interaction, independent channel/return/master EQ audio, state recall and 0.1 migration, channel selection, graph dragging/wheel/reset, parameter attachments and control bounds at 1120 × 740, 1280 × 820 and 1800 × 1160. `mixer_tests <absolute-directory>` additionally renders UI previews. pluginval validates the native Windows VST3 before the workflow uploads its ZIP.

## License

The project is GPL-3.0-only; see [LICENSE](LICENSE) and [THIRD_PARTY.md](THIRD_PARTY.md). No proprietary samples, graphics, product names or source code from other drum instruments are bundled. If a closed-source commercial release is desired later, review JUCE commercial licensing and the rights to all contributed code and sample content before changing distribution terms.
