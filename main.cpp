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

#define DURATION_SEC 20

using namespace std;
using namespace cv;

mutex mtx;
vector<class ROOM> rooms;
bool stop = false;

void status(ImageProcessor* ImgP){
    int input,i=0;
    bool is_empty=false;
    while(1){
        cout<<"<Location list>"<<endl;
        cout << fixed;
        cout.precision(3);

        for (auto& room : rooms){
            cout<<"\t"<<i++<<". "<<room.get_location()
            <<"  Congestion: "<<room.get_congestion()
            <<"|  Base value: "<<room.get_base()<<endl
            <<"\t<CCTV list>"<<endl;
            for(auto& cctv : room.get_cctvs()){
                cout<<"\t\t"<<cctv.get_name()<<endl;
            }
        }
        i=0;
        cout<<"input room index to set base: ";
        cin>>input;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if(input>=0 && input<rooms.size()){
            vector<Mat> images;
            auto& room = rooms[input];
            room.show_cctvs();
            cout<<"press q to shut CCTV"<<endl;
            cout<<"1: set base"<<endl<<"2: back to main"<<endl<<"input: ";
            cin>>input;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            switch (input)
            {
            case 1:
                images = room.get_target_images();
                for(auto& image : images){
                    if(!image.empty()){
                        unique_lock<mutex> lock(mtx);
                        cout<<"is empty: "<<image.empty()<<endl;
                        ImgP->process_image(image);
                        cout<<"process image."<<endl;
                    }
                    else{
                        is_empty = true;
                        //cout<<"image is empty."<<endl; 
                        break;
                    }
                    
                }
                if(!is_empty){
                    cout<<"number of images: "<<images.size()<<endl;
                    room.set_base(images);
                    cout<<room.get_location()<<" base set."<<endl;
                    //base 값 전송하는 부분 필요
                    //////

                    //////
                    //추가
                }
                return;
            case 2:
                return;
            default:
                cout<<"input valid numbers(1~2)"<<endl; 
                break;
            }
        }
        else{
            cout<<"input valid index(0~"<< rooms.size()-1 <<")"<<endl;
            break;
        }
        
    }
}

void work_thread(vector<class ROOM>& rooms, ImageProcessor* ImgP){
    chrono::time_point<chrono::system_clock> start_time, end_time;
    chrono::duration<double> elapsed_seconds;
    vector<Mat> images;
    bool is_empty=false;
    while(!stop){
        start_time = chrono::system_clock::now();
        for (auto& room : rooms){
            images = room.get_target_images();
            cout<<"number of images: "<<images.size()<<endl;
            for(auto& image : images){
                if (stop)
                    break;
                if(!image.empty()){
                    unique_lock<mutex> lock(mtx);
                    cout<<"is empty: "<<image.empty()<<endl;
                    ImgP->process_image(image);
                }
                else{
                    is_empty = true;
                    //cout<<"image is empty."<<endl; 
                    break;
                }
            }
            if(!is_empty){
                room.cal_congestion(images);
                //congestion 값 전송하는 부분 필요
                //////
                CongestionPair cp = CongestionPair("ssu"/*기관명*/, "1"/*room name, 약속한대로 1*/, room.get_congestion());
                string json = make_json_string(cp);
                send_json(json);
                //////
                //추가
            }
            else
                is_empty=false;
            if (stop)
                    break;
        }
        end_time = chrono::system_clock::now();
        elapsed_seconds = end_time - start_time;
        //cout << "Elapsed time: " << elapsed_seconds.count() << "s"<< endl;
        if(chrono::duration_cast<chrono::seconds>(elapsed_seconds) < chrono::seconds(DURATION_SEC)){
            auto remainingTime = chrono::seconds(DURATION_SEC) - chrono::duration_cast<chrono::seconds>(elapsed_seconds);
            //cout<<"sleep thread for "<< remainingTime.count() << "s"<<endl;
            this_thread::sleep_for(remainingTime);
        }
    }
}

int main() {
    //서버 인증하는 부분 필요
    //////
    //////
    //추가
    ImageProcessor *ImgP;
    try {
        ImgP = new ImageProcessor();
    }
    catch(exception& e) {
        cout << "Exception : " << e.what() << endl << "exit program."<< endl;
        return 0;
    }

    //서버 데이터베이스로부터 ROOM, CCTV 정보를 읽어와서 rooms, cctvs 생성하는 코드 필요
    //////
    wstring roomstring = initialize("ssu");      //facility name = ssu로 고정
    // rooms = make_rooms(roomstring);
    //////
    //추가
    try {
        rooms.emplace_back(0, "room1");
        for (auto& room : rooms){
            room.add_cctv("rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0", "cctv1");
            room.add_cctv("rtsp://dbfrb963:dbfrb9786@192.168.1.10:554/stream_ch00_1", "cctv2");
            room.run_threads();
        }
    } catch (...) {

    }

    thread work(&work_thread, ref(rooms), ref(ImgP));
    work.detach();

    int input;
    while(1){
        cout<<"1. see current status & set base"<<endl
        <<"2. quit"<<endl
        <<"input: ";
        cin>>input;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        switch (input)
            {
            case 1:
                status(ImgP);
                break;
            case 2:
                stop = true;
                for (auto& room : rooms){
                    room.stop_threads();
                }
                return 0;
            default:
                cout<<"input valid numbers(1~2)"<<endl; 
                break;
            }
    }
}