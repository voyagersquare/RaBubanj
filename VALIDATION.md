# RaBubanj 0.2.1 validation

Verified 17 November 2026. Tested source: `88b4fcbf5f8ecbdd862cd6d284423a0cb8db4557`.

[Successful native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44159736139) · [Windows ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44159736139/artifacts/14362353517) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44159736139/artifacts/14362732857)

## Changes

- Raised, bevelled dark-blue panels, tactile buttons, metal faders and recessed EQ displays. The cached original finish depicts large corroded patches, exposed metal, scratches, chipped paint and dust, using the supplied example as a visual reference.
- Reverb Size, Damping and Width are rotary controls in the left half of the split side panel. The right half has no controls or captions.
- Removed the header brand and mixer/version captions, plus the post-fader routing explanation underneath the Reverb controls.
- Small functional captions have a contrasting outline; dark EQ-node numbers retain clean edges.
- DSP, routing, parameter IDs and the saved project schema remain compatible with 0.2.

## Verification

- Native MSVC Release builds of Windows x64 VST3 and standalone: passed.
- All four native Windows CTest suites: **passed**, zero failures. Covers the drum engine, parametric EQ, processor/MIDI/state and mixer routing/editor interactions.
- pluginval 1.0.4, strictness 5: **SUCCESS**, with process completion and exit status checked before packaging. The optional separate Steinberg SDK validator was not configured and was skipped.
- Local Linux Release VST3/standalone and all four CTest suites: passed.
- Actual editor snapshots rendered at 1120 × 740, 1280 × 820 and 1800 × 1160. Existing bounds checks pass; minimum and default views were visually inspected for readable controls, round Reverb knobs and the empty right panel. [Preview](docs/preview.png).
- Final ZIP downloaded and checked for archive integrity, expected payloads and the 0.2.1 VST3 manifest: passed. Eleven files, 29,570,251 bytes. The package includes the full VST3 folder, standalone EXE, Croatian instructions, source and licenses.
- The downloaded SHA256 matches GitHub's artifact digest.

Actual Ableton sessions and physical Windows audio/MIDI devices have not been exercised here. Earlier DSP and compatibility checks are documented in the [0.2 validation record](docs/VALIDATION-0.2.md).

## Package checksum

```text
760fcd54446e0036d7aabe1750eb43921afdb7fcc1e61f2975886c933f01a6be  RaBubanj-0.2.1-Windows-x64.zip
```

Actions artifacts are retained for 30 days and can require GitHub sign-in. Manual rebuild: **Actions → Windows VST3 → Run workflow**.
