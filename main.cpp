#include <condition_variable>
#include <ImageProcessor.hpp>
#include <VideoProcessor.hpp>
#include <ClientHandler.hpp>
#ifdef WIN32
#endif

#define DURATION_SEC 5

using namespace std;
using namespace cv;

mutex mtx;
vector<class ROOM> rooms;
bool stop = false, threadExitFlag = false;
string fac;
ImageProcessor* ImgP;

void status(){
    int input,i=0;
    bool is_empty = false;
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
                cout<<"\t\t"<<cctv.get_num()<<endl;
            }
        }
        i=0;
        cout<<"input room index to set base: ";
        cin>>input;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if(input>=0 && input<rooms.size()){
            vector<Mat> images, temp;
            auto& room = rooms[input];
            //room.show_cctvs();
            //cout<<"press q to shut CCTV"<<endl;
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
#ifdef DEBUG
                        cout<<"is empty: "<<image.empty()<<endl;
#endif
                        temp.push_back(ImgP->process_image(image.clone()));
#ifdef DEBUG
                        cout<<"process image."<<endl;
#endif
                    }
                    else{
                        is_empty = true;
#ifdef DEBUG
                        cout<<"image is empty."<<endl;
#endif
                        break;
                    }
                    
                }
                if(!is_empty){
#ifdef DEBUG
                    cout << "number of images: " << images.size() << endl;
#endif
                    room.set_base(temp);
                    BasePair bp = BasePair(room.get_room_number(), room.get_base());
                    string json = make_json_string(bp);
                    send_json(json, true);
                    cout<<room.get_location()<<" base set."<<endl;
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

void work_thread(){
    chrono::time_point<chrono::system_clock> start_time, end_time;
    chrono::duration<double> elapsed_seconds;
    vector<Mat> images, temp;
    bool is_empty=false;
    int i=0,j=0;
    while(!stop){
        start_time = chrono::system_clock::now();
        temp.clear();
        images.clear();
        for (auto& room : rooms){
            images = room.get_target_images();
#ifdef DEBUG
            cout << "number of images: " << images.size() << endl;
#endif    
            for(auto& image : images){
                if (stop)
                    break;
                if(!image.empty()){
                    unique_lock<mutex> lock(mtx);
#ifdef DEBUG
                    cout << "is empty: " << image.empty() << endl;
#endif          
                    temp.push_back(ImgP->process_image(image.clone()));
                }
                else{
                    is_empty = true;
#ifdef DEBUG
                    cout<<"image is empty."<<endl; 
#endif  
                    break;
                }
                i++;
            }
            if(!is_empty){
                room.cal_congestion(temp);
                string location = room.get_location();
                double congestion = room.get_congestion();
                CongestionPair cp = CongestionPair(room.get_room_number(), fac, congestion);
#ifdef DEBUG
                cout << "congestion: " << congestion << endl;
#endif
                string json = make_json_string(cp);
                send_json(json, false);
            }
            else
                is_empty=false;
            if (stop)
                    break;
        }
        end_time = chrono::system_clock::now();
        elapsed_seconds = end_time - start_time;
#ifdef DEBUG
        cout << "Elapsed time: " << elapsed_seconds.count() << "s" << endl;
#endif
        if(chrono::duration_cast<chrono::seconds>(elapsed_seconds) < chrono::seconds(DURATION_SEC)){
            auto remainingTime = chrono::seconds(DURATION_SEC) - chrono::duration_cast<chrono::seconds>(elapsed_seconds);
#ifdef DEBUG
            cout << "sleep thread for " << remainingTime.count() << "s" << endl;
#endif
            this_thread::sleep_for(remainingTime);
        }
    }
    threadExitFlag = true;
}

int main() {
#ifdef WIN32
#endif
#ifdef DEBUG
    cout << "Debug mode" << endl;
#endif
    try {
        ImgP = new ImageProcessor();
    }
    catch(exception& e) {
        cout << "Exception : " << e.what() << endl << "exit program."<< endl;
        return 0;
    }
    
    std::cout << "input facility name: ";
    std::getline(std::cin, fac);

    cout << "initializing..." << endl;
    wstring roomstring = initialize(fac);
    if (roomstring.compare(L"fname required")==0) {
        cout << "theres no facility named " << fac << endl;
        return 0;
    }
    cout << "done." << endl;

    try {
        make_rooms(roomstring, fac, rooms);
    } catch(exception& e){
        cout << "Exception : " << e.what() << endl;
    }

    if (rooms.empty()) {
        cout << "no rooms data" << endl;
        return 0;
    }

#ifdef DEBUG
    cout << "run threads" << endl;
#endif
    for (auto& room : rooms)
            room.run_threads();

    /*
    try {
        rooms.emplace_back(0, "room1");
        //cout << cv::getBuildInformation();
        for (auto& room : rooms){
            room.add_cctv("rtsp://dbfrb963:dbfrb9786@192.168.1.5:554/stream_ch00_0", "1");
            room.add_cctv("rtsp://dbfrb963:dbfrb9786@192.168.1.4:554/stream_ch00_0", "2");
            room.run_threads();
        }
    } catch (const runtime_error& e) {
        cout << "Exception : " << e.what() << endl;
    }*/

    thread work(&work_thread);
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
                status();
                break;
            case 2:
                cout << "Shutting down the program..." << endl;
                stop = true;
                while (!threadExitFlag) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
#ifdef DEBUG
                cout << "work thread terminated." << endl;
#endif
                for (auto& room : rooms) {
                    room.stop_threads();
                }
                    
                delete ImgP;
                return 0;
            default:
                cout<<"input valid numbers(1~2)"<<endl; 
                break;
            }
    }
}