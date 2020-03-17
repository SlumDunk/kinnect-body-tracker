#pragma once
using namespace std;
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
#include <string>
class WebsocketClient
{
public:
	void openConnection();
	void closeConnection();
	void sendMsg(string msg);
	string receiveMsg();
	void handleMsg(string msg);
	WebsocketClient(string host, string port, string url);

private:
	string host;
	string url;
	string port;
	// The io_context is required for all I/O
	net::io_context ioc;

	// These objects perform our I/O
	tcp::resolver resolver{ ioc };
	websocket::stream<tcp::socket> ws{ ioc };
};

