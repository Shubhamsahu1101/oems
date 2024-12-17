#include <iostream>
#include <fstream>

#include "OrderManager.h"
#include "../latency/Timing.h"

OrderManager::OrderManager(WebSocketClient &webSocketClient) : webSocketClient(webSocketClient) {}


void OrderManager::authenticate() {
  // To use json for sending data. Adds extra latency!
  // rapidjson::Document json_data;
  // json_data.SetObject();
  // rapidjson::Document::AllocatorType& allocator = json_data.GetAllocator();

  // json_data.AddMember("jsonrpc", "2.0", allocator);
  // json_data.AddMember("id", 9929, allocator);
  // json_data.AddMember("method", "public/auth", allocator);
  // Create and populate the "params" object
  // rapidjson::Value params(rapidjson::kObjectType);
  // params.AddMember("grant_type", "client_credentials", allocator);
  // params.AddMember("client_id", client_id, allocator);
  // params.AddMember("client_secret", client_secret, allocator);

  // Add the "params" object to the main JSON object
  // json_data.AddMember("params", params, allocator);

  loadEnv("../.env");
  std::string client_id = std::getenv("CLIENT_ID");
  std::string client_secret = std::getenv("CLIENT_SECRET");

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9929,\"method\":\"public/auth\",\"params\":{\"grant_type\":\"client_credentials\",\"client_id\":\"" + client_id + "\",\"client_secret\":\"" + client_secret + "\"}}";
  webSocketClient.sendJson(json);
}

void OrderManager::placeBuyOrder() {

  std::string instrument_name, type, amount, price="0", market_label = "market01";
  std::cout << "Enter instrument name (e.g., ETH-PERPETUAL): ";
  std::getline(std::cin, instrument_name);
  std::cout << "Enter amount (quantity): ";
  std::getline(std::cin, amount);
  std::cout << "Enter order type (e.g., market, limit): ";
  std::getline(std::cin, type);

  if (type == "limit") {
    std::cout << "Enter price: ";
    std::getline(std::cin, price);
  }

  // Timer start for order placement latency   and   end to end latency
  // startTimer(); 
  
  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9930,\"method\":\"private/buy\",\"params\":{\"instrument_name\":\"" + instrument_name + "\",\"amount\":" + amount + ",\"type\":\"" + type + "\",\"price\":" + price + ",\"label\":\"" + market_label + "\"}}";
  webSocketClient.sendJson(json);
}

void OrderManager::placeSellOrder() {
  
  std::string instrument_name, type, amount, price="0", market_label = "market01";
  std::cout << "Enter instrument name (e.g., ETH-PERPETUAL): ";
  std::getline(std::cin, instrument_name);
  std::cout << "Enter amount (quantity): ";
  std::getline(std::cin, amount);
  std::cout << "Enter order type (e.g., market, limit): ";
  std::getline(std::cin, type);

  if (type == "limit") {
    std::cout << "Enter price: ";
    std::getline(std::cin, price);
  }

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9930,\"method\":\"private/sell\",\"params\":{\"instrument_name\":\"" + instrument_name + "\",\"amount\":" + amount + ",\"type\":\"" + type + "\",\"price\":" + price + ",\"label\":\"" + market_label + "\"}}";
  webSocketClient.sendJson(json);
}

void OrderManager::cancelOrder() {
  
  std::string order_id;
  std::cout << "Enter order ID: ";
  std::cin >> order_id;

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9932,\"method\":\"private/cancel\",\"params\":{\"order_id\":\"" + order_id + "\"}}";
  webSocketClient.sendJson(json);
}

void OrderManager::modifyOrder() {
  
  std::string order_id, amount, price;
  std::cout << "Enter order ID: ";
  std::getline(std::cin, order_id);
  std::cout << "Enter new amount (quantity): ";
  std::getline(std::cin, amount);
  std::cout << "Enter new price: ";
  std::getline(std::cin, price);

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9933,\"method\":\"private/edit\",\"params\":{\"order_id\":\"" + order_id + "\",\"amount\":" + amount + ",\"price\":" + price + "}}";
  webSocketClient.sendJson(json);
}

void OrderManager::getOrderbook() {
  
  std::string instrument_name, depth;
  std::cout << "Enter instrument name (e.g., ETH-PERPETUAL): ";
  std::getline(std::cin, instrument_name);
  std::cout << "Enter depth (1/5/10...): ";
  std::getline(std::cin, depth);

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9934,\"method\":\"public/get_order_book\",\"params\":{\"instrument_name\":\"" + instrument_name + "\",\"depth\":" + depth + "}}";
  webSocketClient.sendJson(json);
}

void OrderManager::viewCurrentPositions() {
  
  std::string inst_name;
  std::cout << "Enter instrument name (e.g., ETH-PERPETUAL): ";
  std::getline(std::cin, inst_name);

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9936,\"method\":\"private/get_position\",\"params\":{\"instrument_name\":\"" + inst_name + "\"}}";
  webSocketClient.sendJson(json);
}

void OrderManager::subscribeToSymbols() {
  
  std::string channel;
  std::cout << "Enter channel name (e.g. deribit_price_index.btc_usd): ";
  std::getline(std::cin, channel);

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9937,\"method\":\"private/subscribe\",\"params\":{\"channels\":[\"" + channel + "\"]}}";
  webSocketClient.sendJson(json);
}

void OrderManager::unsubscribeFromSymbols() {
  
  std::string channel;
  std::cout << "Enter channel name (e.g. deribit_price_index.btc_usd): ";
  std::getline(std::cin, channel);

  std::string json = "{\"jsonrpc\":\"2.0\",\"id\":9937,\"method\":\"private/unsubscribe\",\"params\":{\"channels\":[\"" + channel + "\"]}}";
  webSocketClient.sendJson(json);
}

void OrderManager::loadEnv(const std::string &filename) {
  
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open .env file.");
  }

  std::string line;
  while (std::getline(file, line)) {

    size_t pos = line.find('=');
    if (pos != std::string::npos) {

      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      setenv(key.c_str(), value.c_str(), 1);
    }
  }
}