#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

class CCTV{
    private:
        std::string url, location;
        int cctv_num;
        cv::VideoCapture cap;
        cv::Mat frame;
        bool stop_flag, threadExitFlag;

    public:
        CCTV(std::string url, int cctv_num, std::string location);
        cv::Mat get_current_frame();
        void set_stop();
        void process_video();
        //void see_window();
        int get_num();
        bool get_flag();
};

class ROOM{
    private:
        int room_num;
        std::vector<class CCTV> cctvs;
        std::string location;
        double congestion;
        int base;
        std::vector<std::thread> threads;

    public:
        ROOM(int base, std::string location, int room_num);
        void add_cctv(std::string url, int cctv_num, std::string location);
        double get_congestion();
        void cal_congestion(std::vector<cv::Mat> base_images);
        void set_base(std::vector<cv::Mat>& base_images);
        int get_base();
        int get_room_number();
        std::vector<cv::Mat> get_target_images();
        std::vector<class CCTV> get_cctvs();
        std::string get_location();
        void run_threads();
        void stop_threads();
        //void show_cctvs();
};