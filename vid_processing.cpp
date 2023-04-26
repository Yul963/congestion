#include <opencv2/opencv.hpp>
#include <iostream>
#include <condition_variable>
#include <string>

void process_video(std::queue<cv::Mat>& q, std::mutex& mtx, std::condition_variable& conv, std::string& url) {
    cv::VideoCapture cap(url);
    if (!cap.isOpened()) {
        std::cerr << "Error opening the rtsp stream." << std::endl;
        exit(0);
    }

    // Create a window to display the captured frames
    cv::namedWindow("RTSP Stream", cv::WINDOW_NORMAL);
    int fps = cap.get(cv::CAP_PROP_FPS);
    int frame_interval = fps * 60;
    int current_frame = 0;

    while (true) {
        cv::Mat frame;
        bool ret = cap.read(frame);

        if (!ret) {
            std::cout << "Error: Unable to read the frame from the video capture" << std::endl;
            break;
        }

        if (frame.empty()) {
            std::cerr << "End of rtsp stream." << std::endl;
            break;
        }

        // Display the captured frame
        cv::imshow("RTSP Stream", frame);

        // Wait for 1ms and check if the user has pressed the 'q' key
        if (cv::waitKey(1) == 'q') {
            break;
        }

        current_frame++;
        if (current_frame%frame_interval == 0) {
            std::unique_lock<std::mutex> lock(mtx);
            q.push(frame);
            conv.notify_all();
        }
    }
    cap.release();
    cv::destroyAllWindows();
}