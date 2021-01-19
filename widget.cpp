#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    readFile();//为了将上一次保存的人名 存储到names中
    faceToolInit();
    videoInit();
    //每隔10ms启动定时器
    timerId = startTimer(10);
    ui->pushButton->setEnabled(false);
    connect(ui->lineEdit,&QLineEdit::editingFinished,
            this,&Widget::dealBut);
    connect(&studyTimer,&QTimer::timeout,
            this,&Widget::dealStudyTimer);//定时器关联
    connect(&showNameTimer,&QTimer::timeout,
            this,&Widget::dealshowNameTimer);//定时器关联
}

void Widget::faceToolInit()
{
    //级联分类器初始化
    classifier = CascadeClassifier(CLASSIFIERMODEL);//加载模型文件,根据模型创建对象
    if(classifier.empty()==true)
    {
        QMessageBox::warning(this,"warn","级联分类器加载失败");
    }
    //人脸识别器初始化
    QFile file(RECOGNIZEMODEL);//创建一个文件对象
    if(file.exists()==false)
    {
        recognizer = LBPHFaceRecognizer::create();
    }
    else
    {
        recognizer = FaceRecognizer::load<LBPHFaceRecognizer>(RECOGNIZEMODEL);
    }
}

void Widget::videoInit()
{
     //打开摄像头
    if(vc.open(0)==false)
    {
        QMessageBox::warning(this,"warn","打开摄像头失败");
        return;
    }
    vc>>frame;
    //确定有效的识别区域 300*300
    enableRect = Rect(frame.cols/2-150,
                      frame.rows/2-150,
                      300,
                      300);
    //重载label的大小
    ui->label->resize(QSize(frame.cols,frame.rows));
}

void Widget::blackFrame()
{

    for(int i=0;i<frame.cols;++i)
    {
        for(int j=0;j<frame.rows;++j)
        {
            //有效识别区域内的像素点保持不变
            if(enableRect.contains(Point(i,j))==true)
                continue;//亮度不变 也可以进行卷积处理使图像更鲜明
            //暗化
            frame.at<Vec3b>(j,i)[0] = saturate_cast<uchar>
                    (frame.at<Vec3b>(j,i)[0]-100);
            frame.at<Vec3b>(j,i)[1] = saturate_cast<uchar>
                    (frame.at<Vec3b>(j,i)[1]-100);
            frame.at<Vec3b>(j,i)[2] = saturate_cast<uchar>
                    (frame.at<Vec3b>(j,i)[2]-100);
        }
    }

}
void Widget::timerEvent(QTimerEvent *e)
{
    //读取帧图像
    vc>>frame;
    //以y轴反转
    flip(frame,frame,1);
    if(frame.data==false){
        QMessageBox::warning(this,"warn","读取图像失败");
    }
    blackFrame();//暗化
    rectangle(frame,enableRect,Scalar(255,255,255));//显示有效区域
    findFace();//找到人脸矩形框
    drawFace();//框出人脸
    int index = checkFace();
    showName(index);
    showImg(); //将frame放置到label上
}
void Widget::findFace()//找到人脸矩形框
{
    faces.clear();//每次都清空faces
    Mat dst=frame(enableRect);//截图
    cvtColor(dst,dst,CV_BGR2GRAY);//色彩空间转换，级联分类器识别的图片必须为灰度图
    classifier.detectMultiScale(dst,faces);//得到人脸矩形区域
    //这个人脸矩形框是相对dst的，而不是frame的
}

void Widget::drawFace()//框出人脸
{
    if(faces.size()>0)
    {
        Rect r(faces[0].x+enableRect.x,
               faces[0].y+enableRect.y,
               faces[0].width,
               faces[0].height);
        rectangle(frame,r,Scalar(0,0,255));
    }
}
void Widget::showImg()//将frame放置到label上
{
    // 1 色彩空间转换 BGR--RGB,label上显示的是RGB图片
    cvtColor(frame,frame,CV_BGR2RGB);
    // 2 生成QImage的对象
    QImage img(frame.data,
               frame.cols,
               frame.rows,
               frame.cols*frame.channels(),
               QImage::Format_RGB888);
    // 3 将图片放置到label上
    ui->label->setPixmap(QPixmap::fromImage(img));
    // 4 重置label的大小
    ui->label->resize(QSize(frame.cols,frame.rows));
}

Mat Widget::faceGet()
{
    if(faces.size()==0)
        return Mat();
    Rect r(faces[0].x+enableRect.x,
           faces[0].y+enableRect.y,
           faces[0].width,
           faces[0].height);
    //截图 获取需要学习用的人脸灰度图片
    Mat dst = frame(r);
    cvtColor(dst,dst,CV_BGR2GRAY);
    //重置大小
    cv::resize(dst,dst,Size(100,100));
    return dst;
}

void Widget::dealStudyTimer()
{
    if(faces.size()==0)
        return;//代表当前窗口没有要识别的人脸
    Mat face=faceGet();
    if(face.empty()==false)
    {
        //将收集到的人脸图片保存到vector<Mat>studyFaces;
        studyFaces.push_back(face);
    }
    qDebug() <<"*********" <<studyFaces.size() <<"*********";
    if(studyFaces.size()==STUDYFACECOUNT)
    {
        //如果收集到的人脸个数=10 那么就结束采集人脸
         studyTimer.stop();//学习结束
         isStudy==false;
         ui->pushButton->setEnabled(true);

         QString str=ui->lineEdit->text();//保存学习时人脸对应的名字
         names.push_back(str);//names 保存学习时人脸对应的名字
         ui->lineEdit->clear();

         vector<int> faceIndex;
         faceIndex.insert(faceIndex.begin(),STUDYFACECOUNT,names.size()-1);
         //faceindex就是names数组下标

         recognizer->update(studyFaces,faceIndex);
         //保存模型
         recognizer->save(RECOGNIZEMODEL);
         //写文件
         writeFile();//将names的内容写入文件中。

         QMessageBox::warning(this,"warn","人脸录入完成");
    }
}
void Widget::writeFile()
{
    QFile file("names.txt");
    //打开文件
    if(false == file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        qDebug() << "文件打开失败";
        return;
    }
    //读写文件
    QTextStream textstream(&file);
    //textstream.setCodec("UTF-8");
    for(int i=0;i<names.size();i++)
    {
        textstream << names[i] << "\n";//将名字写入textstream中
    }

    //关闭文件
    file.close();
}
int Widget::checkFace()
{
    //正在学习或者人脸识别器无效 返回-1
    if(isStudy == true || recognizer->empty() == true )
        return -1;

    //获取待检测的人脸图片
    Mat face = faceGet();
    if(face.empty() == true)
        return -1;//如果检测到的人脸失败  那么返回-1

    //预测人脸
    int index = -1;
    double confidence = 0.0;
    recognizer->predict(face,index,confidence);
    return index;
}
void Widget::dealshowNameTimer()
{
    ui->label_infro->clear();
    showNameTimer.stop();
}
void Widget::showName(int index)
{
    if(-1 == index)//没有检测到人脸
        return;
    QString name = QString("%1 已打卡").arg(names[index]);

    //显示到label_infro上
    ui->label_infro->setText(name);

    if(false == showNameTimer.isActive()){
        showNameTimer.start(2000);
    }
}
void Widget::readFile()
{
    QFile file("names.txt");
    //打开文件
    if(false == file.open(QIODevice::ReadOnly))
    {
        qDebug() << "打开文件失败";
        return;
    }
    //操作文件 将文件内容读出来然后保存到names容器中
    QTextStream textStream(&file);
    //textStream.setCodec("UTF-8");
    while(false == textStream.atEnd())
    {
        QString name = textStream.readLine();
        names.push_back(name);
    }
    //关闭文件
    file.close();
}
void Widget::dealBut()
{
    ui->pushButton->setEnabled(true);

}
Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    isStudy==true;
    //不允许在学习的过程中重复学习
    ui->pushButton->setEnabled(false);
    //每隔500ms启动学习人脸用的定时器
    studyTimer.start(500);//每隔500ms执行函数dealStudyTimer()
}
