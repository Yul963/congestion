#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <vector>
#include <congestion_cal.hpp>
#include <vid_processing.hpp>

using namespace std;
using namespace cv;

mutex mtx;
condition_variable conv; 
queue<Mat> q;

string model_path = "model_scripted.pt";

string url="rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0";
cv::Mat image = cv::imread("image.jpg");

vector<pair<string, Mat>> url_image; // url과 image를 묶어 저장하는 벡터
vector<thread> threads;// 스레드 저장 벡터
pair<string, Mat> item;

int main() {
    url_image.emplace_back(url, image);//database로부터 url과 기준 image를 읽어 url_image 벡터에 추가하는 과정 필요
    threads.emplace_back(process_image, ref(q),ref(mtx),ref(conv),ref(model_path));
    for (auto& u_i : url_image)
    {
        url = u_i.first;
        image = u_i.second;
        threads.emplace_back(process_video, ref(q),ref(mtx),ref(conv),ref(url));
    }
    for (auto& t : threads)
        t.join();
    return 0;
}