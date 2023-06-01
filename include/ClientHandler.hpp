#include <iostream>

#include <boost/json.hpp>

using namespace std;

// 혼잡도 정보를 처리하기 위한 사용자 정의 클래스
struct CongestionPair{
    string facility_name;
    string name_tag;
    double cngst_value;

    CongestionPair(){
        facility_name = "";
        name_tag = "";
        cngst_value = 0.;
    }

    CongestionPair(string facility_name, string name_tag, double cngst_value){
    this->facility_name = facility_name;
    this->name_tag = name_tag;
    this->cngst_value = cngst_value;
    }
};

string make_json_string(CongestionPair);
string make_json(string, string, double);
boost::json::array make_json_array(CongestionPair*);
int send_json(string);
