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
    int size;
    bool stop_flag;
    std::chrono::system_clock::time_point current_time;
    std::time_t current_time_t;
    torch::jit::script::Module module;
    
    public:
    ImageProcessor();
    torch::Tensor post_process(cv::Mat image);
    cv::Mat tensor_to_image(torch::Tensor tensor);
    void set_stop();
    cv::Mat process_image(cv::Mat& image);
};

cv::Mat getImage(std::string img_path);