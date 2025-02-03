# Easy FFmpeg

**Easy FFmpeg** is a library designed to simplify common tasks in audio/video processing by wrapping FFmpeg's complex functionality into easy-to-use APIs. The library supports various operations such as YUV-to-RGB conversion, saving video frames in common formats like PPM.

## Features

- **YUV to RGB Conversion**: Convert YUV420p video frames to RGB24 format.
- **FFmpeg Integration**: Built on top of FFmpeg's powerful libraries (`libavcodec`, `libavformat`, `libswscale`).
- **Easy-to-use API**: Simple function calls to perform common audio/video tasks.
- **High Performance**: Optimized to reduce redundant computations and improve speed.
- **Cross-platform**: Works on Linux, Windows, and macOS.

## Installation

### Prerequisites

You will need to install FFmpeg(4.x.x-5.x.x) libraries before using **Easy FFmpeg**:
- `libavcodec`
- `libavformat`
- `libswscale`

To install FFmpeg on Linux (Ubuntu):
```bash
sudo apt-get update
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev
```
On macOS, you can install FFmpeg using Homebrew:
```bash
brew install ffmpeg
```


## Demos
We provide three demo programs that showcase the key features of Easy FFmpeg:

### Video Player (video_player.c):

A simple video player that uses Easy FFmpeg to decode and display video frames in real time.
Demonstrates frame decoding, RGB conversion, and rendering.

### Filtering Video (filtering_video.c):

A demo to apply simple filters to video frames (e.g., grayscale, sepia, etc.).
Shows how to manipulate decoded frames and perform custom processing before displaying or saving them.

### Decode and Save (decode_and_save.c):

A straightforward demo that decodes video frames from a video file and saves each frame to a PPM file.
Useful for saving individual frames from videos or performing frame-by-frame processing.


## License
This project is licensed under the Apache 2.0 License - see the [LICENSE](./LICENSE) file for details.