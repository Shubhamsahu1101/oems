## High Performance Order Execution and Management System

Video demonstration: [OEMS execution video](https://drive.google.com/file/d/1c317htZptTptEp-2uQmtxB-skIBjTNhc/view?usp=sharing)

## Features
- WebSocket based trading client built completely in C++.
- Multithreaded for high performance.
- Communicates with API using a persistent WebSocket connection.
- Functionality for a WebSocket server with subscribe/unsubscribe functionality for clients.
- Robust Error Handling with 3 levels of logging messages (INFO, WARNING and ERROR)

## Dependencies

- Websocketpp (For WebSocket client and server functionality)
- Boost  (Required by Websocketpp for various utilities)
- OpenSSL (For secure connections using SSL)
- Rapidjson (For fast JSON parsing)
- Pthread (Multi-threading support in C++)
