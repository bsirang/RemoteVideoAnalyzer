#include <iostream>
#include <errno.h>
#include <cassert>
#include "UdpNetwork.hpp"

#define MAX_BUF 0xFFFF

using namespace bsirang;

UdpNetwork::UdpNetwork() : mSock(0)
{
    mBuf = std::vector<uint8_t>(MAX_BUF, 0);
}

UdpNetwork::~UdpNetwork()
{
    if(mSock)
    {
        close(mSock);
    }
}

void UdpNetwork::initializeServer(uint16_t localPort)
{
    int status;

    mSock = socket(AF_INET, SOCK_DGRAM, 0);
    assert ( mSock != -1 );

    mServerAddr.sin_family = AF_INET;
    mServerAddr.sin_addr.s_addr = INADDR_ANY;
    mServerAddr.sin_port = htons(localPort);

    status = bind(mSock, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr));
    assert (status != -1);

}
void UdpNetwork::initializeClient(std::string &remoteAddress, uint16_t remotePort)
{
    int status;

    mSock = socket(AF_INET, SOCK_DGRAM, 0);
    assert ( mSock != -1 );

    mClientAddr.sin_family = AF_INET;
    mClientAddr.sin_addr.s_addr = INADDR_ANY;
    mClientAddr.sin_port = 0;

    status = bind(mSock, (struct sockaddr*)&mClientAddr, sizeof(mClientAddr));
    assert (status != -1);

    mServerAddr.sin_family = AF_INET;
    inet_aton(remoteAddress.c_str(), &mServerAddr.sin_addr);
    mServerAddr.sin_port = htons(remotePort);

    std::cout << "Initialized UDP network client configured for endpoint " << remoteAddress << ":" << remotePort << std::endl;
}

void UdpNetwork::sendData(std::vector<uint8_t> &data)
{
    if(data.size() > MAX_BUF)
    {
        std::cout << "Attempting to send too large of a message " << data.size() << " > " << MAX_BUF << std::endl;
        return;
    }
    int status = sendto(mSock, &data[0], data.size(), 0,
      (struct sockaddr*)&mServerAddr, sizeof(mClientAddr));
    if(status < 0)
    {
        std::cout << "Error sending data - " << strerror(errno) << std::endl;
    }
    else
    {
        std::cout << "Sent data" << std::endl;
    }

}

std::vector<uint8_t> UdpNetwork::waitForData()
{
    uint8_t *buf = &mBuf[0];
    int addrlen = sizeof(mClientAddr);
    ssize_t numReceived = recvfrom(mSock, buf, mBuf.size(), 0,(struct sockaddr*)&mClientAddr, (socklen_t *)&addrlen);
    if(numReceived != -1)
    {
        std::cout << "Received " << numReceived << " bytes." << std::endl;
        if(numReceived)
        {
            return std::vector<uint8_t>(buf, buf+numReceived);
        }
    }else
    {
        std::cout << "Error receiving data - " << strerror(errno);
    }
    return std::vector<uint8_t>();
}

void UdpNetwork::didReceiveFrame(EncodedFrame frame)
{
    std::cout << "Did receive encoded frame " << std::endl;
    std::vector<uint8_t> data = frame.serialize();
    sendData(data);
}

