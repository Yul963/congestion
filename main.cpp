#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <vector>
#include <ImageProcessor.hpp>
#include <VideoProcessor.hpp>

using namespace std;
using namespace cv;

mutex mtx;
condition_variable conv; 
queue<Mat> q;

vector<thread> threads;
vector<class CCTV> cctvs;

void status(){

}

void add_cctv(){

}

void settings(){

}

int main() {
    string model_path = "model_scripted_cpu.pt";
    ImageProcessor ImgP;
    //for()
    try {
        cctvs.emplace_back("rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0", "cctv1", cv::imread("image.jpg"));
    } catch (...) {
        cctvs.pop_back();
    }

    threads.emplace_back([&]() {ImgP.process_image(q,mtx,conv,model_path);});
    for (auto& cctv : cctvs){
        threads.emplace_back([&]() { cctv.process_video(q, mtx, conv); });
    }
        
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