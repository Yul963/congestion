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
                is_opened=false;
            }
        }
    }
    cap.release();
    if(is_opened)
        cv::destroyAllWindows();
    return;
}

ROOM::ROOM(int base, std::string location){
    if (base==0){
        std::cout<<"base not set."<<std::endl;
    }
    this->base = base;
    this->location = location;
}

std::vector<cv::Mat> ROOM::get_target_images(){
    images.clear();
    for (auto& cctv : cctvs){
        images.emplace_back(cctv.get_current_frame());
    }
    return images;
}

void ROOM::add_cctv(std::string url, std::string name){
    try {
        cctvs.emplace_back(url, name);
    } catch (...) {
        
    }
}

float ROOM::get_congestion(std::vector<cv::Mat> target_images){
    cv::Mat merged_target;
    cv::vconcat(target_images, merged_target);
    int target = cv::countNonZero(merged_target);
    return target >= base ? 0.0f : (float)(base-target)/(float)base;
}

void ROOM::set_base(std::vector<cv::Mat> base_images){
    cv::Mat merged_base;
    cv::vconcat(base_images, merged_base);
    base = (float)cv::countNonZero(merged_base);//0이 아닌 값 개수(바닥 픽셀수)
    if (base==0){
        std::cout<<"base_image countNonZero returned 0. check base image."<<std::endl;
    }
    //base 값 저장하는 부분 필요

    //추가
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

