#ifndef WIDGET_H
#define WIDGET_H

#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <QApplication>
#include <QTimerEvent>
#include <QWidget>
#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <vector>
using namespace std;
using namespace cv;
using namespace cv::face;
#define CLASSIFIERMODEL "haarcascade_frontalface_alt2.xml"
//人脸检测模型文件相对路径,用于级联分类器
#define RECOGNIZEMODEL "face_recognize.xml"
//人脸识别模型文件相对路径
#define STUDYFACECOUNT 32 //需要学习的人脸的个数
//需要学习的人脸的个数
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    void faceToolInit();//初始化人脸识别要用的工具,级联分类器与人脸识别器
    void videoInit();//初始化摄像头相关
    void blackFrame();//黑化处理 有效识别区域以外的范围像素点降低
    void timerEvent(QTimerEvent *e);//定时器事件
    void findFace();//在有效识别区域内查找人脸
    void drawFace();//框出人脸
    void showImg();//将frame放置到label上
    Mat faceGet();//得到人脸截图
    void writeFile();//将names的内容写入文件中
    int checkFace();//检测人脸 返回对应人脸的标签
    void showName(int index);//根据label的值显示人名
    void readFile();//读文件中的内容保存到names
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void dealBut();
    void dealStudyTimer();//处理学习人脸时的槽函数
    void dealshowNameTimer();//处理显示名字的槽函数

private:
    Ui::Widget *ui;
    CascadeClassifier classifier;//创建一个级联分类器的空对象
    Ptr<FaceRecognizer>  recognizer;//创建人脸识别器的空对象
    VideoCapture vc;//操作摄像头的类对象
    int timerId;//定时器ID
    Mat frame;//显示在label上的帧图像
    Rect enableRect;//有效识别矩形区域
    vector<Rect>faces;//关于人脸的矩形区域容器
    bool isStudy;//正在学习的话  那么 isStudy = true 否则就是false
    QTimer studyTimer;//定时器ID
    vector<Mat>studyFaces;//学习时保存的人脸的图片
    vector<QString>names;//保存学习时人脸对应的名字
    QTimer showNameTimer;//显示人名相关的定时器
};

#endif // WIDGET_H
