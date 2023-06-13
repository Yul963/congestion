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
    //device를 cpu로 생성하고, cuda가 사용가능하면 cuda로 바꿔줌
    size = 224;
    duration_second=60;
    stop_flag = false;
    if (torch::cuda::is_available()) {
        device = torch::Device(torch::kCUDA, 0);
        std::cout << "CUDA is available. Using GPU." << std::endl;
    } else {
        std::cout << "CUDA is not available. Using CPU." << std::endl;
    }
    //정해진 경로의 모델을 로드
    std::cout<<"loading model..." << std::endl;
    try {
        module = torch::jit::load("model_scripted_cpu_224.pt");
    }
    catch (const c10::Error& e) {
        throw std::runtime_error("Error loading the model.");
    }
    torch::NoGradGuard no_grad;
    module.eval();
    module.to(device);
    std::cout<<"done."<<std::endl;
}

//정규화 등의 이미지 전처리 수행하고 텐서로 변환해서 리턴
torch::Tensor ImageProcessor::post_process(cv::Mat image){
    resize(image, image, cv::Size(size, size));
    
    if (!std::filesystem::exists("examples")) {
        std::filesystem::create_directory("examples");
    }
    current_time = std::chrono::system_clock::now();
    current_time_t = std::chrono::system_clock::to_time_t(current_time);
    std::string img_path;
    img_path.append("examples/").append(std::ctime(&current_time_t)).append(".jpg");
    //imwrite(img_path, image);

    float MEAN[] = {0.48897059, 0.46548275, 0.4294};
    float STD[] = {0.22861765, 0.22948039, 0.24054667};

    cv::Mat float_image;
    image.convertTo(float_image, CV_32FC3, 1.0 / 255);
    float_image -= cv::Scalar(MEAN[0], MEAN[1], MEAN[2]);
    float_image /= cv::Scalar(STD[0], STD[1], STD[2]);

    torch::Tensor input_tensor = torch::from_blob(float_image.data, {1, size, size, 3}).permute({0, 3, 1, 2});

    return input_tensor;
}

//출력 텐서를 이미지로 변환함
cv::Mat ImageProcessor::tensor_to_image(torch::Tensor tensor){
    tensor = torch::softmax(tensor.squeeze(0), 0).argmax(0).cpu().to(torch::kUInt8);

    cv::Mat mask_image(tensor.size(0), tensor.size(1), CV_8UC1, tensor.data_ptr());

    cv::multiply(mask_image, cv::Scalar(255), mask_image);
    return mask_image;
}

void ImageProcessor::set_stop(){
    stop_flag = true;
}

void ImageProcessor::set_duration(int sec){
    duration_second = sec;
}

void ImageProcessor::process_image(cv::Mat& image){//시간 나면 std::vector<cv::Mat> images로 수정
    
    //module.forward({torch::zeros({1, 3, size, size})});
    torch::Tensor output;
    std::string img_path;
    torch::Tensor input_tensor;

    input_tensor = post_process(image).to(device);
    output = module.forward({input_tensor}).toTensor();
    image = tensor_to_image(output);


    //imwrite 버그있어서 고쳐야함.
    /*
    if (!img_path.empty())
        img_path.clear();
    img_path.append("examples/").append(std::ctime(&current_time_t)).append(".png");
    imwrite(img_path, image);
    */
}

//img_path의 이미지를 로드함
cv::Mat getImage(std::string img_path){
    cv::Mat image;
    if (!std::ifstream(img_path).good()){
        std::cout << "File not found: " << img_path << std::endl;
        return image;
    }
    image = cv::imread(img_path, cv::IMREAD_COLOR);
    if (image.empty()){
            std::cout << "Error: Unable to load image" << std::endl;
            return image;
    }
    return image;
}