#define __STDC_CONSTANT_MACROS
#include "modules/CameraInterface.hpp"
#include "modules/VideoEncoder.hpp"
#include "modules/UdpNetwork.hpp"


int main(int argc, char *argv[])
{
    std::string devName = "/dev/video0";
    bsirang::UdpNetwork udp;
    std::string remoteAddress("127.0.0.1");
    udp.initializeClient(remoteAddress, (uint16_t)12345);
    bsirang::VideoEncoder ve = bsirang::VideoEncoder(&udp);
    if(!ve.initEncoder()) return -1;

    bsirang::CameraInterface ci = bsirang::CameraInterface(&ve);
    if(ci.startCapture(devName))
    {
        ci.beginReceivingFrames();
    }

    return 0;
}
