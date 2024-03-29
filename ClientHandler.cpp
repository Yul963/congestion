#include <ClientHandler.hpp>

using namespace std;        
namespace beast = boost::beast;     
namespace http = boost::beast::http;       
namespace net = boost::asio;
using tcp = net::ip::tcp;

wstring replaceUnicodeEscape(wstring& sequence) {
    wstring result;
    for (size_t i = 0; i < sequence.length(); ++i) {
        if (sequence[i] == L'\\' && i + 1 < sequence.length() && sequence[i + 1] == L'u') {
            if (i + 5 < sequence.length()) {
                wstring unicodeCode = sequence.substr(i + 2, 4);
                int unicodeValue = stoi(unicodeCode, nullptr, 16);
                result += static_cast<wchar_t>(unicodeValue);
                i += 5;
            }
            else {
                result += sequence[i];
            }
        }
        else {
            result += sequence[i];
        }
    }
    return result;
}

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
        std::string host = "3.37.73.70";
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
        encodedjson = boost::locale::conv::utf_to_utf<wchar_t>(json);
        encodedjson = replaceUnicodeEscape(encodedjson);
        
        //string str = boost::locale::conv::utf_to_utf<char>(encodedjson);
        //string EUC = boost::locale::conv::between(str, "EUC-KR", "UTF-8");

#ifdef DEBUG
        locale::global(locale("kor"));
        wcout << "encodedjson value: " << encodedjson << endl;
#endif
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
string make_json_string(BasePair bp){
    boost::json::value jv = boost::json::value_from(bp);
    string json = boost::json::serialize(jv);
    return json;
}

//json string을 받아 http post
int send_json(string json, bool set){
#ifdef DEBUG
    cout<<json<<endl;
#endif
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;

    std::string host = "3.37.73.70";
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
#ifdef DEBUG
        cout << response << endl;
#endif
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
/*
void tag_invoke(const boost::json::value_from_tag&, boost::json::value& jv, const Pair& pair) {
    jv = {
        {"nametag", pair.nametag},
    };
}*/

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

void make_rooms(std::wstring json,std::string fac, vector<class ROOM>& rooms){
    boost::property_tree::wptree pt;
    wstringstream ss(json);
    boost::property_tree::read_json(ss, pt);

    string EUC = boost::locale::conv::between(fac, "UTF-8", "EUC-KR");
    wstring wideString = boost::locale::conv::utf_to_utf<wchar_t>(EUC);

    //string str = boost::locale::conv::utf_to_utf<char>(encodedjson);
    //string EUC = boost::locale::conv::between(str, "EUC-KR", "UTF-8");

    const boost::property_tree::wptree& root = pt.get_child(wideString);
    const boost::property_tree::wptree& buildings = root.get_child(L"buildings");
    for (const auto& building : buildings) {
        wstring buildingName = boost::locale::conv::utf_to_utf<wchar_t>(building.first);;
        
        const boost::property_tree::wptree& buildingInfo = building.second;
        wstring base = buildingInfo.get<std::wstring>(L"base");

        string base_s(base.begin(), base.end());
        string build = boost::locale::conv::utf_to_utf<char>(buildingName);
        build = boost::locale::conv::between(build, "EUC-KR", "UTF-8");
        wstring build_num = buildingInfo.get<std::wstring>(L"id");
        wcout << "build_num: " << build_num << endl;
        if(base_s == "null"){
            rooms.emplace_back(0, build, stoi(build_num));
        }
        else{
            rooms.emplace_back(stoi(base_s), build, stoi(build_num));
        }
        
        const boost::property_tree::wptree& cctvs = buildingInfo.get_child(L"cctvs");
        for (const auto& cctv : cctvs) {
            wstring cctvId = cctv.first;
            const boost::property_tree::wptree& cctvInfo = cctv.second;

            wstring rtspUrl = cctvInfo.get<wstring>(L"rtsp_url");

            string id = boost::locale::conv::utf_to_utf<char>(cctvId);
            id = boost::locale::conv::between(id, "EUC-KR", "UTF-8");

            string url = boost::locale::conv::utf_to_utf<char>(rtspUrl);
            url = boost::locale::conv::between(url, "EUC-KR", "UTF-8");

            rooms.back().add_cctv(url, stoi(id), build);
        }
    }
}