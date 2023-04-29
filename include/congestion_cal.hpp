#pragma once
#include <torch/torch.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

cv::Mat getImage(std::string& img_path);

torch::Tensor post_process(cv::Mat image);

torch::jit::script::Module load_model(std::string& model_path);

cv::Mat tensor_to_image(torch::Tensor tensor);

float get_congestion(std::vector<cv::Mat> base_images, std::vector<cv::Mat> target_image);

void process_image(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv, std::string& model_path);