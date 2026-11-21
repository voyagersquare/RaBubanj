# Original wood texture

`wood-reference.jpg` is the unmodified JPEG extracted from the user's `wood texture.rar` attachment, originally named `natural-light-brown-wood-grain-texture.jpg`.

- Dimensions: 7680 × 4302; progressive JPEG, RGB.
- Size: 9,501,641 bytes.
- SHA256: `5f664206eec18dae81a2e6d4ca04dc0be0967afd1d981d44fdf7c87650f7cca5`.

CMake embeds the original bytes. The image is decoded once on the GUI thread, shared across editor instances, and drawn with uniform scaling/cropping. A 20% translucent red-brown finish is applied at paint time to make the displayed wood slightly darker and warmer; it does not modify the JPEG. Relief lighting is rendered separately. Metal controls do not use the wood material. No generated substitute is included.
