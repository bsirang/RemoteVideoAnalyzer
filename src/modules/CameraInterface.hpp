#ifndef CAMERAINTERFACE_HPP
#define CAMERAINTERFACE_HPP

#include <linux/videodev2.h>
#include <string>
#include <vector>
#include <stdint.h>

namespace bsirang
{
    /**
     * Represents a raw camera frame in one contiguous block of memory.
     * TODO Add width/height parameters
     *
     * NOTE: This struct should be passed by reference to avoid deep copying of the object because it will result in
     * double freeing in the deconstructor. Eventually smart pointers should be used.
     */
    struct CameraFrame
    {
        CameraFrame(unsigned char *data, size_t size, uint32_t pixelFormat);
        ~CameraFrame();
        unsigned char *mData;
        size_t mSize;
        uint32_t mPixelFormat;
        private:
        void YUYVTo422P(unsigned char *planar, unsigned char *yuyv, size_t size);
    };

    /**
     * Describes an interface for receiving raw camera frames.
     */
    class CameraStreamReceiver
    {
        public:
        virtual void didReceiveFrame(CameraFrame &frame) = 0;
    };

    /**
     * Interfaces with a V4L2 compatible camera. Obtains camera frames,
     * and sends them to the specified CameraStreamReceiver.
     *
     * V4L2 reference: http://linuxtv.org/downloads/v4l-dvb-apis/capture-example.html
     *
     * TODO Add support for setting various V4L2 parameters.
     */
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
