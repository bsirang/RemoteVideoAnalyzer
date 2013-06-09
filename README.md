RemoteVideoAnalyzer
===================

Linux client and server programs for streaming encoded video from a webcam via UDP for remote decoding/processing of the video.

FFMpeg was built using the following configure parameters:
./configure --enable-libx264 --enable-gpl --enable-decoder=h264 --enable-shared --disable-static --prefix=/usr
