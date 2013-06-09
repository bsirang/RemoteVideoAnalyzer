#define __STDC_CONSTANT_MACROS
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "VideoEncoder.hpp"

namespace bsirang
{
    class NetworkInterface
    {
        public:
        virtual void initializeServer(uint16_t localPort) = 0;
        virtual void initializeClient(std::string &remoteAddress, uint16_t remotePort) = 0;
        virtual void sendData(std::vector<uint8_t> &data) = 0;
        virtual std::vector<uint8_t> waitForData() = 0;
    };
    class UdpNetwork : public NetworkInterface, public EncodedStreamReceiver
    {
        public:
        UdpNetwork();
        ~UdpNetwork();
        void initializeServer(uint16_t localPort);
        void initializeClient(std::string &remoteAddress, uint16_t remotePort);
        void sendData(std::vector<uint8_t> &data);
        std::vector<uint8_t> waitForData();
        void didReceiveFrame(EncodedFrame frame);
        private:
        int mSock;
        struct sockaddr_in mServerAddr, mClientAddr;
        std::vector<uint8_t> mBuf;
    };
}
