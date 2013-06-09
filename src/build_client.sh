#!/bin/bash

g++ modules/CameraInterface.cpp modules/UdpNetwork.cpp modules/VideoEncoder.cpp client.cpp -lavcodec -lavutil -lx264 -lavformat -o ../build/client
