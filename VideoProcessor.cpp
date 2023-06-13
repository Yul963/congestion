#include <opencv2/opencv.hpp>
#include <iostream>
#include <condition_variable>
#include <string>
#include <thread>
#include <VideoProcessor.hpp>

CCTV::CCTV(std::string url, std::string name){
    this->url = url;
    this->name = name;
    stop_flag = false;
    window = false;
    //url로부터 비디오를 받는 데 실패하면 throw
    std::cout<<"opening VideoCapture of "<< this->name << "..." << std::endl;
    cap.open(this->url);

    if (!cap.isOpened())
        throw std::runtime_error("Error opening the rtsp stream.");
    std::cout<<"done."<<std::endl;
}

void CCTV::set_stop(){
    stop_flag = true;
}

cv::Mat CCTV::get_current_frame(){
    return frame.clone();
}

void CCTV::see_window(){
    if(frame.empty())
        std::cout<<"stream is not active."<<std::endl;
    else
        window = true;
}

void CCTV::process_video() {//영상을 계속 받아 frame 업데이트
    int fps = cap.get(cv::CAP_PROP_FPS);
    int frame_interval;
    bool ret;
    bool is_opened=false;
    while (!stop_flag) {
        ret = cap.read(frame);
        if (!ret) {
            std::cout << "Error: Unable to read the frame from the video capture" << std::endl;
            break;
        }
        if (frame.empty()) {
            std::cerr << "End of rtsp stream." << std::endl;
            break;
        }
        if(window){
            if(!is_opened){
                cv::namedWindow(name, cv::WINDOW_NORMAL);
                is_opened = true;
            }
            cv::imshow(name, frame);
            if (cv::waitKey(1) == 'q') {
                std::cout << "quit showing video." << std::endl;
                cv::destroyAllWindows();
                is_opened = false;
                window = false;
            }
        }
    }
    cap.release();
    if(is_opened)
        cv::destroyAllWindows();
    return;
}

std::string CCTV::get_name(){
    return name;
}

ROOM::ROOM(int base, std::string location){
    if (base==0){
        std::cout<<location<<" base not set."<<std::endl;
    }
    this->congestion = 0.;
    this->base = base;
    this->location = location;
}

std::vector<cv::Mat> ROOM::get_target_images(){
    std::vector<cv::Mat> images;
    for (auto& cctv : cctvs){
        //std::cout<<std::endl<<"get frame"<<std::endl;
        images.emplace_back(cctv.get_current_frame());
    }
    return images;
}

std::vector<class CCTV> ROOM::get_cctvs(){
    return cctvs;
}

std::string ROOM::get_location(){
    return location;
}

void ROOM::add_cctv(std::string url, std::string name){
    try {
        cctvs.emplace_back(url, name);
    } catch (...) {
        
    }
}

void ROOM::cal_congestion(std::vector<cv::Mat> target_images){
    int target=0;
    for (auto& image : target_images){
        target+=cv::countNonZero(image);
    }
    congestion = target >= base ? 0.0f : (float)(base-target)/(float)base;
}

float ROOM::get_congestion(){
    return congestion;
}

void ROOM::set_base(std::vector<cv::Mat> base_images){
    base=0;
    for (auto& image : base_images){
        int numChannels = image.channels();
        std::cout << "Number of channels: " << numChannels <<"  "<<  cv::countNonZero(image) << std::endl;
        base+=cv::countNonZero(image);
    }
    if (base==0)
        std::cout<<"base_image countNonZero returned 0. check base image."<<std::endl;
}

int ROOM::get_base(){
    return base;
}

void ROOM::run_threads(){
    for (auto& cctv : cctvs){
        threads.emplace_back([&]() { cctv.process_video(); });
    }
    for (auto& t : threads)
        t.detach();
}

void ROOM::stop_threads(){
    for (auto& cctv : cctvs){
        cctv.set_stop();
    }
}

void ROOM::show_cctvs(){
    for (auto& cctv : cctvs){
        cctv.see_window();
    }
}

