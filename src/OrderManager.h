#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include <string>
#include "WebSocketClient.h"

class WebSocketClient;

class OrderManager {
public:
  // Constructor
  explicit OrderManager(WebSocketClient& webSocketClient);

  // Order management methods
  void authenticate();
  void placeBuyOrder();
  void placeSellOrder();
  void cancelOrder();
  void modifyOrder();
  void getOrderbook();
  void viewCurrentPositions();
  void subscribeToSymbols();
  void unsubscribeFromSymbols();

private:
  WebSocketClient& webSocketClient;

  // Helper methods
  void loadEnv(const std::string& path);
};


#endif // ORDER_MANAGER_H
