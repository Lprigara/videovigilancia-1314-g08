#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include <QObject>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/opencv.hpp>
#include <cvmatandqimage.h>
#include <QDebug>

//typedef std::vector<cv::Mat> ImagesType;
typedef std::vector<std::vector<cv::Point> > ContoursType;

class MotionDetector : public QObject
{
    Q_OBJECT
public:
    explicit MotionDetector(QObject *parent = 0);
    ~MotionDetector();

signals:
    void signalResult(std::vector<QRect > boundingRects, QImage image);

public slots:
    void detectMotion(QImage img);
    void reset();

private:
    cv::BackgroundSubtractorMOG2* backgroundSubtractor;
    cv::Mat* foregroundMask;

};

#endif // MOTIONDETECTOR_H
