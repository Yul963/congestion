#include <VideoProcessor.hpp>

CCTV::CCTV(std::string url, int cctv_num, std::string location) {
    this->url = url;
    this->cctv_num = cctv_num;
    this->location = location;
    threadExitFlag = false;;
    stop_flag = false;
    //window = false;
    //url로부터 비디오를 받는 데 실패하면 throw
    std::cout<<"opening VideoCapture of "<< this->cctv_num << "..." << std::endl;
    if (!cap.open(this->url)) {
        throw std::runtime_error("Error opening the RTSP stream.");
    }
    std::cout<<"done."<<std::endl;
}

void CCTV::set_stop(){
    stop_flag = true;
}

cv::Mat CCTV::get_current_frame(){
    return frame.clone();
}
/*
void CCTV::see_window(){
    if(frame.empty())
        std::cout<<"stream is not active."<<std::endl;
    else
        window = true;
}*/

void CCTV::process_video() {//영상을 계속 받아 frame 업데이트
    int fps = cap.get(cv::CAP_PROP_FPS);
    int frame_interval;
    bool ret;
    //bool is_opened=false;
    while (!stop_flag) {
        ret = cap.read(frame);
        if (!ret) {
            std::cout << "Error: Unable to read the frame from the video capture" << std::endl;
            break;
        }
        if (frame.empty()) {
            std::cerr << "End of rtsp stream." << std::endl;
            break;
        }
        /*
        if(window){
            if(!is_opened){
                cv::namedWindow(location + "-" + std::to_string(cctv_num), cv::WINDOW_NORMAL);
                is_opened = true;
            }
            cv::imshow(location + "-" + std::to_string(cctv_num), frame);
            if (cv::waitKey(1) == 'q') {
                std::cout << "quit showing video." << std::endl;
                cv::destroyAllWindows();
                is_opened = false;
                window = false;
            }
        }
        */
    }
    cap.release();
    //if(is_opened)
    //    cv::destroyAllWindows();
    threadExitFlag = true;
    return;
}

int CCTV::get_num(){
    return cctv_num;
}

bool CCTV::get_flag()
{
    return threadExitFlag;
}

ROOM::ROOM(int base, std::string location, int room_num){
    if (base==0){
        std::cout<<location<<" base not set."<<std::endl;
    }
    this->congestion = 0.;
    this->base = base;
    this->location = location;
    this->room_num = room_num;
}

std::vector<cv::Mat> ROOM::get_target_images(){
    std::vector<cv::Mat> images;
    for (auto& cctv : cctvs){
#ifdef DEBUG
        std::cout<<std::endl<<"get frame of "<< location << "-" << cctv.get_num() << std::endl;
#endif
        images.emplace_back(cctv.get_current_frame());
    }
    return images;
}

std::vector<class CCTV> ROOM::get_cctvs(){
    return cctvs;
}

std::string ROOM::get_location(){
    return location;
}

void ROOM::add_cctv(std::string url, int cctv_num, std::string location){
    try {
        cctvs.emplace_back(url, cctv_num, location);
    } catch (const std::runtime_error& e) {
        std::cout << "Exception : " << e.what() << std::endl;
    }
}

void ROOM::cal_congestion(std::vector<cv::Mat> target_images){
    int target=0;
    for (auto& image : target_images){     
#ifdef DEBUG
        std::cout << "cal_congestion: " << cv::countNonZero(image) << std::endl;
#endif
        target+=cv::countNonZero(image);
    }
    congestion = target >= base ? 0.0f : (float)(base-target)/(float)base;
}

double ROOM::get_congestion(){
    return congestion;
}

void ROOM::set_base(std::vector<cv::Mat>& base_images){
    base=0;
    for (auto& image : base_images){
#ifdef DEBUG
        int numChannels = image.channels();
        std::cout <<"set_base: " <<  cv::countNonZero(image) << std::endl;
#endif
        base+=cv::countNonZero(image);
    }
    if (base==0)
        std::cout<<"base_image countNonZero returned 0. check base image."<<std::endl;
}

int ROOM::get_base(){
    return base;
}

int ROOM::get_room_number()
{
    return room_num;
}

void ROOM::run_threads(){
    for (auto& cctv : cctvs){
        threads.emplace_back([&]() { cctv.process_video(); });
    }
    for (auto& t : threads)
        t.detach();
}

void ROOM::stop_threads(){
    for (auto& cctv : cctvs){
        cctv.set_stop();
        while (!cctv.get_flag()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
#ifdef DEBUG
        std::cout << "cctv thread terminated." << std::endl;
#endif
    }
}
/*
void ROOM::show_cctvs(){
    for (auto& cctv : cctvs){
        cctv.see_window();
    }
}*/

