#include <iostream>
#include <ctime>
#include <locale>
#include <codecvt>
#include <string>
#include <ClientHandler.hpp>
#include <boost/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/locale.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#define CCTVNUM 2       // cctv 개수

using namespace std;        
namespace beast = boost::beast;     
namespace http = boost::beast::http;       
namespace net = boost::asio;
using tcp = net::ip::tcp;


/*
    혼잡도측정장비 boot 시
    저장된 fname으로 웹서버에 HTTP GET Request 시도,
    시설 정보를 받아와서 return

    오류 시 return empty string
    정상일 때는 return json string
*/ 
wstring initialize(string fname){
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    wstring encodedjson;
    
    try{
        std::string host = "ec2-54-180-160-31.ap-northeast-2.compute.amazonaws.com";
        std::string port = "80";
        auto const target = "/get/info/facility/?fname=";

        bool isVer1_0 = false;
        int version = isVer1_0 ? 10 : 11;

        tcp::resolver::results_type endpoints = resolver.resolve(host, port);
        auto endpoint = endpoints.begin();
        stream.connect(endpoint->endpoint());

        string urlhost = host;
        urlhost += ":";
        urlhost += port;

        //set request body 
        http::request<http::string_body> req{
            http::verb::get, target+fname, version
        };
        req.set(http::field::host, urlhost);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // send request
        http::write(stream, req);
        http::read(stream, buffer, res);

        string json = beast::buffers_to_string(res.body().data());
        std::string str = boost::locale::conv::to_utf<char>(json, "EUC-KR");
        /*
        std::istringstream iss(json);
        boost::property_tree::ptree pt;
        boost::property_tree::json_parser::read_json(iss, pt);

        std::ostringstream oss;
        boost::property_tree::json_parser::write_json(oss, pt, false);

        std::string decodedString = boost::locale::conv::from_utf<char>(oss.str(), "UTF-8");
        std::cout << "gotten value: " << decodedString << std::endl;
        //cout<<  "gotten value: " << json << endl;
        
        encodedjson = std::wstring().assign(decodedString.begin(), decodedString.end());
        wcout << "gotten value: " << encodedjson << endl;
        */
        encodedjson = boost::locale::conv::utf_to_utf<wchar_t>(str);
        //shutdown socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != beast::errc::not_connected){
            clog << "error: " << ec.message() << endl;
            encodedjson.clear();
            return encodedjson;
        }
    } catch (exception const& ex) {
        clog << "exception: " << ex.what() << endl;
        encodedjson.clear();
        return encodedjson;
    }
    return encodedjson;
}

//room 정보 parsing 함수
/*
    json 예시:
    {"ssu": 
        {
            "intro": "ssu test", 
            "addr": "j", 
            "web_addr": "j", 
            "phone_num": "01012345678", 
            "buildings": 
            {
                "\uc815\ubcf4\uacfc\ud559\uad00": 
                {
                    "intro": "\uc815\ubcf4\uc12c", 
                    "congest_lv": null, 
                    "cctvs": 
                    {
                        "1": 
                        {
                            "coverage": "406\ud638"
                        }
                    }
                }
            }
        }
    }
*/
/*
vector<class ROOM> make_rooms(wstring wjsonstr){
    vector<class ROOM> rooms;

    // std::wstring을 std::string으로 변환
    std::wstringstream wss;
    wss << wjsonstr;
    std::wstring converted = wss.str();
    std::string jsonstr(converted.begin(), converted.end());

    // boost::json::parse() 함수 호출
    boost::json::value val = boost::json::parse(jsonstr);
    
    

    return rooms;
}
*/



// @overloaded, json string 작성
string make_json_string(CongestionPair cp){
    boost::json::value jv = boost::json::value_from(cp);
    string json = boost::json::serialize(jv);
    return json;
}

/* json 예시
{
    "fname": "국립중앙박물관",
    "nametag": "cctv1",
    "value": "0.64".
    "timestamp": ""
}
*/
// @overloaded
string make_json_string(BasePair bp){        //fname = facility, nametag = cctv, value = 혼잡도값
    boost::json::value jv = boost::json::value_from(bp);
    string json = boost::json::serialize(jv);
    return json;
}

/* 
    혼잡도쌍 배열을 받아 json array화
    (cctv가 한 시설에 여러 대일 경우, 아직 카메라 2대)

boost::json::array make_json_array(CongestionPair *args){
    boost::json::array json_data = {};
    boost::json::value jv;

    for (CongestionPair c : args){
        if (c.name_tag.length() != 0){
            jv = boost::json::value_from(c);
            json_data.push_back(jv);
        }
    }
    return json_data;
}
*/

//json string을 받아 http post
int send_json(string json, bool set){
    //cout<<json<<endl;
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;

    std::string host = "ec2-54-180-160-31.ap-northeast-2.compute.amazonaws.com";
    std::string port = "80";
    auto target = "/update_congest/";
    if (set)
        target = "/set_base/";
    bool isVer1_0 = false;
    int version = isVer1_0 ? 10 : 11;

    try{
        auto const results = resolver.resolve(host, port);
        stream.connect(results);

        string urlhost = host;
        urlhost += ":";
        urlhost += port;

        //request setting
        http::request<http::string_body> req{http::verb::post, target, version};
        req.set(http::field::host, urlhost);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_type, "application/json");
        req.set(http::field::accept, "*/*");
        req.set(http::field::content_length, to_string(json.length()));
        req.set(http::field::connection, "close");
        req.body() = json;

        //post and response
        http::write(stream, req);
        http::read(stream, buffer, res);
        
        string response = beast::buffers_to_string(res.body().data());
        //cout << response << endl;
        
        beast::error_code errcode;
        stream.socket().shutdown(tcp::socket::shutdown_both, errcode);

        if (errcode && errcode != beast::errc::not_connected){
            clog << "error: " << errcode.message() << endl;
            return -1;
        }
    }
    catch (exception const& e){
        clog << "exception: " << e.what() << endl;
        return -1;
    }
    return 0;
}

// simple test function
int test(){
    CongestionPair list[CCTVNUM];

    //test code
    string facility = "ssu";
    string nametag = "1";
    double val = 0.0;
    cout << "data 입력" << endl;
    cin >> nametag >> val;
    CongestionPair a = CongestionPair(facility, nametag, val);
    
    string json = make_json_string(a);
    send_json(json, false);

    return 0;
}

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& jv, const CongestionPair& pair) {
    jv = {
        {"fname", pair.fname},
        {"nametag", pair.nametag},
        {"value", pair.value},
        {"timestamp", pair.timestamp}
    };
}

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& jv, const BasePair& pair) {
    jv = {
        {"nametag", pair.nametag},
        {"base", pair.base},
    };
}

void make_rooms(std::wstring json, vector<class ROOM>& rooms){
    boost::property_tree::wptree pt;
    std::wstringstream ss(json);
    boost::property_tree::read_json(ss, pt);

    const boost::property_tree::wptree& buildings = pt.get_child(L"ssu.buildings");
    for (const auto& building : buildings) {
        std::string buildingName = boost::locale::conv::utf_to_utf<char>(building.first);;
        
        const boost::property_tree::wptree& buildingInfo = building.second;
        std::wstring base = buildingInfo.get<std::wstring>(L"base");

        std::string base_s(base.begin(), base.end());
        std::string build(buildingName.begin(), buildingName.end());
        if(base_s == "null"){
            rooms.emplace_back(0, build);
        }
        else{
            rooms.emplace_back(stoi(base_s), build);
        }

        const boost::property_tree::wptree& cctvs = buildingInfo.get_child(L"cctvs");
        for (const auto& cctv : cctvs) {
            std::wstring cctvId = cctv.first;
            const boost::property_tree::wptree& cctvInfo = cctv.second;

            std::wstring rtspUrl = cctvInfo.get<std::wstring>(L"rtsp_url");

            std::string id(cctvId.begin(), cctvId.end());
            std::string url(rtspUrl.begin(), rtspUrl.end());

            rooms.back().add_cctv(url, id);
        }
    }
}