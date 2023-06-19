#include <iostream>
#include <ctime>
#include <string>
#include <boost/json.hpp>
#include <VideoProcessor.hpp>

#define TIME_MAXSIZE 80

using namespace std;

// 혼잡도 정보를 처리하기 위한 사용자 정의 클래스
struct CongestionPair {
    std::string fname;
    std::string nametag;
    double value;
    time_t timestamp;

    CongestionPair() {
        fname = "";
        nametag = "";
        value = 0.0;
        timestamp = 0;
    }

    CongestionPair(const std::string& facility_name, const std::string& name_tag, double cngst_value) {
        fname = facility_name;
        nametag = name_tag;
        value = cngst_value;
        timestamp = time(NULL);
    }
};

struct BasePair{
    std::string nametag;
    int base;
    BasePair(){
        base = 0;
        nametag = "";
    }
    BasePair(const std::string& name_tag, int base_value){
        base = base_value;
        nametag = name_tag;
    }
};

wstring initialize(string);
string make_json_string(CongestionPair);
string make_json_string(BasePair);
boost::json::array make_json_array(CongestionPair*);
int send_json(string, bool);
void tag_invoke(const boost::json::value_from_tag&, boost::json::value& jv, const CongestionPair& pair);
void make_rooms(std::wstring json, vector<class ROOM>& rooms);