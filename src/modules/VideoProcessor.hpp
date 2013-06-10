
#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#define __STDC_CONSTANT_MACROS
#include <opencv/cv.h> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "VideoDecoder.hpp"

namespace bsirang
{
    /**
     * Receives decoded frames, processes them, and displays them
     * using OpenCV.
     *
     * This class currently detects faces. Implementation borrowed from
     * an OpenCV tutorial seen here:
     * http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html
     *
     */
    class VideoProcessor : public DecodedStreamReceiver
    {
        public:
        VideoProcessor();
        void didReceiveFrame(AVFrame *frame);

        private:
        void detectAndDisplay(cv::Mat frame);
        static IplImage *AVFrameToIplImage(AVFrame *frame);

        std::string mFaceCascadeXml;
        std::string mEyesCascadeXml;
        std::string mWindowName;
        cv::CascadeClassifier mFaceCascade;
        cv::CascadeClassifier mEyesCascade;
    };
}

#endif //VIDEOPROCESSOR_H
