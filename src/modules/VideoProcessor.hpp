
#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#define __STDC_CONSTANT_MACROS
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "VideoDecoder.hpp"

namespace bsirang
{
    class VideoProcessor : public DecodedStreamReceiver
    {
        public:
        VideoProcessor();
        void didReceiveFrame(AVFrame *frame);
        private:
        static IplImage *AVFrameToIplImage(AVFrame *frame);
    };
}

#endif //VIDEOPROCESSOR_H
