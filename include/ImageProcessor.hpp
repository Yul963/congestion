#pragma once
#include <torch/torch.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

class ImageProcessor{
    private:
    torch::Device device;
    std::chrono::system_clock::time_point current_time;
    std::time_t current_time_t;
    torch::jit::script::Module module;
    
    public:
    ImageProcessor();
    torch::Tensor post_process(cv::Mat image);
    cv::Mat tensor_to_image(torch::Tensor tensor);
    void process_image(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv);
};

float get_congestion(std::vector<cv::Mat> base_images, std::vector<cv::Mat> target_image);
cv::Mat getImage(std::string& img_path);