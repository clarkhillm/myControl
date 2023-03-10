#include "WebsocketServer.h"
#include "myMesHandler.hpp"
#include <iostream>
#include <asio/io_service.hpp>
#include <string>

int registerHandler(asio::io_service& mainEventLoop,WebsocketServer& server)
{
    //Register our network callbacks, ensuring the logic is run on the main thread's event loop
    server.connect([&mainEventLoop, &server](ClientConnection conn)
    {
        mainEventLoop.post([conn, &server]()
        {
            std::clog << "Connection opened." << std::endl;
            std::clog << "There are now " << server.numConnections() << " open connections." << std::endl;
            
            //Send a hello message to the client
            server.sendMessage(conn, "hello", Json::Value());
        });
    });
    server.disconnect([&mainEventLoop, &server](ClientConnection conn)
    {
        mainEventLoop.post([conn, &server]()
        {
            std::clog << "Connection closed." << std::endl;
            std::clog << "There are now " << server.numConnections() << " open connections." << std::endl;
        });
    });
    server.message("message", [&mainEventLoop, &server](ClientConnection conn, const Json::Value& args)
    {
        mainEventLoop.post([conn, args, &server]()
        {
            std::clog << "message handler on the main thread" << std::endl;
            std::clog << "Message payload:" << std::endl;
            for (auto key : args.getMemberNames()) {
                std::clog << "\t" << key << ": " << args[key].asString() << std::endl;
            }
            //Echo the message pack to the client
            server.sendMessage(conn, "message", args);
        });
    });
    server.message("mouse", [&mainEventLoop, &server](ClientConnection conn, const Json::Value& args)
    {
        mainEventLoop.post([conn, args, &server]()
        {
            if(args.isMember("move")){
                mouseMoveHandler(args["move"].toStyledString(), server);
            }
            if(args.isMember("up")){
                mouseUpHandler(args["up"].toStyledString(), server);
            }
            if(args.isMember("down")){
                mouseDownHandler(args["down"].toStyledString(), server);
            }
            if(args.isMember("wheel")){
                mouseWheelHandler(args["wheel"].toStyledString(), server);
            }
        });
    });
    server.message("keyboard", [&mainEventLoop, &server](ClientConnection conn, const Json::Value& args)
    {
        mainEventLoop.post([conn, args, &server]()
        {
            if(args.isMember("down")){
                keyDownHandler(args["down"].toStyledString(), server);
            }
            if(args.isMember("up")){
                keyUpHandler(args["up"].toStyledString(), server);
            }
        });
    });

    return 0;
}

