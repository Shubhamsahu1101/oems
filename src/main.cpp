#include <iostream>
#include <thread>

#include "WebSocketClient.h"
#include "WebSocketServer.h"
#include "OrderManager.h"
#include "../latency/Timing.h"

void show_menu() {
  std::cout << "\nSelect an action:\n"
            << "1. Place Buy Order\n"
            << "2. Place Sell Order\n"
            << "3. Cancel Order\n"
            << "4. Modify Order\n"
            << "5. Get Orderbook\n"
            << "6. Get Position\n"
            << "7. Subscribe to Symbols\n"
            << "8. Unsubscribe from Symbols\n"
            << "9. Exit\n"
            << "Enter choice\n\n";
}

void startTrade(OrderManager &orderManager, WebSocketClient &webSocketClient, WebSocketServer &webSocketServer) {
  std::cout << std::endl << "Trade thread started" << std::endl;
  orderManager.authenticate();

  while (true) {
    show_menu();
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    switch (choice)
    {
    case 1:
    {
      orderManager.placeBuyOrder();
      break;
    }
    case 2:
    {
      orderManager.placeSellOrder();
      break;
    }
    case 3:
    {
      orderManager.cancelOrder();
      break;
    }
    case 4:
    {
      orderManager.modifyOrder();
      break;
    }
    case 5:
    {
      orderManager.getOrderbook();
      break;
    }
    case 6:
    {
      orderManager.viewCurrentPositions();
      break;
    }
    case 7:
    {
      orderManager.subscribeToSymbols();
      break;
    }
    case 8:
    {
      orderManager.unsubscribeFromSymbols();
      break;
    }
    case 9:
    {
      webSocketClient.close();
      webSocketServer.stop();
      return;
    }
    default:
    {
      std::cout << "Invalid choice. Please try again." << std::endl;
      break;
    }
    }
  }
}

int main() {
  // Initialize the WebSocket server
  uint16_t PORT = 3001;
  WebSocketServer webSocketServer(PORT);

  // Initialize the WebSocket client to connect to the server
  std::string uri = "wss://test.deribit.com/ws/api/v2";
  WebSocketClient webSocketClient(uri, webSocketServer);
  
  // Initialize the Order Management system
  OrderManager orderManager(webSocketClient);

  // create a vector to hold the threads
  std::vector<std::thread> threads;
  // thread 1 -> websocket client to deribit api
  threads.emplace_back(&WebSocketClient::connect, &webSocketClient);
  // thread 2 -> websocket server
  threads.emplace_back(&WebSocketServer::run, &webSocketServer);
  // thread 3 -> trading menu
  threads.emplace_back(startTrade, std::ref(orderManager), std::ref(webSocketClient), std::ref(webSocketServer));

  // all threads running

  // wait for the threads to finish
  for (auto &thread : threads) {
    thread.join();
  }

  // calculateLatency();

  return 0;
}