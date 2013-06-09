#ifndef VIDEODECODER_HPP
#define VIDEODECODER_HPP

#define __STDC_CONSTANT_MACROS
#include "CameraInterface.hpp"
#include "VideoEncoder.hpp"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

namespace bsirang
{
    class DecodedStreamReceiver
    {
        public:
        virtual void didReceiveFrame(AVFrame *frame) = 0;
    };
    class VideoDecoder
    {
        public:
        VideoDecoder(DecodedStreamReceiver *dsr);
        void decodeFrame(EncodedFrame &frame);
        bool initDecoder();
        private:
        DecodedStreamReceiver *mDsr;
        bool mDecoderInitialized;
        AVCodecContext *mCtx;
        AVFrame *mFrame;
    };
}

#endif //VIDEODECODER_HPP
