#include <torch/torch.h>
#include <torch/script.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>

using namespace std;
using namespace cv;

int main() {
// 이미지 파일 경로 설정
    string img_path = "image.jpg";
    if (!ifstream(img_path).good()) {
        cout << "File not found: " << img_path << endl;
            return -1;
    }
    Mat image = imread(img_path, IMREAD_COLOR);
    if (image.empty()) {
            cout << "Error: Unable to load image" << endl;
            return -1;
    }

    // 이미지 전처리
    cvtColor(image, image, cv::COLOR_BGR2RGB);
    resize(image, image, cv::Size(400, 400));

    // MEAN, STD 값으로 정규화
    float MEAN[] = {0.48897059, 0.46548275, 0.4294};
    float STD[] = {0.22861765, 0.22948039, 0.24054667};

    cv::Mat float_image;
    image.convertTo(float_image, CV_32FC3, 1.0 / 255);
    float_image -= cv::Scalar(MEAN[0], MEAN[1], MEAN[2]);
    float_image /= cv::Scalar(STD[0], STD[1], STD[2]);

    // 텐서 변환
    torch::Tensor input_tensor = torch::from_blob(float_image.data, {1, 400, 400, 3}).permute({0, 3, 1, 2});
    input_tensor = input_tensor.to(torch::kCUDA);

    // 토치스크립트 모델 로드
    string model_path = "model_scripted.pt";
    torch::jit::script::Module module;
    try {
        module = torch::jit::load(model_path);
    }
    catch (const c10::Error& e) {
        cerr << "Error loading the model\n";
        return -1;
    }

    // 모델 실행
    at::Tensor output = module.forward({input_tensor}).toTensor();

    // softmax 함수를 적용하여 확률값으로 변환
    output = torch::softmax(output.squeeze(0), 0).argmax(0).cpu().to(torch::kUInt8);

    cv::Mat mask_image(output.size(0), output.size(1), CV_8UC1, output.data_ptr());

    cv::multiply(mask_image, cv::Scalar(255), mask_image);

    // 이미지를 저장합니다.
    cv::imwrite("output.png", mask_image);

    return 0;
}