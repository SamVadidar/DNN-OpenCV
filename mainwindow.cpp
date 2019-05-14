#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//#define CAFFE
void MainWindow::detectFaceOpenCVDNN(cv::dnn::Net net, Mat frame)
{
    int frameHeight = frame.rows;
    int frameWidth = frame.cols;
    const cv::Scalar meanVal(104.0, 177.0, 123.0);

//#ifdef CAFFE
        cv::Mat inputBlob = cv::dnn::blobFromImage(frame, inScaleFactor, cv::Size(300, 300 ), meanVal, false, false);
//#else
//        cv::Mat inputBlob = cv::dnn::blobFromImage(frameOpenCVDNN, inScaleFactor, cv::Size(320, 240), meanVal, true, false);
//#endif

    net.setInput(inputBlob, "data");
    cv::Mat detection = net.forward("detection_out");

    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    for(int i = 0; i < detectionMat.rows; i++)
    {
        float confidence = detectionMat.at<float>(i, 2);

        if(confidence > confidenceThreshold)
        {
            x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth);
            y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight);
            x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth);
            y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight);

            cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0),2, 4);
//            cv::rectangle(frame, cv::Point(100, 100), cv::Point(400, 400), cv::Scalar(0, 255, 0),2, 4);

            // The problem is that the currect ROI of face is not passed to facemark!
            faces.push_back(cv::Rect(x1,y1,x2,y2));
            success = facemark->fit(frame, faces, landmarks);
            for(uint j = 0; j < 68 ; j++)
            {
              cv::circle(frame,landmarks[0][j],1, cv::Scalar(0,0,255), cv::FILLED);
            }

//            faces[0]=face;

//            dlib::rectangle()
//            faces[0]=face;
//            cout << faces[0].height << endl;
        }
    }

}

void MainWindow::on_pushButton_open_webcam_clicked()
{
    cap.open(2);
//#ifdef CAFFE
    net = cv::dnn::readNetFromCaffe(caffeConfigFile, caffeWeightFile);
    facemark = cv::face::FacemarkLBF::create();
    facemark->loadModel("/home/sam/Dev/lbfmodel.yaml");
//#else
//  cv::dnn::Net net = cv::dnn::readNetFromTensorflow(tensorflowWeightFile, tensorflowConfigFile);
//#endif

//    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
//    dlib::deserialize("/home/sam/Dev/shape_predictor_68_face_landmarks.dat") >> sp;

    if(!cap.isOpened())  // Check if we succeeded
    {
        cout << "camera is not open" << endl;
    }
    else
    {
        cout << "camera is open" << endl;

        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(20);
    }
}

void MainWindow::update_window()
{
    cap >> frame;
    cv::resize(frame, frame, cv::Size(640,480));
    detectFaceOpenCVDNN (net, frame);
//    faces = face;
//     cout << success << endl;
//    std::cout << landmarks.size() << endl;

//    dlib::cv_image<dlib::bgr_pixel> cimg(frame);
//    dlib::full_object_detection shape = sp(cimg,face);



    cvtColor(frame, frame, CV_BGR2RGB);

    qt_image = QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888);

    ui->label->setPixmap(QPixmap::fromImage(qt_image));

//    ui->label->resize(ui->label->pixmap()->size());
}

void MainWindow::on_pushButton_close_webcam_clicked()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
    cap.release();

    Mat image = Mat::zeros(frame.size(),CV_8UC3);

    qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);

    ui->label->setPixmap(QPixmap::fromImage(qt_image));

    ui->label->resize(ui->label->pixmap()->size());

    cout << "camera is closed" << endl;
}
