#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <iostream>
#include <string>

#include "OrderManager.h"
#include "WebSocketServer.h"

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::connection_hdl connection_hdl;

class WebSocketClient {
public:
  // Constructor
  explicit WebSocketClient(const std::string &uri, WebSocketServer& webSocketServer);
  
  // Connection lifecycle methods
  void connect();
  void reconnect();
  void close();

  // Sending methods
  void sendJson(const std::string &json_data);

private:
  // WebSocket client setup
  client ws_client;
  connection_hdl ws_hdl;
  std::string uri;
  WebSocketServer& webSocketServer;

  // Connection state management
  std::atomic<bool> isConnected;
  std::atomic<bool> shouldReconnect;
  std::mutex connectionMutex;

  // Logging and error handling
  enum class LogLevel {
    INFO,
    WARNING,
    ERROR
  };
  
  // Logging method
  void log(LogLevel level, const std::string& message);

  // Event handlers
  void onOpen(connection_hdl hdl);
  void onClose(connection_hdl hdl);
  void onFail(connection_hdl hdl);
  void onMessage(connection_hdl hdl, client::message_ptr msg);

  // Message processing
  void processSubscriptionMessage(const rapidjson::Document& response);
  void processOrderMessage(const rapidjson::Document& response);
  void writeMessageToLog(const rapidjson::Document& response);

};

#endif // WEBSOCKET_CLIENT_H
