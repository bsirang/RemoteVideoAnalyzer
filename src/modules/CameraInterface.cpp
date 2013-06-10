#include <cstddef>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cassert>
#include <sys/mman.h>
#include <stdlib.h>


#include <iostream>
#include "CameraInterface.hpp"

using namespace bsirang;


/* CameraFrame definition */

CameraFrame::CameraFrame(uint8_t *data, size_t size, uint32_t pixelFormat)
{
    mData = (uint8_t *)malloc(size);
    if(mData)
    {
        mSize = size;
        mPixelFormat = pixelFormat;
        assert(mPixelFormat == V4L2_PIX_FMT_YUYV); //currently only supporting YUYV from camera
        YUYVTo422P(mData, data, size);
    }else
    {
        mSize = 0;
    }
}

void CameraFrame::YUYVTo422P(uint8_t *planar, uint8_t *yuyv, size_t size)
{
    size_t numPixels = size / 2;
    size_t yOffset = 0;
    size_t vOffset = numPixels;
    size_t uOffset = vOffset + (numPixels / 2);
    uint8_t *y = (planar + yOffset);
    uint8_t *u = (planar + uOffset);
    uint8_t *v = (planar + vOffset);
    int count = 0;
    for(unsigned int i = 0; i < size / 4; i++)
    {
        y[count] =   yuyv[i*4+0];
        u[count/2] = yuyv[i*4+1];
        y[count+1] = yuyv[i*4+2];
        v[count/2] = yuyv[i*4+3];
        count += 2;
    }
}

CameraFrame::~CameraFrame()
{
    if(mData)
    {
        free(mData);
    }
}

/* CameraInterface definition */

CameraInterface::CameraInterface(CameraStreamReceiver *csr) : mCsr(csr), mFd(0) {}

CameraInterface::CameraInterface() : mCsr(NULL), mFd(0) {}

bool CameraInterface::startCapture(std::string &devName)
{
    mFd = openDevice(devName);
    bool result = (mFd > 0) && initDevice() && startStream();

    if(result) std::cout << "Capturing from " << devName << "!" << std::endl;
    else std::cout << "Attempt to start capture failed!" << std::endl;

    return result;
}

bool CameraInterface::stopCapture()
{
    return true;
}

void CameraInterface::setStreamReceiver(CameraStreamReceiver *csr)
{
    mCsr = csr;
}

void CameraInterface::beginReceivingFrames()
{
    assert(mCsr);

    for(;;)
    {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(mFd, &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;
        r = select(mFd + 1, &fds, NULL, NULL, &tv);
        if(r < 0)
        {
            if(errno == EINTR) continue; //syscall interrupt is ok
            std::cout << "select() - " << strerror(errno) << std::endl;
            return;
        }else if(r == 0)
        {
            std::cout << "select() timeout" << std::endl;
            return;
        }
        readFrame();
    }
}


/* Private */

int CameraInterface::openDevice(std::string &devName)
{
    int fd = open(devName.c_str(), O_RDWR | O_NONBLOCK, 0);

    if(fd < 0)
    {
        std::cout << "Cannot open " << devName << " - " << strerror(errno) << std::endl;
    }
    return fd;
}

bool CameraInterface::initDevice()
{
    struct v4l2_capability cap = {0};
    struct v4l2_format fmt = {0};
    struct v4l2_requestbuffers req = {0};

    if(xioctl(VIDIOC_QUERYCAP, &cap) == -1)
    {
        std::cout << "VIDIOC_QUERYCAP - " << strerror(errno) << std::endl;
        return false;
    }
    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        std::cout << "Device specified is not a capture device!" << std::endl;
        return false;
    }
    if(!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        std::cout << "Device specified does not support streaming!" << std::endl;
        return false;
    }

    //TODO VIDIOC_CROPCAP ?

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(xioctl(VIDIOC_G_FMT, &fmt) == -1)
    {
        std::cout << "VIDIOC_G_FMT - " << strerror(errno) << std::endl;
        return false;
    }else
    {
        mPixelFormat = fmt.fmt.pix.pixelformat;
        printVideoFormat(fmt);
    }

    /* Set up mmap-based i/o */
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if(xioctl(VIDIOC_REQBUFS, &req) == -1)
    {
        std::cout << "VIDIOC_REQBUFS - " << strerror(errno) << std::endl;
        return false;
    }
    if(req.count < 2)
    {
        std::cout << "Insufficient buffer memory for specified device" << std::endl;
        return false;
    }
    CameraBuffer buf;
    mBuffers = std::vector<CameraBuffer>(req.count, buf);

    assert(req.count == mBuffers.size());

    for(unsigned int i = 0; i < req.count; i++)
    {
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if(xioctl(VIDIOC_QUERYBUF, &buf) == -1)
        {
            std::cout << "VIDIOC_QUERYBUF - " << strerror(errno) << std::endl;
            return false;
        }

        mBuffers[i].length = buf.length;
        mBuffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, mFd, buf.m.offset);

        if(mBuffers[i].start == MAP_FAILED)
        {
            std::cout << "Error from mmap for buffer #" << i << std::endl;
            return false;
        }
    }

    return true;
}

bool CameraInterface::startStream()
{
    enum v4l2_buf_type type;
    for(unsigned int i = 0; i < mBuffers.size(); i++)
    {
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if(xioctl(VIDIOC_QBUF, &buf) == -1)
        {
            std::cout << "VIDIOC_QBUF - " << strerror(errno) << std::endl;
            return false;
        }
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(xioctl(VIDIOC_STREAMON, &type) == -1)
    {
        std::cout << "VIDIOC_STREAMON - " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

void CameraInterface::readFrame()
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if(xioctl(VIDIOC_DQBUF, &buf) == -1)
    {
        std::cout << "VIDIOC_DQBUF - " << strerror(errno) << std::endl;
        return;
    }
    assert(buf.index < mBuffers.size());

    //std::cout << "New frame read and stored in buffer #" << buf.index << std::endl;
    CameraFrame frame((uint8_t *)mBuffers[buf.index].start, buf.bytesused, mPixelFormat);

    static uint32_t count = 0;
    if(count++ % 10 == 0) //TODO implement cleaner solution (this is quick hack to reduce FPS)
    {
        mCsr->didReceiveFrame(frame);
    }

    if(xioctl(VIDIOC_QBUF, &buf) == -1)
    {
        std::cout << "VIDIOC_QBUF - " << strerror(errno) << std::endl;
    }
}


bool CameraInterface::stopStream()
{
    return true;
}

bool CameraInterface::uninitDevice()
{
    return true;
}

bool CameraInterface::closeDevice()
{
    return true;
}

int CameraInterface::xioctl(int request, void *arg)
{
    int r;
    do
    {
        r = ioctl(mFd, request, arg);
    }while(r == -1 && errno == EINTR);

    return r;
}

void CameraInterface::printVideoFormat(struct v4l2_format fmt)
{
    std::cout << "Video Format: " << std::endl;
    std::cout << "\tWidth: " << fmt.fmt.pix.width << std::endl;
    std::cout << "\tHeight: " << fmt.fmt.pix.height << std::endl;
    std::cout << "\tPixel Format: " << fmt.fmt.pix.pixelformat << std::endl;
    std::cout << "\tPixel Field: " << fmt.fmt.pix.field << std::endl;
}
