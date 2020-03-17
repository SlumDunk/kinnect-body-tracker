#include "WebsocketClient.h"

WebsocketClient::WebsocketClient(string host, string port, string url) {
	this->host = host;
	this->port = port;
	this->url = url;
}

void WebsocketClient::openConnection() {


	// Look up the domain name
	auto const results = resolver.resolve(host, port);

	// Make the connection on the IP address we get from a lookup
	net::connect(ws.next_layer(), results.begin(), results.end());

	// Set a decorator to change the User-Agent of the handshake
	ws.set_option(websocket::stream_base::decorator(
		[](websocket::request_type& req)
		{
			req.set(http::field::user_agent,
				std::string(BOOST_BEAST_VERSION_STRING) +
				" websocket-client-coro");
		}));

	// Perform the websocket handshake
	ws.handshake(host, url);

}

void WebsocketClient::closeConnection() {
	// Close the WebSocket connection
	ws.close(websocket::close_code::normal);
}

void WebsocketClient::handleMsg(string msg) {

}

string WebsocketClient::receiveMsg() {
	// This buffer will hold the incoming message
	beast::flat_buffer buffer;

	// Read a message into our buffer
	ws.read(buffer);
	return beast::buffers_to_string(buffer.cdata());


}

void WebsocketClient::sendMsg(string msg) {
	// Send the message
	ws.write(net::buffer(std::string(msg)));
	string msg = receiveMsg();
}