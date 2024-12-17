#include <fstream>

#include "WebSocketClient.h"
#include "../latency/Timing.h"

WebSocketClient::WebSocketClient(const std::string &uri, WebSocketServer& webSocketServer) 
    : uri(uri), isConnected(false), shouldReconnect(true), webSocketServer(webSocketServer) {
  // Initialize ASIO client
  ws_client.init_asio();

  // Configure TLS
  ws_client.set_tls_init_handler([this](connection_hdl) {
    return std::make_shared<websocketpp::lib::asio::ssl::context>(
      websocketpp::lib::asio::ssl::context::tlsv12
    );
  });

  // Set event handlers
  ws_client.set_message_handler([this](websocketpp::connection_hdl hdl, client::message_ptr msg) { onMessage(hdl, msg); });

  ws_client.set_open_handler([this](websocketpp::connection_hdl hdl) { onOpen(hdl); });

  ws_client.set_fail_handler([this](websocketpp::connection_hdl hdl) { onFail(hdl); });

  ws_client.set_close_handler([this](websocketpp::connection_hdl hdl) { onClose(hdl); });

  // Mute all logs
  ws_client.clear_access_channels(websocketpp::log::alevel::all);
}


void WebSocketClient::connect() {
  log(LogLevel::INFO, "Attempting to connect to: " + uri);
  
  websocketpp::lib::error_code ec;
  client::connection_ptr con = ws_client.get_connection(uri, ec);
  
  if (ec) {
    log(LogLevel::ERROR, "Connection failed: " + ec.message());
    return;
  }

  ws_client.connect(con);
  log(LogLevel::INFO, "WebSocket client connected to API");
  
  // Run WebSocket client in a separate thread
  ws_client.run();
}

void WebSocketClient::reconnect() {
  log(LogLevel::WARNING, "Attempting to reconnect...");
  
  if (isConnected) {
    close();
  }
  
  std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait before reconnecting
  connect();
}

void WebSocketClient::close() {
  log(LogLevel::INFO, "Closing WebSocket connection");
  
  std::lock_guard<std::mutex> lock(connectionMutex);
  
  if (isConnected) {
    websocketpp::lib::error_code ec;
    ws_client.close(ws_hdl, websocketpp::close::status::normal, "Client closing", ec);
    
    if (ec) {
      log(LogLevel::ERROR, "Error closing connection: " + ec.message());
    }
    
    ws_client.stop();
    isConnected = false;
  }

  log(LogLevel::INFO, "WebSocket connection to API ended");
}


void WebSocketClient::sendJson(const std::string &json_data) {
  if (!isConnected) {
    log(LogLevel::ERROR, "Cannot send message, not connected.");

    while (!isConnected) {
      std::this_thread::yield(); // Yield thread while waiting for connection
    }
  }

  //  To use rapidjson for sending data. Adds extra latency!
  //  Convert RapidJSON Document to string
  //  rapidjson::StringBuffer buffer;
  //  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  //  json_data.Accept(writer);
  //  std::string msg = buffer.GetString();
  // ws_client.send(ws_hdl, msg, websocketpp::frame::opcode::text, ec);

  // startTimer();
  
  websocketpp::lib::error_code ec;
  ws_client.send(ws_hdl, json_data, websocketpp::frame::opcode::text, ec);

  if (ec) {
    log(LogLevel::ERROR, "Send failed: " + ec.message());
  } else {
    log(LogLevel::INFO, "Sent message: " + json_data);
  }
}


void WebSocketClient::log(LogLevel level, const std::string& message) {
  std::string prefix;
  switch (level) {
    case LogLevel::INFO:    prefix = "[INFO] "; break;
    case LogLevel::WARNING: prefix = "[WARNING] "; break;
    case LogLevel::ERROR:   prefix = "[ERROR] "; break;
  }
  
  std::cout << prefix << message << std::endl;
}


void WebSocketClient::onOpen(connection_hdl hdl) {
  std::lock_guard<std::mutex> lock(connectionMutex);
  
  ws_hdl = hdl;
  isConnected = true;
}

void WebSocketClient::onClose(connection_hdl) {
  std::lock_guard<std::mutex> lock(connectionMutex);
  
  isConnected = false;
  log(LogLevel::WARNING, "WebSocket connection closed");
  
  if (shouldReconnect) {
    reconnect();
  }
}

void WebSocketClient::onFail(connection_hdl) {
  log(LogLevel::ERROR, "WebSocket connection failed");
  
  if (shouldReconnect) {
    reconnect();
  }
}

void WebSocketClient::onMessage(connection_hdl, client::message_ptr msg) {
  // stopTimer();

  // Parse the message into a JSON document
  rapidjson::Document response;
  response.Parse(msg->get_payload().c_str());

  if (response.HasParseError()) {
    log(LogLevel::ERROR, "JSON parse error");
  }

  if (response.HasMember("error")) {
    log(LogLevel::ERROR, "API Error: " + std::string(response["error"]["message"].GetString()));
  }

  // Process different types of messages
  if (response.HasMember("method") && response["method"].GetString() == std::string("subscription")) {
    processSubscriptionMessage(response);
  } else if (response.HasMember("result") && !response["result"].IsArray() && response["result"].HasMember("order")) {
    processOrderMessage(response);
  } else if (response.HasMember("result") && !response["result"].IsArray() && response["result"].HasMember("access_token")) {
    log(LogLevel::INFO, "Authentication successful");
  }

  // Log the message
  writeMessageToLog(response);
}


void WebSocketClient::processSubscriptionMessage(const rapidjson::Document& response) {
  std::string channel = response["params"]["channel"].GetString();
  
  // Send the message to the WebSocket server
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  response.Accept(writer);
  
  webSocketServer.sendMessage(channel, buffer.GetString());
  
  log(LogLevel::INFO, "Received message from channel: " + channel);
}

void WebSocketClient::processOrderMessage(const rapidjson::Document& response) {
  std::string order_id = response["result"]["order"]["order_id"].GetString();
  log(LogLevel::INFO, "Order Successful. Order ID: " + order_id);
}

void WebSocketClient::writeMessageToLog(const rapidjson::Document& response) {
  log(LogLevel::INFO, "Received a message from API");
  std::ofstream log_file("../OutputLog.txt", std::ios_base::app);
  
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  response.Accept(writer);

  log_file << buffer.GetString() << std::endl;
  log_file.flush();
}
