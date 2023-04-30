#include <torch/torch.h>
#include <torch/script.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include <stdlib.h>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <ImageProcessor.hpp>

ImageProcessor::ImageProcessor() : device(torch::kCPU) {
    if (torch::cuda::is_available()) {
        device = torch::Device(torch::kCUDA, 0); // Set current device to CUDA device 0
        std::cout << "CUDA is available. Using GPU." << std::endl;
    } else {
        std::cout << "CUDA is not available. Using CPU." << std::endl;
    }
    std::cout<<"loading model...";
    try {
        module = torch::jit::load("model_scripted_cpu.pt");
    }
    catch (const c10::Error& e) {
        throw std::runtime_error("Error loading the model.");
    }
    std::cout<<"done."<<std::endl;;
}

torch::Tensor ImageProcessor::post_process(cv::Mat image){
    //cvtColor(image, image, cv::COLOR_BGR2RGB);
    resize(image, image, cv::Size(400, 400));
    
    if (!std::filesystem::exists("examples")) {
        std::filesystem::create_directory("examples");
    }
    current_time = std::chrono::system_clock::now();
    current_time_t = std::chrono::system_clock::to_time_t(current_time);
    std::string img_path;
    img_path.append("examples/").append(std::ctime(&current_time_t)).append(".jpg");
    imwrite(img_path, image);

    float MEAN[] = {0.48897059, 0.46548275, 0.4294};
    float STD[] = {0.22861765, 0.22948039, 0.24054667};

    cv::Mat float_image;
    image.convertTo(float_image, CV_32FC3, 1.0 / 255);
    float_image -= cv::Scalar(MEAN[0], MEAN[1], MEAN[2]);
    float_image /= cv::Scalar(STD[0], STD[1], STD[2]);

    torch::Tensor input_tensor = torch::from_blob(float_image.data, {1, 400, 400, 3}).permute({0, 3, 1, 2});

    return input_tensor;
}

cv::Mat ImageProcessor::tensor_to_image(torch::Tensor tensor){//opt 0 to mask, 1 to color
    tensor = torch::softmax(tensor.squeeze(0), 0).argmax(0).cpu().to(torch::kUInt8);

    cv::Mat mask_image(tensor.size(0), tensor.size(1), CV_8UC1, tensor.data_ptr());

    cv::multiply(mask_image, cv::Scalar(255), mask_image);
    return mask_image;
}

void ImageProcessor::process_image(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv){
    std::chrono::time_point<std::chrono::system_clock> start_time, end_time;
    std::chrono::duration<double> elapsed_seconds;
    
    torch::NoGradGuard no_grad;

    module.eval();
    module.to(device);
    //module.forward({torch::zeros({1, 3, 400, 400})});

    torch::Tensor output;
    std::string img_path;
    cv::Mat image;
    torch::Tensor input_tensor;
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        conv.wait(lock, [&q]{ return !q.empty(); });
        start_time = std::chrono::system_clock::now();
        image = q.front();
        q.pop();
        lock.unlock();
        input_tensor = post_process(image).to(device);
        //std::cout<< input_tensor.requires_grad()<<"  "<< torch::autograd::GradMode::is_enabled()<<std::endl;
        output = module.forward({input_tensor}).toTensor();
        image = tensor_to_image(output);
        end_time = std::chrono::system_clock::now();
        if (!img_path.empty())
            img_path.clear();
        img_path.append("examples/").append(std::ctime(&current_time_t)).append(".png");
        elapsed_seconds = end_time - start_time;
        std::cout << "Elapsed time: " << elapsed_seconds.count() << "s"<< std::endl;
        imwrite(img_path, image);
    }
}

float get_congestion(std::vector<cv::Mat> base_images, std::vector<cv::Mat> target_image){
    cv::Mat merged_base, merged_target;
    cv::vconcat(base_images, merged_base);
    cv::vconcat(target_image, merged_target);
    int base = (float)cv::countNonZero(merged_base);
    if (base==0){
        std::cout<<"base_image countNonZero returned 0. check base_image."<<std::endl;
        return -1.0f;
    }
    int target = cv::countNonZero(merged_target);
    return target >= base ? 0.0f : (float)(base-target)/(float)base;
}

cv::Mat getImage(std::string& img_path){
    cv::Mat image;
    if (!std::ifstream(img_path).good()){
        std::cout << "File not found: " << img_path << std::endl;
        exit(0);
    }
    image = cv::imread(img_path, cv::IMREAD_COLOR);
    if (image.empty()){
            std::cout << "Error: Unable to load image" << std::endl;
            exit(0);
    }
    return image;
}