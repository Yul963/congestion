#include <ImageProcessor.hpp>

ImageProcessor::ImageProcessor() : device(torch::kCPU) {
    //device를 cpu로 생성하고, cuda가 사용가능하면 cuda로 바꿔줌
    size = 400;
    //stop_flag = false;
    if (!std::filesystem::exists("examples"))
        std::filesystem::create_directory("examples");

    if (torch::cuda::is_available()) {
        device = torch::Device(torch::kCUDA, 0);
        std::cout << "CUDA is available. Using GPU." << std::endl;
    } else {
        std::cout << "CUDA is not available. Using CPU." << std::endl;
    }
    //정해진 경로의 모델을 로드
    std::cout<<"loading model..." << std::endl;
    try {
        module = torch::jit::load("model_scripted_cpu.pt");
    }
    catch (const c10::Error& e) {
        std::cout << "Error message: " << e.what();
        throw std::runtime_error("Error loading the model.");
    }
    torch::NoGradGuard no_grad;
    module.eval();
    module.to(device);
    std::cout<<"done."<<std::endl;
}

//정규화 등의 이미지 전처리 수행하고 텐서로 변환해서 리턴
torch::Tensor ImageProcessor::post_process(cv::Mat image){
    std::vector<double> MEAN = { 0.48897059, 0.46548275, 0.4294 };
    std::vector<double> STD = { 0.22861765, 0.22948039, 0.24054667 };
    cv::resize(image, image, cv::Size(size, size));

#ifdef DEBUG
    std::string img_path;
    img_path.append("examples/").append(std::to_string((int)current_time_t)).append(".jpg");
    save_image(img_path, image.clone());
#endif
    cv::Mat float_image;
    image.convertTo(float_image, CV_32FC3, 1.0 / 255);
    cv::subtract(float_image, cv::Scalar(MEAN[0], MEAN[1], MEAN[2]), float_image);
    cv::divide(float_image, cv::Scalar(STD[0], STD[1], STD[2]), float_image);

    torch::Tensor input_tensor = torch::from_blob(float_image.data, { 1, size, size, 3 }).permute({ 0, 3, 1, 2 });
    return input_tensor.clone();
}

//출력 텐서를 이미지로 변환함
cv::Mat ImageProcessor::tensor_to_image(torch::Tensor tensor){
    torch::Tensor t = torch::softmax(tensor.squeeze(0), 0).argmax(0).cpu().to(torch::kUInt8);
    cv::Mat mask_image(t.size(0), t.size(1), CV_8UC1, t.data_ptr());
    cv::multiply(mask_image, cv::Scalar(255), mask_image);
    return mask_image.clone();
}
/*
void ImageProcessor::set_stop(){
    stop_flag = true;
}*/

cv::Mat ImageProcessor::process_image(cv::Mat image){//시간 나면 std::vector<cv::Mat> images로 수정
    current_time = std::chrono::system_clock::now();
    current_time_t = std::chrono::system_clock::to_time_t(current_time);
    //module.forward({torch::zeros({1, 3, size, size})});
    std::string img_path;
    torch::Tensor input_tensor, output_tensor;
    
    input_tensor = post_process(image).to(device);
    
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(input_tensor);
    output_tensor = module.forward(inputs).toTensor();
    //inputs.push_back(torch::rand({ 1, 3, 400, 400 }));
    //output_tensor = module.forward({ input_tensor }).toTensor();
    image = tensor_to_image(output_tensor);
#ifdef DEBUG
    img_path.append("examples/").append(std::to_string((int)current_time_t)).append(".png");
    save_image(img_path, image.clone());
#endif
    return image.clone();
}

void ImageProcessor::save_image(std::string& img_path, cv::Mat image) {
    std::string prefix = img_path.substr(0, img_path.find("."));
    std::string suffix = img_path.substr(img_path.find("."));

    if (std::filesystem::exists(img_path)) {
        std::string prefix = img_path.substr(0, img_path.find("."));
        std::string suffix = img_path.substr(img_path.find("."));
        prefix.append("-").append(suffix);
        save_image(prefix, image);
    }
    else {
        imwrite(img_path, image);
    }
}

/*
cv::Mat getImage(std::string& img_path){
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

void save_tensor(torch::Tensor t) {
    std::ostringstream tensorStream;
    tensorStream << t;
    std::string tensorData = tensorStream.str();
    std::ofstream outFile("tensor_data.txt");
    if (outFile.is_open()) {
        outFile << tensorData;
        outFile.close();
        std::cout << "Tensor data saved to tensor_data.txt" << std::endl;
    }
    else {
        std::cerr << "Unable to open the file." << std::endl;
    }
}*/