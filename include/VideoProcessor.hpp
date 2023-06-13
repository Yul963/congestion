#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

class CCTV{
    private:
        std::string url, name;
        cv::VideoCapture cap;
        cv::Mat frame;
        bool stop_flag;
        bool window;

    public:
        CCTV(std::string url, std::string name);
        cv::Mat get_current_frame();
        void set_stop();
        void process_video();
        void see_window();
        std::string get_name();
};

class ROOM{
    private:
        std::vector<class CCTV> cctvs;
        std::string location;
        float congestion;
        int base;
        std::vector<std::thread> threads;

    public:
        ROOM(int base, std::string location);
        void add_cctv(std::string url, std::string name);
        float get_congestion();
        void cal_congestion(std::vector<cv::Mat> base_images);
        void set_base(std::vector<cv::Mat> base_images);
        int get_base();
        std::vector<cv::Mat> get_target_images();
        std::vector<class CCTV> get_cctvs();
        std::string get_location();
        void run_threads();
        void stop_threads();
        void show_cctvs();
};