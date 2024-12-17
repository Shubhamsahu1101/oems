#include "WebSocketServer.h"

WebSocketServer::WebSocketServer(uint16_t PORT) : PORT(PORT), isListening(false) {
  // Initialize ASIO
  ws_server.init_asio();

  // Register handlers
  ws_server.set_open_handler([this](connection_hdl hdl) { onOpen(hdl); });
  ws_server.set_message_handler([this](connection_hdl hdl, server::message_ptr msg) { onMessage(hdl, msg); });
  ws_server.set_close_handler([this](connection_hdl hdl) { onClose(hdl); });

  // Mute all logs
  ws_server.clear_access_channels(websocketpp::log::alevel::all);
}


void WebSocketServer::run() {
  ws_server.listen(PORT);
  ws_server.start_accept();
  std::cout << "WebSocket server running on port " << PORT << std::endl;
  isListening = true;
  ws_server.run();
}

void WebSocketServer::stop() {
  isListening = false;
  
  std::lock_guard<std::mutex> lock(subscribers_mutex);
  for (auto &client : connected_clients) {
    websocketpp::lib::error_code ec;
    ws_server.close(client, websocketpp::close::status::going_away, "Server shutting down", ec);
    if (ec) {
      std::cerr << "Failed to close Web Socket Server: " << ec.message() << std::endl;
    }
  }

  ws_server.stop_listening();
  ws_server.stop();
  std::cout << "WebSocket Server stopped." << std::endl;
}


void WebSocketServer::sendMessage(const std::string &symbol, const std::string &message) {
  std::lock_guard<std::mutex> lock(subscribers_mutex);
  
  auto it = symbol_subscribers.find(symbol);
  if (it != symbol_subscribers.end()) {
    for (const auto& client : it->second) {
      websocketpp::lib::error_code ec;
      ws_server.send(client, message, websocketpp::frame::opcode::text, ec);
      if (ec) {
        std::cerr << "Failed to send message to subscriber: " << ec.message() << std::endl;
      }
    }
  }
}


void WebSocketServer::registerValidSymbol(const std::string& symbol) {
  std::lock_guard<std::mutex> lock(subscribers_mutex);
  valid_symbols.insert(symbol);
}


void WebSocketServer::onOpen(connection_hdl hdl) {
  std::lock_guard<std::mutex> lock(subscribers_mutex);
  connected_clients.push_back(hdl);
  std::cout << "New client connected to your server!" << std::endl;
}

void WebSocketServer::onMessage(connection_hdl hdl, server::message_ptr msg) {
  std::string payload = msg->get_payload();
  
  rapidjson::Document doc;
  doc.Parse(payload.c_str());

  // Validate JSON and check for subscription request
  if (doc.IsObject() && doc.HasMember("type") && doc["type"].IsString()) {
    std::string type = doc["type"].GetString();
    if(doc.HasMember("symbol") && doc["symbol"].IsString()) {
      std::string symbol = doc["symbol"].GetString();
      if (type == "subscribe") {
        processSubscriptionRequest(hdl, symbol);
      } else if (type == "unsubscribe") {
        processUnsubscribeRequest(hdl, symbol);
      }
    }
  }

  // Handle invalid request
  sendInvalidJsonResponse(hdl);
}

void WebSocketServer::onClose(connection_hdl hdl) {
  std::lock_guard<std::mutex> lock(subscribers_mutex);
  
  // Remove client from all symbol subscriptions
  // for (auto& symbol_sub : symbol_subscribers) {
    // symbol_sub.second.erase(hdl);
  // }

  // Remove from connected clients
  // auto it = std::find(connected_clients.begin(), connected_clients.end(), hdl);
  // if (it != connected_clients.end()) {
  //   connected_clients.erase(it);
  // }

  std::cout << "A client disconnected from your server!" << std::endl;
}


void WebSocketServer::processSubscriptionRequest(connection_hdl hdl, const std::string& symbol) {
  std::lock_guard<std::mutex> lock(subscribers_mutex);

  if(valid_symbols.count(symbol) > 0){
    symbol_subscribers[symbol].push_back(hdl);

    // Send success response
    std::string json = "{\"type\":\"subscription_response\",\"subscription_successful_to\":\"" + symbol + "\"}";
    ws_server.send(hdl, json, websocketpp::frame::opcode::text);
  } else {
    // Send error response
    std::string json = "{\"type\":\"subscription_response\",\"invalid_symbol\":\"" + symbol + "\"}";
    ws_server.send(hdl, json, websocketpp::frame::opcode::text);
  }
}

void WebSocketServer::processUnsubscribeRequest(connection_hdl hdl, const std::string& symbol) {
  std::lock_guard<std::mutex> lock(subscribers_mutex);

  if (symbol_subscribers.count(symbol) > 0) {
    // symbol_subscribers[symbol].erase(hdl);

    // Send success response
    std::string json = "{\"type\":\"unsubscription_response\",\"unsubscription_successful_from\":\"" + symbol + "\"}";
    ws_server.send(hdl, json, websocketpp::frame::opcode::text);
  } else {
    // Send error response
    std::string json = "{\"type\":\"unsubscription_response\",\"invalid_symbol\":\"" + symbol + "\"}";
    ws_server.send(hdl, json, websocketpp::frame::opcode::text);
  }
}


void WebSocketServer::sendInvalidJsonResponse(connection_hdl hdl) {
  std::string error_response = "{\"error\":\"Invalid JSON\"}";
  ws_server.send(hdl, error_response, websocketpp::frame::opcode::text);
}