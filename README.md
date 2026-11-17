# RaBubanj 0.4.0

Windows x64 VST3 drum instrument by **Voyagersquare**, with a compact studio mixer, independent parametric EQs and a shared stereo Reverb bus.

**[Preuzmi RaBubanj 0.4 za Windows x64 — ZIP bez instalera](https://github.com/voyagersquare/RaBubanj/actions/runs/44183161228/artifacts/14373781111)**

**[Preuzmi 160 suhih WAV primjera](https://github.com/voyagersquare/RaBubanj/actions/runs/44183161228/artifacts/14371589929)** — izborni paket za preslušavanje. Windows build, svih šest skupova testova i pluginval strictness 5 prolaze.

 [Windows instructions in Croatian](docs/WINDOWS-HR.md) · [Validation](VALIDATION.md)

Extract the ZIP, copy the entire `RaBubanj.vst3` folder into `C:\Program Files\Common Files\VST3\`, then rescan instruments in your DAW. The included `RaBubanj.exe` is a standalone application, not an installer.

![RaBubanj 0.4.0 worn dark-blue mixer, with an example Kick EQ edit](docs/preview.png)

## New in 0.4

- **SEND defaults and double-click reset are now 0%.** Older 25% defaults migrate to zero; other saved Send levels are preserved.
- **20 original boom-bap factory sounds per drum, 160 total.** Left/right arrows replace the dropdowns and wrap through all choices. Refined snares have shell and wire layers; hats and cymbals use denser, decaying inharmonic modes and filtered noise, with smoother attacks and less harsh coloration. [Catalogue and references](docs/FACTORY-PRESETS.md).
- **Drop an audio file onto any drum strip** to replace only that channel. Mono and stereo WAV/AIFF/FLAC/Ogg files load in the background. The filename appears below the channel name; **x** restores the current factory source. The preset arrows return to factory sounds. Imported audio is embedded in saved projects.
- **Chromatic MIDI C1–G1 (36–43)** triggers Kick through Ride in order, including black keys. The header's MIDI button can select the old GM mapping for existing clips.
- **KEYS: DAW** passes computer keys to the host for keyboard MIDI and recording. **KEYS: PLUGIN** auditions A W S E D F T G with this window focused, suppressing key repeats. During host recording it forwards keys to the DAW. Incoming MIDI works in either mode.
- The worn dark-blue finish, compact mixer, raised controls, graph EQs, rotary Reverb controls and reserved right panel remain.

Click a channel **name** to select its EQ and play its current source once. Other channel clicks only select its EQ. Preset arrows do not audition or change mixer/EQ settings; existing notes keep their original tails.

## Core mixer

- Compact mixer with cyan channel headers, silver faders, channel meters, pan, mute, solo, channel-name audition and factory selectors. Eight drum strips, one Reverb return and a stereo master leave room for further mixer controls.
- Each drum has its own **post-fader Reverb Send next to the level fader**. Send values are displayed as percentages; the Reverb return has its own dB fader, mute and wet-only solo.
- Click a channel name or its controls to display that channel's EQ in the large upper panel. Reverb has its own return EQ. The independent global master EQ stays visible on the upper right.
- **Six parametric bands per EQ**, each with frequency (20 Hz–20 kHz), gain (±24 dB), Q (0.15–18), type and enable. Types: Bell, Low shelf, High shelf, Low cut, High cut and Notch. Cuts are 12 dB/octave. Gain is disabled for cuts and notch.
- Drag graph nodes for frequency/gain, scroll over a node for Q, and double-click a node to reset its gain. Graph nodes select bands and expose value tooltips. Right-click toggles a band. Each EQ has bypass and reset. The response graph uses the same filter formulas as the audio processing.
- Reverb Size, Damping and Width controls. All mixer, send, return and EQ parameters support DAW automation and project recall. The selected channel is remembered.
- Retains plugin identity and the existing level/pan/mute/master/room parameter IDs. Version 0.1 project states load with flat EQs and unity sends, preserving their original shared room feed and dry calibration. New kits start with 0% sends and the original 0.12 linear return level (about -18.4 dB).

### Signal path

```text
Drum → channel EQ → level / pan ──────────────→ dry mix ─┐
                         └→ post-fader Send → Reverb    │
                            → return EQ → return fader ├→ global EQ → master → stereo out
                                                       ┘
```

The Reverb return is wet-only. A drum mute also mutes its send; previously generated reverb tails decay naturally. Reverb mute or its return fader controls the entire return. Solo on Reverb auditions the wet return without the dry mix. The Reverb bus is internal; the VST3 still exposes one stereo output.

## Drum engine

Eight original generated instruments, with twenty choices each: kick, snare, closed/open hi-hat, low/high tom, crash and ride. Each has three velocity layers and three round-robin variations, with 64 shared voices and hi-hat choke. MIDI note-ons use their audio-buffer timestamps. Note-offs do not cut one-shots; MIDI CC120/123 resets voices, EQ state and the reverb at its exact event offset.

The starter kit remains synthetic. Recorded acoustic libraries, multi-microphone routing, grooves and an internal sequencer are future work. Imported files play as velocity-scaled one-shots; factory sounds provide velocity layers and round robins. The EQ is a six-band minimum-phase processor with a frequency-response graph; it does not implement dynamic EQ, linear-phase processing or a spectrum analyser. The master retains gentle saturation for stacked hits.

## MIDI and computer keyboard

| Instrument | Ableton note | MIDI number | PLUGIN audition key |
| --- | --- | ---: | --- |
| Kick | C1 | 36 | A |
| Snare | C#1 | 37 | W |
| Closed hi-hat | D1 | 38 | S |
| Open hi-hat | D#1 | 39 | E |
| Low tom | E1 | 40 | D |
| High tom | F1 | 41 | F |
| Crash | F#1 | 42 | T |
| Ride | G1 | 43 | G |

All MIDI channels are accepted. Octave labels vary between hosts; MIDI numbers are authoritative. In Ableton, enable the computer MIDI keyboard, arm/monitor the instrument track and set its keyboard octave so the first key sends C1 (36). Use **KEYS: DAW** when recording; local PLUGIN audition does not write MIDI notes into the host. Text editing and Ctrl/Cmd/Alt shortcuts are not intercepted.

**MIDI: GM** restores main notes 36/38/42/46/45/48/49/51 and the original aliases. Default mapping is chromatic even when opening older projects, so select GM explicitly for old GM clips.

## Sample import

Drop one file from the DAW browser or the operating system onto the target drum channel. Standard file paths and `file://` URI payloads are accepted. Proprietary clips and internal browser objects that expose no local audio file cannot be decoded; export/consolidate them to an audio file first. Files may be mono or stereo, 8–384 kHz, up to 30 seconds, with a 32 MB encoded-file and decoded-PCM limit. Playback follows the host sample rate and preserves stereo.

Imports retain the channel's level, pan, send and EQ. Failed new imports leave the previous sound intact; the row shows the error in its tooltip. The **x** button cancels loading or clears the custom source. Source data is embedded in the DAW state for portable recall, increasing project size. If restored data is missing/corrupt, that channel stays silent and shows an error until a replacement is dropped or **x** returns to the factory source. Samples load asynchronously; wait for the filename before playing after a project opens. Stereo files use the channel pan as left/right balance.

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

A full plugin build also runs processor, mixer/editor and sample import/recall integration suites. They check file/URI drops, stereo resampling, embedded project recall after deleting original files, failed imports, concurrent source changes and cancellation, chromatic/GM MIDI, keyboard repeat suppression, factory arrows and state, channel-name audition and graph-only EQ editing, as well as send routing and fader/mute/solo interaction, independent channel/return/master EQ audio, state recall and 0.1 migration, channel selection, graph dragging/wheel/reset, parameter attachments and control bounds at 1120 × 740, 1280 × 820 and 1800 × 1160. `mixer_tests <absolute-directory>` additionally renders UI previews; `factory_preset_tests <absolute-directory>` exports the 160 dry WAV previews. pluginval validates the native Windows VST3 before the workflow uploads its ZIP.

## License

The project is GPL-3.0-only; see [LICENSE](LICENSE) and [THIRD_PARTY.md](THIRD_PARTY.md). No proprietary samples, graphics, product names or source code from other drum instruments are bundled. If a closed-source commercial release is desired later, review JUCE commercial licensing and the rights to all contributed code and sample content before changing distribution terms.
