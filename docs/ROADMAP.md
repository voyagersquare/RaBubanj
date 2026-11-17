# Development roadmap

## 0.1 — playable foundation

Windows x64 VST3 + standalone, original generated starter kit, GM MIDI, layered round robin playback, channel mixer, room, state recall and portable build pipeline.

## Next: acoustic sample engine

- Load a documented kit manifest with licensed WAV samples, velocity zones, independent round-robin pools, tuning and per-articulation envelopes.
- Decode files away from the audio callback and swap complete immutable kits at an audio-safe boundary.
- Add loading progress, missing-file errors, library path management and user kit recall.
- Keep initial built-in samples available as a fallback.

## Then: detailed kit and mixing

- Independent kick/snare articulations, hi-hat openness via CC4 and e-drum mappings.
- Close, overhead and room microphone buses with phase-safe routing and multi-output support.
- Per-channel EQ/compression, solo, presets and a realistic multi-sampled acoustic kit.

## Then: grooves and release polish

- Licensed MIDI grooves, host transport sync and MIDI drag-and-drop.
- Audition representative DAWs on real Windows audio hardware, including device changes and project reloads.
- Signing and a versioned release download; an installer is deliberately outside the initial scope.

Reference products inform the workflow only. Artwork, sample recordings and implementation remain original or appropriately licensed.
