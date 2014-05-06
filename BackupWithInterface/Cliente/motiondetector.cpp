#include "motiondetector.h"

MotionDetector::MotionDetector(QObject *parent) :
    QObject(parent)
{
    backgroundSubtractor = new cv::BackgroundSubtractorMOG2();
    backgroundSubtractor->set("nmixtures",3);
    backgroundSubtractor->set("detectShadows",false);
    foregroundMask = new cv::Mat();
}

MotionDetector::~MotionDetector()
{
    delete backgroundSubtractor;
    delete foregroundMask;
}

void MotionDetector::reset()
{
    delete backgroundSubtractor;
    delete foregroundMask;

    backgroundSubtractor = new cv::BackgroundSubtractorMOG2();
    backgroundSubtractor->set("nmixtures",3);
    backgroundSubtractor->set("detectShadows",false);
    foregroundMask = new cv::Mat();
}

void MotionDetector::detectMotion(QImage img)
{
    //convert QImage to cv::Mat
    QImage imgConverted = img.convertToFormat(QImage::Format_RGB32,Qt::ColorOnly);
    cv::Mat img_mat = QtOcv::image2Mat(imgConverted);

    backgroundSubtractor->operator ()(img_mat, *foregroundMask);

    //Erode and dilate mask to get rid of noise
    cv::erode(*foregroundMask, *foregroundMask, cv::Mat());
    cv::erode(*foregroundMask, *foregroundMask, cv::Mat());
    cv::erode(*foregroundMask, *foregroundMask, cv::Mat());
    cv::dilate(*foregroundMask, *foregroundMask, cv::Mat());
    cv::dilate(*foregroundMask, *foregroundMask, cv::Mat());
    cv::dilate(*foregroundMask, *foregroundMask, cv::Mat());

    //Return contours of blobs in the foreground mask.
    ContoursType contours;
    cv::findContours(*foregroundMask, contours, CV_RETR_EXTERNAL,
                     CV_CHAIN_APPROX_NONE);

    std::vector<QRect > boundingRects;

    for (std::vector<std::vector<cv::Point> >::iterator it = contours.begin() ; it != contours.end(); ++it)
    {
        cv::Rect rect = cv::boundingRect(*it);
        boundingRects.push_back(QRect(rect.x, rect.y, rect.width, rect.height));
    }


    emit signalResult(boundingRects, imgConverted);
}
