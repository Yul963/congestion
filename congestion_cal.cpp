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
#include <congestion_cal.hpp>

std::chrono::system_clock::time_point current_time;
std::time_t current_time_t;

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

torch::Tensor post_process(cv::Mat image){
    // 이미지 전처리
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

    // MEAN, STD 값으로 정규화
    float MEAN[] = {0.48897059, 0.46548275, 0.4294};
    float STD[] = {0.22861765, 0.22948039, 0.24054667};

    cv::Mat float_image;
    image.convertTo(float_image, CV_32FC3, 1.0 / 255);
    float_image -= cv::Scalar(MEAN[0], MEAN[1], MEAN[2]);
    float_image /= cv::Scalar(STD[0], STD[1], STD[2]);

    // 텐서 변환
    torch::Tensor input_tensor = torch::from_blob(float_image.data, {1, 400, 400, 3}).permute({0, 3, 1, 2});

    return input_tensor;
}

    // 토치스크립트 모델 로드
torch::jit::script::Module load_model(std::string& model_path){
    torch::jit::script::Module module;
    try {
        module = torch::jit::load(model_path);
    }
    catch (const c10::Error& e) {
        std::cerr << "Error loading the model\n";
        exit(0);
    }
    return module;
}

cv::Mat tensor_to_image(torch::Tensor tensor){//opt 0 to mask, 1 to color
    tensor = torch::softmax(tensor.squeeze(0), 0).argmax(0).cpu().to(torch::kUInt8);

    cv::Mat mask_image(tensor.size(0), tensor.size(1), CV_8UC1, tensor.data_ptr());

    cv::multiply(mask_image, cv::Scalar(255), mask_image);
    return mask_image;
}

void process_image(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv, std::string& model_path){
    std::chrono::time_point<std::chrono::system_clock> start_time, end_time;
    torch::Device device(torch::kCPU);
    if (torch::cuda::is_available()) {
        device = torch::Device(torch::kCUDA, 0); // Set current device to CUDA device 0
        std::cout << "CUDA is available. Using GPU." << std::endl;
    } else {
        std::cout << "CUDA is not available. Using CPU." << std::endl;
    }
    torch::DeviceGuard guard(device);
    torch::NoGradGuard no_grad;

    torch::jit::script::Module module = load_model(model_path);
    module.to(device);

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
        input_tensor.to(device);
        //std::cout<< input_tensor.requires_grad()<<"  "<< torch::autograd::GradMode::is_enabled()<<std::endl;
        output = module.forward({input_tensor}).toTensor();
        
        image = tensor_to_image(output);
        end_time = std::chrono::system_clock::now();
        if (!img_path.empty())
            img_path.clear();
        img_path.append("examples/").append(std::ctime(&current_time_t)).append(".png");
        std::chrono::duration<double> elapsed_seconds = end_time - start_time;
        std::cout << "Elapsed time: " << elapsed_seconds.count() << "s"<< std::endl;
        imwrite(img_path, image);
    }
}