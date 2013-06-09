#ifndef CAMERAINTERFACE_HPP
#define CAMERAINTERFACE_HPP

#include <linux/videodev2.h>
#include <string>
#include <vector>
#include <stdint.h>

namespace bsirang
{
    struct CameraFrame
    {
        CameraFrame(unsigned char *data, size_t size, uint32_t pixelFormat);
        ~CameraFrame();
        unsigned char *mData;
        size_t mSize;
        uint32_t mPixelFormat;
        private:
        void YUYVToPlanar(unsigned char *planar, unsigned char *yuyv, size_t size);
    };
    class CameraStreamReceiver
    {
        public:
        virtual void didReceiveFrame(CameraFrame &frame) = 0;
    };

    class CameraInterface
    {
        public:
        CameraInterface(CameraStreamReceiver *csr);
        CameraInterface();
        bool startCapture(std::string &devName);
        bool stopCapture();
        void setStreamReceiver(CameraStreamReceiver *csr);
        void beginReceivingFrames();

        private:
        struct CameraBuffer
        {
            void *start;
            size_t length;
        };
        CameraStreamReceiver *mCsr;
        int mFd;
        std::vector<CameraBuffer> mBuffers;
        uint32_t mPixelFormat;

        int openDevice(std::string &devName);
        bool initDevice();
        bool startStream();

        void readFrame();

        bool stopStream();
        bool uninitDevice();
        bool closeDevice();

        int xioctl(int request, void *arg);
        void printVideoFormat(struct v4l2_format fmt);
    };
}

#endif //CAMERAINTERFACE_HPP
