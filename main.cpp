#include <torch/torch.h>
#include <torch/script.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>
#include <congestion_cal.hpp>
#include <vid_processing.hpp>

using namespace std;
using namespace cv;

int main() {
    string img_path = "image.jpg";
    Mat image = getImage(img_path);
    
    torch::NoGradGuard no_grad;
    torch::Tensor input_tensor = post_process(image);

    string model_path = "model_scripted.pt";
    torch::jit::script::Module module = load_model(model_path);

    at::Tensor output = module.forward({input_tensor}).toTensor();

    image = tensor_to_image(output, 1);

    // 이미지를 저장합니다.
    imwrite("output.png", image);

    vid();

    return 0;
}