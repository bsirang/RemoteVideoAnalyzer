RemoteVideoAnalyzer
===================

Linux client and server programs for streaming encoded video from a webcam via UDP for remote decoding/processing of the video.

These programs depend on the following libraries:
1) libav
2) libopencv
3) libx264


FFMpeg (libav) was built using the following configure parameters:
./configure --enable-libx264 --enable-gpl --enable-decoder=h264 --enable-shared --disable-static --prefix=/usr


Things to Improve:

1. Remove hardcoded values in favor of general impementations.
2. Convert synchronous pipeline to asynchronous.
3. Tuning of video codec parameters.
4. Support for sending fragments of large packets.
5. Implementing proper Makefile or equivalent
