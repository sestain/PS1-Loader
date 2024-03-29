# PS1 Loader

## Description

- I wanted to recreate a bootleg crash ps1 collection (that uses modern SDK - [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK)).
- Tested on PCSX-Redux (18059.20240224.7.x64 - 2024-02-24 19:02:19) and Duckstation 0.1-6292-g0bc42c38 (dev).
- Tested on SCPH-50004, SCPH-77004 and SCPH-90004 (all mechapwn'd) (iso.xml specifies a PAL license so it works on fat models. You can extract the license file out of an original game dump.)

## Compiling and usage

To build this (tested only on Windows), you'll need the [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK/blob/master/doc/installation.md).

Once you have the SDK ready, you can start compiling it by opening cmd.exe, powershell.exe, or a Windows terminal in the git cloned or downloaded folder.

Once you are in the folder where the source is, then you can do these commands and it should compile and create a bin/cue file.

    cmake --preset default .
    cmake --build ./build
    
In order to build this, you will need the game files and license file 
(all of these can be extracted from the original game but require small hex editing due to files having the same name, license from the Official Playstation 1 Software Development Kit (PSYQ)).

Now you can run the .bin file on almost any emulator or on real hardware with or without a modchip (I haven't tested this on a real PS1 yet).

## Credits

Thanks to [Lameguy64](https://github.com/Lameguy64) for the SDK, examples, and documentation.

Thanks to [spicyjpeg](https://github.com/spicyjpeg) for the launchExecutable function.
