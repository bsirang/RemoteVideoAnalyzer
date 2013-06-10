#include <iostream>
#include "VideoDecoder.hpp"

using namespace bsirang;

VideoDecoder::VideoDecoder(DecodedStreamReceiver *dsr)
{
    mDsr = dsr;
}

void VideoDecoder::decodeFrame(EncodedFrame &frame)
{
    //mCtx->extradata = frame.mExtraData;
    //mCtx->extradata_size = frame.mExtraDataSize;
    AVPacket packet;
    av_init_packet(&packet);
    std::vector<uint8_t> fullData;
    for(int i = 0; i < frame.mExtraDataSize; i++)
    {
        fullData.push_back(frame.mExtraData[i]);
    }
    for(int i = 0; i < frame.mSize; i++)
    {
        fullData.push_back(frame.mData[i]);
    }
    packet.size = fullData.size();
    packet.data = &fullData[0];
    int got_frame = 0;
    int len = avcodec_decode_video2(mCtx, mFrame, &got_frame, &packet);
    std::cout << "avcodec_decode_video2() = " << len << std::endl;
    if(got_frame)
    {
        mDsr->didReceiveFrame(mFrame);
    }
}

bool VideoDecoder::initDecoder()
{
    avcodec_register_all();
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(!codec)
    {
        std::cout << "H264 decoder not found!" << std::endl;
        return false;
    }

    mCtx = avcodec_alloc_context3(codec);

    mCtx->width = 640;
    mCtx->height = 480;
    mCtx->time_base.den = 25;
    mCtx->time_base.num = 1;
    mCtx->pix_fmt = AV_PIX_FMT_YUV422P;
    mCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    //mCtx->profile = FF_PROFILE_H264_BASELINE;
    //mCtx->bit_rate = 640*480*4;
    //mCtx->gop_size = 10;
    //mCtx->max_b_frames = 1;

    if(!mCtx)
    {
        std::cout << "Could not allocate codec context!" << std::endl;
        return false;
    }

    if(avcodec_open2(mCtx, codec, NULL) < 0)
    {
        std::cout << "Error opening codec" << std::endl;
        return false;
    }else
    {
        std::cout << "Codec opened with extra data size = " << mCtx->extradata_size << std::endl;
    }

    mFrame = avcodec_alloc_frame();
    if(!mFrame)
    {
        std::cout << "Error allocating frame" << std::endl;
        return false;
    }
    std::cout << "Decoder initialized successfully" << std::endl;
    return true;
}
