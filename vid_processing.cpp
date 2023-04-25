#include <opencv2/opencv.hpp>
#include <iostream>

int vid() {

    cv::VideoCapture cap("rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0");
    if (!cap.isOpened()) {
        std::cerr << "Error opening the rtsp stream." << std::endl;
        return -1;
    }

    // Create a window to display the captured frames
    cv::namedWindow("RTSP Stream", cv::WINDOW_NORMAL);

    // Start capturing and processing frames
    cv::Mat frame;
    while (true) {
        // Read a new frame from the rtsp stream
        cap.read(frame);

        // Check if the frame is empty
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
    }

    // Release the resources
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
