#ifndef VIDEOENCODER_HPP
#define VIDEOENCODER_HPP

#define __STDC_CONSTANT_MACROS
#include "CameraInterface.hpp"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

namespace bsirang
{
    struct EncodedFrame
    {
        EncodedFrame(unsigned char *data, size_t size);
        ~EncodedFrame();
        unsigned char *mData;
        size_t mSize;
        std::vector<uint8_t> serialize();
        static EncodedFrame deserialize(std::vector<uint8_t> serializedData);

    };
    class EncodedStreamReceiver
    {
        public:
        virtual void didReceiveFrame(EncodedFrame frame) = 0;
    };
    class VideoEncoder : public CameraStreamReceiver
    {
        public:
        VideoEncoder(EncodedStreamReceiver *esr);
        void didReceiveFrame(CameraFrame &frame);
        bool initEncoder();
        private:
        uint8_t *mPictureBuf;
        AVFrame *mFrame;
        AVCodecContext *mCtx;
        bool mEncoderInitialized;
        EncodedStreamReceiver *mEsr;
    };
}

#endif //VIDEOENCODER_HPP
