#include "VideoProcessor.hpp"
extern "C"
{
#include <libswscale/swscale.h>
}
using namespace bsirang;

VideoProcessor::VideoProcessor()
{
    cvNamedWindow("videoDisplay", CV_WINDOW_AUTOSIZE); 
    cvMoveWindow("videoDisplay", 100, 100);
    std::cout << "VideoProcessor created window" << std::endl;
}

void VideoProcessor::didReceiveFrame(AVFrame *frame)
{
    /*
    std::cout << "Decoded frame" << std::endl;
    std::cout << "\tWidth: " << frame->width << std::endl;
    std::cout << "\tHeight: " << frame->height << std::endl;
    std::cout << "\tFormat: " << frame->format << std::endl;
    std::cout << "\tKey Frame: " << frame->key_frame << std::endl;
    */
    IplImage *image = AVFrameToIplImage(frame);
    cvShowImage("videoDisplay", image);
    cvWaitKey(30);
    cvReleaseImage(&image);
}

IplImage *VideoProcessor::AVFrameToIplImage(AVFrame *frame)
{
    AVPicture pict;
    struct SwsContext *convertContext = sws_getContext(frame->width, //source
                                                       frame->height, //source
                                                       (AVPixelFormat)frame->format, //source
                                                       frame->width, //destination
                                                       frame->height, //destination
                                                       PIX_FMT_RGB24, //destination
                                                       SWS_BICUBIC,
                                                       NULL, NULL, NULL);
    IplImage *result = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);

    int r = avpicture_alloc(&pict, PIX_FMT_RGB24, frame->width, frame->height);
    assert(r == 0);
    sws_scale(convertContext,
              frame->data, //src
              frame->linesize, //src
              0, //offset of first row
              frame->height, //number of rows
              pict.data, //dst
              pict.linesize); //dst
    memcpy(result->imageData, pict.data[0], result->imageSize);
    result->widthStep = pict.linesize[0];

    avpicture_free(&pict);

    return result;
}
