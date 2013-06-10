#!/bin/bash

g++ -Wall modules/UdpNetwork.cpp modules/VideoDecoder.cpp modules/VideoEncoder.cpp modules/VideoProcessor.cpp server.cpp -lswscale -lavcodec -lavutil -lx264 -lavformat `pkg-config --libs opencv` -o ../build/server

