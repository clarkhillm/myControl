#include "WebsocketServer.h"
#include "myMesHandler.hpp"
#include <iostream>
#include <thread>
#include <asio/io_service.hpp>

int main(int argc, char* argv[])
{
    //Create the event loop for the main thread, and the WebSocket server
    asio::io_service mainEventLoop;
    WebsocketServer server;
    registerHandler(mainEventLoop,server);
    //Start the networking thread
    std::thread serverThread([&server]() {
        server.run(8080);
    });
    //Start the event loop for the main thread
    asio::io_service::work work(mainEventLoop);
    mainEventLoop.run();
    return 0;
}
