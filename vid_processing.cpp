#include <opencv2/opencv.hpp>
#include <iostream>
#include <condition_variable>
#include <string>
#include <vid_processing.hpp>

int sec = 10;
bool see_windows;

void change_sec(int s){
    sec = s;
}

void process_video(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv, struct cctv& info) {
    std::cout<<"getting VideoCapture of "<< info.name << "...";
    cv::VideoCapture cap(info.url);
    if (!cap.isOpened()) {
        std::cerr << "Error opening the rtsp stream." << std::endl;
        return;
    }
    std::cout<<"done."<<std::endl;
    // Create a window to display the captured frames
    cv::namedWindow(info.name, cv::WINDOW_NORMAL);
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
        cv::imshow(info.name, frame);

        // Wait for 1ms and check if the user has pressed the 'q' key
        if (cv::waitKey(1) == 'q') {
            std::cout << "quit streaming video." << std::endl;
            break;
        }

        current_frame++;
        frame_interval = fps * sec;
        if (current_frame%frame_interval == 0) {
            std::unique_lock<std::mutex> lock(mtx);
            std::cout<< sec << std::endl;
            current_frame = 0;
            q.push(frame);
            conv.notify_all();
        }
    }
    cap.release();
    cv::destroyAllWindows();
}