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
    qDebug() << "RESET";
    delete backgroundSubtractor;
    delete foregroundMask;

    backgroundSubtractor = new cv::BackgroundSubtractorMOG2();
    backgroundSubtractor->set("nmixtures",3);
    backgroundSubtractor->set("detectShadows",false);
    foregroundMask = new cv::Mat();
}

void MotionDetector::detectMotion(QImage img)
{
    qDebug() << "DETECT MOTION";

    //convert QImage to cv::Mat
    cv::Mat img_mat = QtOcv::image2Mat(img);

    backgroundSubtractor->operator ()(img_mat, *foregroundMask);

    //Erode and dilate mask to get rid of noise
    cv::erode(*foregroundMask, *foregroundMask, cv::Mat());
    cv::dilate(*foregroundMask, *foregroundMask, cv::Mat());

    //Return contours of blobs in the foreground mask.
    ContoursType contours;
    cv::findContours(*foregroundMask, contours, CV_RETR_EXTERNAL,
                     CV_CHAIN_APPROX_NONE);

    std::vector<std::vector<int> > boundingRects;

    for (std::vector<std::vector<cv::Point> >::iterator it = contours.begin() ; it != contours.end(); ++it)
    {
        cv::Rect rect = cv::boundingRect(*it);

        std::vector<int> rectData;
        rectData.push_back(rect.x);
        rectData.push_back(rect.y);
        rectData.push_back(rect.width);
        rectData.push_back(rect.height);
        boundingRects.push_back(rectData);
    }


    emit signalResult(boundingRects);
}

/*

preguntas:

const & * en slot signals correcto?
hilo creacion/delete correcto?
usando * para backgroundsubtractor correcto?
correcto el hacer boundingrect y enviarlo por senal?


*/
