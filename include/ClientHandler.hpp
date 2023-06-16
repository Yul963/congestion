#include <iostream>
#include <ctime>
#include <boost/json.hpp>

#define TIME_MAXSIZE = 80;

using namespace std;

// 혼잡도 정보를 처리하기 위한 사용자 정의 클래스
struct CongestionPair{
    string fname;
    string nametag;
    double value;
    string timestamp;

    CongestionPair(){
        fname = "";
        nametag = "";
        value = 0.0;
    }

    CongestionPair(string facility_name, string name_tag, double cngst_value){
    this->fname = facility_name;
    this->nametag = name_tag;
    this->value = cngst_value;
    strftime(this->timestamp, TIME_MAXSIZE, "%Z-%m-%d %H:%M:%S", time(NULL));    //2023-06-16 13:26:01
    }
};

string make_json_string(CongestionPair);
string make_json(string, string, double);
boost::json::array make_json_array(CongestionPair*);
int send_json(string);
