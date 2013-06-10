#include "VideoProcessor.hpp"
#include <opencv/highgui.h>
#include <opencv2/objdetect/objdetect.hpp>
extern "C"
{
#include <libswscale/swscale.h>
}
using namespace bsirang;

VideoProcessor::VideoProcessor()
{
    mFaceCascadeXml = "objdetect/haarcascade_frontalface_alt.xml";
    mEyesCascadeXml = "objdetect/haarcascade_eye_tree_eyeglasses.xml";

    assert(mFaceCascade.load(mFaceCascadeXml));
    assert(mEyesCascade.load(mEyesCascadeXml));
    mWindowName = "Face Detection";
    cvNamedWindow(mWindowName.c_str(), CV_WINDOW_AUTOSIZE); 
    cvMoveWindow(mWindowName.c_str(), 100, 100);
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
    //cvShowImage(mWindowName.c_str(), image);
    detectAndDisplay(image);
    cvWaitKey(30);
    cvReleaseImage(&image);
}

void VideoProcessor::detectAndDisplay(cv::Mat frame)
{
    std::vector<cv::Rect> faces;
    cv::Mat frame_gray;

    cvtColor( frame, frame_gray, CV_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    //-- Detect faces
    mFaceCascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(30, 30) );

    for(unsigned int i = 0; i < faces.size(); i++ )
    {
        cv::Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
        ellipse( frame, center, cv::Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, cv::Scalar( 255, 0, 255 ), 4, 8, 0 );

        cv::Mat faceROI = frame_gray( faces[i] );
        std::vector<cv::Rect> eyes;

        //-- In each face, detect eyes
        /*
        mEyesCascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, cv::Size(30, 30) );

        for( int j = 0; j < eyes.size(); j++ )
        {
            cv::Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( frame, center, radius, cv::Scalar( 255, 0, 0 ), 4, 8, 0 );
        }
        */
    }
    //-- Show what you got
    imshow( mWindowName, frame );
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
