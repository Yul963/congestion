#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

struct cctv{
    std::string url;
    std::string name;
    cv::Mat base_image;
};

void change_sec(int s);

void process_video(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv, struct cctv& info);