#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <vector>
#include <ImageProcessor.hpp>
#include <VideoProcessor.hpp>
#include <ClientHandler.hpp>

#define DURATION_SEC 120

using namespace std;
using namespace cv;

vector<class ROOM> rooms;

void status(){

}

void cctv_management(){

}

bool stop = false;

void work_thread(vector<class ROOM>& rooms, ImageProcessor* ImgP){
    chrono::time_point<chrono::system_clock> start_time, end_time;
    chrono::duration<double> elapsed_seconds;

    start_time = chrono::system_clock::now();
    while(!stop){
        
        for (auto& room : rooms){
            room.get_target_images();
            for(auto& image : room.get_target_images()){
                if (stop)
                    break;
                ImgP->process_image(image);
            }
            if (stop)
                    break;
        }
        end_time = chrono::system_clock::now();
        elapsed_seconds = end_time - start_time;
        cout << "Elapsed time: " << elapsed_seconds.count() << "s"<< endl;
        if(chrono::duration_cast<chrono::seconds>(elapsed_seconds) < chrono::seconds(DURATION_SEC)){
            auto remainingTime = chrono::seconds(DURATION_SEC) - chrono::duration_cast<chrono::seconds>(elapsed_seconds);
            this_thread::sleep_for(remainingTime);
        }
    }
}

int main() {
    cout << "PyTorch version: "
    << TORCH_VERSION_MAJOR << "."
    << TORCH_VERSION_MINOR << "."
    << TORCH_VERSION_PATCH << endl;

    ImageProcessor *ImgP;
    try {
        ImgP = new ImageProcessor();
    }
    catch(exception& e) {
        cout << "Exception : " << e.what() << endl << "exit program."<< endl;
        return 0;
    }

    //데이터베이스로부터 ROOM, CCTV 정보를 읽어 객체 생성하는 코드 필요
    try {
        rooms.emplace_back(0, "room1");
        for (auto& room : rooms){
            room.add_cctv("rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0", "cctv1");

            room.run_threads();
        }
    } catch (...) {

    }

    thread work(&work_thread, ref(rooms), ref(ImgP));
    work.detach();

    int input;
    while(1){
        cout<<"1. see current status"<<endl
        <<"4. quit"<<endl
        <<"input: ";
        cin>>input;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if(input>0 && input<5){
            switch (input)
            {
            case 1:
                status();
                break;
            case 2:
                cctv_management();
                break;
            case 4:
                /*
                ImgP->set_stop();
                add_junk_q();
                delete ImgP;
                */
               stop = true;
                for (auto& room : rooms){
                    room.stop_threads();
                }
                return 0;
            default:
                break;
            }
        }else
            cout<<"input valid numbers(1~4)"<<endl; 
    }
}