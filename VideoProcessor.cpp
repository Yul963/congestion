#include <opencv2/opencv.hpp>
#include <iostream>
#include <condition_variable>
#include <string>
#include <thread>
#include <VideoProcessor.hpp>

CCTV::CCTV(std::string url, std::string name, std::string location,cv::Mat base_image){
    sec = 10;
    see_windows = false;
    this->url = url;
    this->name = name;
    this->location = location;
    this->base_image = base_image;
    
    //url로부터 비디오를 받는 데 실패하면 throw
    std::cout<<"opening VideoCapture of "<< this->name << "...";
    cap.open(this->url);
    if (!cap.isOpened())
        throw std::runtime_error("Error opening the rtsp stream.");
    std::cout<<"done."<<std::endl;
}

void CCTV::change_sec(int s){
    sec = s;
}

//sec마다 비디오의 한 프레임을 큐에 추가. 추가된 프레임은 ImageProcessor::process_image에서 처리
void CCTV::process_video(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv) {
    // Create a window to display the captured frames
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    int fps = cap.get(cv::CAP_PROP_FPS);
    int frame_interval;
    int current_frame = 0;
    cv::Mat frame;
    bool ret;
    while (true) {
        ret = cap.read(frame);

        if (!ret) {
            std::cout << "Error: Unable to read the frame from the video capture" << std::endl;
            break;
        }

        if (frame.empty()) {
            std::cerr << "End of rtsp stream." << std::endl;
            break;
        }

        // Display the captured frame
        cv::imshow(name, frame);

        // Wait for 1ms and check if the user has pressed the 'q' key
        if (cv::waitKey(1) == 'q') {
            std::cout << "quit streaming video." << std::endl;
            break;
        }

        current_frame++;
        frame_interval = fps * sec;
        //sec마다 이미지 큐에 추가
        if (current_frame%frame_interval == 0) {
            std::unique_lock<std::mutex> lock(mtx);
            //std::cout<< sec << std::endl;
            current_frame = 0;
            q.push(frame);
            conv.notify_all();
        }
    }
    cap.release();
    cv::destroyAllWindows();
    return;
}