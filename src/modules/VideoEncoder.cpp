#include "VideoEncoder.hpp"
#include <iostream>
#include <cassert>

using namespace bsirang;

EncodedFrame::EncodedFrame(uint8_t *data, size_t size, uint8_t *extraData, size_t extraDataSize)
{
    mData = (uint8_t *)malloc(size);
    assert(mData);
    mSize = size;
    memcpy(mData, data, size);

    if(extraData && extraDataSize)
    {
        std::cout << "Attempting to allocate " << extraDataSize << " bytes for extraData" << std::endl;
        mExtraData = (uint8_t *)malloc(extraDataSize);
        assert(mExtraData);
        mExtraDataSize = extraDataSize;
        memcpy(mExtraData, extraData, extraDataSize);
    }
    std::cout << "Created EncodedFrame with size = " << size << " and extraDataSize = " << extraDataSize << std::endl;
}

EncodedFrame::~EncodedFrame()
{
    if(mData)
        free(mData);

    if(mExtraData)
        free(mExtraData);
}

/*
void EncodedFrame::printExtraData()
{
    for(int i = 0; i < mExtraDataSize; i++)
    {
        if(i % 16 == 0) std::cout << std::endl;
        std::cout << std::hex << (int)mExtraData[i] << " ";
    }
    std::cout << std::endl;
}
*/

std::vector<uint8_t> EncodedFrame::serialize()
{
    std::vector<uint8_t> result;

    uint8_t *mSizeBytes = (uint8_t *)&mSize;
    for(unsigned int i = 0; i < sizeof(mSize); i++)
    {
        result.push_back(mSizeBytes[i]);
    }

    for(unsigned int i = 0; i < mSize; i++)
    {
        result.push_back(mData[i]);
    }

    uint8_t *mExtraDataSizeBytes = (uint8_t *)&mExtraDataSize;
    for(unsigned int i = 0; i < sizeof(mExtraDataSize); i++)
    {
        result.push_back(mExtraDataSizeBytes[i]);
    }
    for(unsigned int i = 0; i < mExtraDataSize; i++)
    {
        result.push_back(mExtraData[i]);
    }

    return result;
}

EncodedFrame EncodedFrame::deserialize(std::vector<uint8_t> serializedData)
{
    size_t offset = 0;
    size_t size = *(size_t *)&serializedData[offset];

    offset += sizeof(size);
    uint8_t *data = &serializedData[offset];

    offset += size;
    size_t extraDataSize = *(size_t *)&serializedData[offset];
    
    offset += sizeof(extraDataSize);
    uint8_t *extraData = &serializedData[offset];

    std::cout << "Deserialized EncodedFrame with size = " << size << " and extraDataSize = " << extraDataSize << std::endl;

    return EncodedFrame(data, size, extraData, extraDataSize);
}


VideoEncoder::VideoEncoder(EncodedStreamReceiver *esr) : mPictureBuf(NULL), mFrame(NULL), mCtx(NULL),  mEncoderInitialized(false)
{
   mEsr = esr; 
}

void VideoEncoder::didReceiveFrame(CameraFrame &frame)
{
    assert(mEncoderInitialized); //must be initialized before any frames received
    assert(frame.mSize == 640*480*2);
    size_t numPixels = 640*480;
    size_t yOffset = 0;
    size_t uOffset = numPixels;
    size_t vOffset = uOffset + (numPixels / 2);

    uint8_t *y = (frame.mData + yOffset);
    uint8_t *u = (frame.mData + uOffset);
    uint8_t *v = (frame.mData + vOffset);

    memcpy(mFrame->data[0], y, numPixels);
    memcpy(mFrame->data[1], u, numPixels/2);
    memcpy(mFrame->data[2], v, numPixels/2);
    //memset(mFrame->data[1], 127, numPixels / 2);
    //memset(mFrame->data[2], 127, numPixels / 2); 

    /*
    int uvWidth = 640 / 2;
    for(int i = 0; i < 480; i++)
    {
        for(int j = 0; j < uvWidth; j++)
        {
            mFrame->data[1][(i/2)*uvWidth + j] = u[i*uvWidth + j];
            mFrame->data[2][(i/2)*uvWidth + j] = v[i*uvWidth + j];
        }
    }
    */
    AVPacket packet;
    av_init_packet(&packet);

    packet.dts = AV_NOPTS_VALUE;
    packet.pts = AV_NOPTS_VALUE;
    packet.data = NULL;
    packet.size = 0;
    int got_packet = 0;
    //std::cout << "Encoding frame with width = " << mFrame->width << " height = " << mFrame->height << " pts = " << packet.pts << std::endl;
    int r = avcodec_encode_video2(mCtx, &packet, mFrame, &got_packet);
    std::cout << "avcodec_encode_video() = " << r << " got_packet = " << got_packet << std::endl;
    if(got_packet)
    {
        std::cout << "Got packet of size = " << packet.size << std::endl;
        mEsr->didReceiveFrame(EncodedFrame(packet.data, packet.size, mCtx->extradata, mCtx->extradata_size));
    }
}

bool VideoEncoder::initEncoder()
{
    avcodec_register_all();

    AVCodec *codec;

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!codec)
    {
        std::cout << "CODEC_ID_H264 was't found!" << std::endl;
        return false;
    }
    mCtx = avcodec_alloc_context3(codec);
    mCtx->codec_id = AV_CODEC_ID_H264;
    mCtx->width = 640;
    mCtx->height = 480;
    mCtx->time_base.den = 8;
    mCtx->time_base.num = 1;
    mCtx->pix_fmt = AV_PIX_FMT_YUV422P;
    mCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    mCtx->bit_rate = 640*480*4; // 1 bits per pixel
    //av_opt_set(mCtx->priv_data, "profile", "baseline", 0);
    //av_opt_set(mCtx->priv_data, "preset", "slow", 0);
    av_opt_set(mCtx->priv_data, "tune", "zerolatency", 0);

    if(avcodec_open2(mCtx, codec, NULL) < 0)
    {
        std::cout << "Could not open codec" << std::endl;
        return false;
    }else
    {
        std::cout << "Codec opened with extra data size = " << mCtx->extradata_size << std::endl;
    }

    size_t numPixels = mCtx->width * mCtx->height;
    //mPictureBuf = (uint8_t *)malloc(numPixels * 3 / 2);

    mFrame = avcodec_alloc_frame();
    int ret = av_image_alloc(mFrame->data, mFrame->linesize, mCtx->width, mCtx->height,
                             mCtx->pix_fmt, 32);

    if(ret < 0)
    {
        std::cout << "av_image_alloc returned " << ret << std::endl;
    }
    /*
    mFrame->data[0] = mPictureBuf; //Y data at start of buffer (one sample per pixel)
    mFrame->data[1] = mPictureBuf + numPixels;
    mFrame->data[2] = mPictureBuf + numPixels + numPixels / 4;
    mFrame->linesize[0] = mCtx->width;
    mFrame->linesize[1] = mCtx->width / 2;
    mFrame->linesize[2] = mCtx->width / 2;
    */
    mFrame->format = mCtx->pix_fmt;
    mFrame->width = mCtx->width;
    mFrame->height = mCtx->height;

    mEncoderInitialized = true;
    return true;
}
