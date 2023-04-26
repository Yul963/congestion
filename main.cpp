#include <torch/torch.h>
#include <torch/script.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <congestion_cal.hpp>
#include <vid_processing.hpp>

using namespace std;
using namespace cv;

mutex mtx;
condition_variable conv; 
queue<Mat> q;
string url="rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0";
string model_path = "model_scripted.pt";

int main() {
    thread t_img(process_image, ref(q),ref(mtx),ref(conv),ref(model_path));
    thread t_vid(process_video, ref(q),ref(mtx),ref(conv),ref(url));
    t_img.join();
    t_vid.join();
    return 0;
}