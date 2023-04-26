#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

void process_video(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv, std::string& url);