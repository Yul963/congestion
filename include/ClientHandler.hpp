#include <ctime>
#include <boost/json.hpp>
#include <VideoProcessor.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/locale.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <locale>
#include <codecvt>

#define TIME_MAXSIZE 80

using namespace std;

// 혼잡도 정보를 처리하기 위한 사용자 정의 클래스
struct Pair {
public:
    int nametag;
    Pair() {
        nametag = 0;
    }
    Pair(int name_tag) {
        nametag = name_tag;
    }
};

struct CongestionPair : public Pair {
public:
    std::string fname;
    double value;
    std::time_t timestamp;

    CongestionPair() : Pair(){
        fname = "";
        value = 0.0;
        timestamp = 0;
    }

    CongestionPair(int name_tag, const std::string& facility_name, double cngst_value) : Pair(name_tag) {
        fname = facility_name;
        value = cngst_value;
        timestamp = time(NULL);
    }
};

struct BasePair : public Pair {
public:
    int base;
    BasePair() : Pair() {
        base = 0;
    }
    BasePair(int name_tag, int base_value) : Pair(name_tag) {
        base = base_value;
    }
};

wstring replaceUnicodeEscape(wstring& sequence);
wstring initialize(string);
string make_json_string(CongestionPair);
string make_json_string(BasePair);
int send_json(string, bool);
void make_rooms(std::wstring json, std::string fac, vector<class ROOM>& rooms);