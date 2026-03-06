## Sources
- Main streaming source: https://sivr.dev/posts/video-playback-using-raylib-gstreamer-rust

## Scaling
- The window should be created without reference of the media source (kindaaaaa)
- The media source should scale to the window size and re-scale on a re-sized window

- Objective is to keep the video ratio on a resized screen
- If video is larger than screen, cut the dimensions in half until it can fit on the screen
