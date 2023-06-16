#include <iostream>
#include <ctime>
#include <ClientHandler.hpp>
#include <boost/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

// webserver url로 수정 필요 (현재 json response test api url)
#define HOST "jsonplaceholder.typicode.com" 
#define CCTVNUM 2

using namespace std;        
namespace beast = boost::beast;     
namespace http = boost::beast::http;       
namespace net = boost::asio;
using tcp = net::ip::tcp;

// @overloaded, json string 작성
string make_json_string(CongestionPair cp){
    boost::json::value jv = boost::json::value_from(cp);
    string json = boost::json::serialize(jv);
    return json;
}
// @overloaded
/*
{
    "fname": "국립중앙박물관",
    "nametag": "cctv1",
    "value": "0.64".
    "timestamp": ""
}
*/

string make_json_string(string fname, string nametag, double value){        //fname = facility, nametag = cctv, value = 혼잡도값
    struct CongestionPair cp = CongestionPair(fname, nametag, value);
    boost::json::value jv = boost::json::value_from(cp);
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
int send_json(string json){
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;

    auto const host = HOST;
    auto const port = "80";
    auto const target = "/posts";
    bool isVer1_0 = false;
    int version = isVer1_0 ? 10 : 11;

    try{
        auto const results = resolver.resolve(host, port);
        stream.connect(results);

        string urlhost = HOST;
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
        cout << response << endl;
        
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
    string facility = "국립중앙박물관";
    string nametag = "";
    double val = 0.0;
    cout << "data 입력" << endl;
    cin >> nametag >> val;
    CongestionPair a = CongestionPair(facility, nametag, val);
    
    string json = make_json(a);
    send_json(json);

    return 0;
}
