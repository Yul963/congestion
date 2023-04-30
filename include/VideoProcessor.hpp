#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

class CCTV{
    private:
        int sec;
        bool see_windows;
        std::string url, name, location;
        cv::Mat base_image;
        cv::VideoCapture cap;
        std::thread t;

    public:
        CCTV(std::string url, std::string name, std::string location, cv::Mat base_image);
        void change_sec(int s);
        void process_video(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv);
        std::thread start_thread();
};