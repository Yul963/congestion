#pragma once
#include <torch/torch.h>
#include <opencv2/opencv.hpp>
#include <string>

cv::Mat getImage(std::string& img_path);
torch::Tensor post_process(cv::Mat image);
torch::jit::script::Module load_model(std::string& model_path);
cv::Mat tensor_to_image(torch::Tensor tensor, int opt);