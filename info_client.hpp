#ifndef INFO_CLIENT
#define INFO_CLIENT
#include "info_receiver.hpp"
#include <nlohmann/json.hpp>
#include <simdjson.h>
#include <string>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
using CLIENT = websocketpp::client<websocketpp::config::asio_tls_client>;
using CONTEXT_PTR = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;
inline CONTEXT_PTR on_tls_init(const char *hostname, websocketpp::connection_hdl) {
    CONTEXT_PTR ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);


        ctx->set_verify_mode(boost::asio::ssl::verify_none);
    } catch (std::exception &e) {
        std::cout << std::string(e.what()) << std::endl;
    }
    return ctx;
}


template<typename T>
class InfoClient : public InfoReceiver<T> {
public:
    using InfoReceiver<T>::get_subscribe_message;
    using InfoReceiver<T>::process_message;
    explicit InfoClient(const std::vector<std::string> &symbols, std::string url);
    std::string url;
private:
    CLIENT client;
    void on_open(websocketpp::connection_hdl hdl) {
        auto subscribe_message = get_subscribe_message();
        client.send(hdl, subscribe_message.dump(), websocketpp::frame::opcode::text);
    }
    void on_message(websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
        //Websocket 服务器每3分钟发送Ping消息, 回复pong消息保持心跳
        if (msg->get_opcode() == websocketpp::frame::opcode::ping) {
            std::string payload = msg->get_payload();
            // 直接使用收到的payload回复Pong
            try {
                client.pong(hdl, payload);
            } catch (const websocketpp::lib::error_code &e) {
                std::cerr << "Pong reply failed: " << e.message() << std::endl;
            }
        } else {
            std::string_view json_sv = msg->get_payload();
            process_message(json_sv);
        }
    }
public:
    void run() {
        websocketpp::lib::error_code ec;
        CLIENT::connection_ptr con = client.get_connection(url, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return;
        }
        con->set_open_handshake_timeout(10000);// 10秒
        client.connect(con);
        client.run();
    }
};

template<typename T>
InfoClient<T>::InfoClient(const std::vector<std::string> &symbols, std::string url) : InfoReceiver<T>(symbols), url(url) {
    using websocketpp::lib::bind;
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    client.set_tls_init_handler(bind(&on_tls_init, url.c_str(), _1));
    client.set_open_handler(bind(&InfoClient::on_open, this, _1));
    client.set_message_handler(bind(&InfoClient::on_message, this, _1, _2));
    client.init_asio();
};

#endif
