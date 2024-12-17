#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>

class WebSocketServer {
public:
  // Define server type
  using server = websocketpp::server<websocketpp::config::asio>;
  using connection_hdl = websocketpp::connection_hdl;

  // Constructor
  explicit WebSocketServer(uint16_t PORT);

  // Server lifecycle methods
  void run();
  void stop();

  // Message sending with symbol-specific routing
  void sendMessage(const std::string &symbol, const std::string &message);

  // Predefined valid symbols
  void registerValidSymbol(const std::string& symbol);

private:
  // WebSocket server setup
  server ws_server;
  const uint16_t PORT;
  std::atomic<bool> isListening;

  std::unordered_map<std::string, std::vector<connection_hdl>> symbol_subscribers;
  std::unordered_set<std::string> valid_symbols;
  std::mutex subscribers_mutex;

  // Connection tracking
  std::vector<connection_hdl> connected_clients;

  // Event handlers
  void onOpen(connection_hdl hdl);
  void onMessage(connection_hdl hdl, server::message_ptr msg);
  void onClose(connection_hdl hdl);

  // Subscription management methods
  void processSubscriptionRequest(connection_hdl hdl, const std::string& payload);
  void processUnsubscribeRequest(connection_hdl hdl, const std::string& payload);

  // Error handling
  void sendInvalidJsonResponse(connection_hdl hdl);
};

#endif // WEBSOCKET_SERVER_H