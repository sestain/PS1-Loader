# PS1 Loader

## Description

- I wanted to remake a bootleg crash ps1 collection (that uses modern SDK - [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK)).
- Tested on PCSX-Redux (13075.20230113.7.x64 - 2023-01-13 10:45:48) and Duckstation 0.1-5477-gb5f806a8 (dev)

## Compiling and usage

1. To build this (tested only on Windows), you'll need the [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK/blob/master/doc/installation.md).
2. Once you have the SDK ready, you can start compiling it by opening cmd/powershell/windows terminal in the git cloned/downloaded folder.
3. Once you are in the folder where the source is, then you can do cmake --preset default . and then cmake --build ./build and it should compile and create a bin/cue file.
4. Now you can run the .bin file on almost any emulator or on real hardware with or without a modchip (I haven't tested this on a real PS1 yet).

## Credits

Thanks to [Lameguy64](https://github.com/Lameguy64) for the SDK, examples, and documentations.

Thanks to [spicyjpeg](https://github.com/spicyjpeg) for the launchExecutable function.