/**
 * Entry point for the server application. The server application listens for
 * encoded frames over UDP, decodes them, processes them, and displays them.
 */

#include <iostream>
#include "modules/UdpNetwork.hpp"
#include "modules/VideoEncoder.hpp"
#include "modules/VideoDecoder.hpp"
#include "modules/VideoProcessor.hpp"


int main(int argc, char *argv[])
{
    bsirang::UdpNetwork udp;
    udp.initializeServer(12345);
    bsirang::VideoProcessor vp;
    bsirang::VideoDecoder dec(&vp);
    dec.initDecoder();

    while(1)
    {
        std::vector<uint8_t> data = udp.waitForData();
        if(!data.size()) continue;
        bsirang::EncodedFrame encFrame = bsirang::EncodedFrame::deserialize(data);
        std::cout << "Received encoded frame of size " << encFrame.mSize << std::endl;
        dec.decodeFrame(encFrame);
    }

    return 0;
}
