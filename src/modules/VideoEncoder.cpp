#include "VideoEncoder.hpp"
#include <iostream>
#include <cassert>

using namespace bsirang;

EncodedFrame::EncodedFrame(unsigned char *data, size_t size)
{
    mData = (unsigned char *)malloc(size);
    if(mData)
    {
        mSize = size;
    }else
    {
        mSize = 0;
    }
}

EncodedFrame::~EncodedFrame()
{
    if(mData)
    {
        free(mData);
    }
}

std::vector<uint8_t> EncodedFrame::serialize()
{
    std::vector<uint8_t> result;
    uint8_t *mSizeBytes = (uint8_t *)&mSize;
    for(int i = 0; i < sizeof(mSize); i++)
    {
        result.push_back(mSizeBytes[i]);
    }
    for(int i = 0; i < mSize; i++)
    {
        result.push_back(mData[i]);
    }

    return result;
}

EncodedFrame EncodedFrame::deserialize(std::vector<uint8_t> serializedData)
{
    size_t size = *(size_t *)&serializedData[0];
    return EncodedFrame(&serializedData[sizeof(size)], size);
}


VideoEncoder::VideoEncoder(EncodedStreamReceiver *esr) : mPictureBuf(NULL), mFrame(NULL), mCtx(NULL),  mEncoderInitialized(false)
{
   mEsr = esr; 
}

void VideoEncoder::didReceiveFrame(CameraFrame &frame)
{
    assert(mEncoderInitialized); //must be initialized before any frames received
    assert(frame.mSize == 640*480*2);
    memcpy(mFrame->data[0], frame.mData, 640*480); //TODO copy UV planes
    AVPacket packet;
    av_init_packet(&packet);

    packet.dts = AV_NOPTS_VALUE;
    packet.pts = AV_NOPTS_VALUE;
    int got_packet = 0;
    std::cout << "Encoding frame with width = " << mFrame->width << " height = " << mFrame->height << " pts = " << packet.pts << std::endl;
    int r = avcodec_encode_video2(mCtx, &packet, mFrame, &got_packet);
    std::cout << "avcodec_encode_video() = " << r << " got_packet = " << got_packet << std::endl;
    if(got_packet)
    {
        std::cout << "Got packet of size = " << packet.size << std::endl;
        mEsr->didReceiveFrame(EncodedFrame(packet.data, packet.size));
    }
}

bool VideoEncoder::initEncoder()
{
    avcodec_register_all();

    AVCodec *codec;

    codec = avcodec_find_encoder(CODEC_ID_H264);
    if(!codec)
    {
        std::cout << "CODEC_ID_H264 was't found!" << std::endl;
        return false;
    }
    mCtx = avcodec_alloc_context3(codec);

    mCtx->width = 640;
    mCtx->height = 480;
    mCtx->time_base.den = 25;
    mCtx->time_base.num = 1;
    mCtx->bit_rate = 500*1000;
    mCtx->gop_size = 10;
    mCtx->max_b_frames = 1;
    mCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(mCtx->priv_data, "preset", "slow", 0);
    //mCtx->profile = FF_PROFILE_H264_BASELINE;
    //mCtx->bit_rate_tolerance = 0;
    //mCtx->rc_max_rate = 0;
    //mCtx->rc_buffer_size = 0;
    //mCtx->max_b_frames = 3;
    //mCtx->b_frame_strategy = 1;
    //mCtx->coder_type = 1;
    //mCtx->me_cmp = 1;
    //mCtx->me_range = 16;
    //mCtx->qmin = 10;
    //mCtx->qmax = 51;
    //mCtx->scenechange_threshold = 40;
    //mCtx->flags |= CODEC_FLAG_LOOP_FILTER;
    //mCtx->me_method = ME_HEX;
    //mCtx->me_subpel_quality = 5;
    //mCtx->i_quant_factor = 0.71;
    //mCtx->qcompress = 0.6;
    //mCtx->max_qdiff = 4;
    //mCtx->directpred = 1;
    //mCtx->flags2 |= CODEC_FLAG2_FASTPSKIP;

    if(avcodec_open2(mCtx, codec, NULL) < 0)
    {
        std::cout << "Could not open codec" << std::endl;
        return false;
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
    mFrame->format = mCtx->pix_fmt;
    mFrame->width = mCtx->width;
    mFrame->height = mCtx->height;
    */

    mEncoderInitialized = true;
    return true;
}
