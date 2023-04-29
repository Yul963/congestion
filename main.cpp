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

vector<struct cctv> cctv_info; // cctv 정보를 저장하는 벡터
vector<thread> threads;// 스레드 저장 벡터

void status(){

}

void add_cctv(){

}

void settings(){

}

int main() {
    string model_path = "model_scripted_cpu.pt";

    //database로부터 url과 기준 image를 읽어 url_image 벡터에 추가하는 과정 필요
    cctv_info.emplace_back("rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0", "cctv1", cv::imread("image.jpg"));

    threads.emplace_back(process_image, ref(q),ref(mtx),ref(conv),ref(model_path));//스레드 벡터에 이미지 처리 스레드 추가
    for (auto& info : cctv_info)//cctv 정보로 cctv 처리 스레드 추가
        threads.emplace_back(process_video, ref(q),ref(mtx),ref(conv),ref(info));
    for (auto& t : threads)
        t.detach();

    int input;
    while(1){
        cout<<"1. see current status"<<endl
        <<"2. add cctv"<<endl
        <<"3. settings"<<endl
        <<"4. quit"<<endl
        <<"input: ";
        cin>>input;
        if(input>0 && input<4){
            switch (input)
            {
            case 1:
                status();
                break;
            case 2:
                add_cctv();
                break;
            case 3:
                settings();
                break;
            case 4:
                return 0;
                break;
            default:
                break;
            }
        }else
            cout<<"input valid numbers(1~3)"<<endl; 
    }
}