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
    /**
     *
     * Represents an encoded frame.
     *
     * NOTE: This struct should be passed by reference to avoid deep copying of the object because it will result in
     * double freeing in the deconstructor. Eventually smart pointers should be used.
     *
     */
    struct EncodedFrame
    {
        EncodedFrame(uint8_t *data, size_t size, uint8_t *extraData, size_t extraDataSize);
        ~EncodedFrame();
        void printExtraData();
        std::vector<uint8_t> serialize();
        static EncodedFrame deserialize(std::vector<uint8_t> serializedData);

        uint8_t *mData;
        size_t mSize;
        uint8_t *mExtraData;
        size_t mExtraDataSize;

    };

    /**
     * Describes an interface for receiving encoded camera frames.
     */
    class EncodedStreamReceiver
    {
        public:
        virtual void didReceiveFrame(EncodedFrame frame) = 0;
    };

    /**
     * Receives raw camera images, encodes them, and sends them to the
     * specified EncodedStreamReceiver.
     *
     * FFMpeg encoding/decoding reference: http://ffmpeg.org/doxygen/trunk/decoding__encoding_8c-source.html
     *
     * TODO Generalize. Currently video encoder, frame dimensions, and
     * pixel formats are hardcoded.
     */
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
