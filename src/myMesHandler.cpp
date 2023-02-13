#include "myMesHandler.hpp"

using namespace std;

int msgHandler(std::string msg, WebsocketServer &server)
{
    clog << "msgHandler:" << msg << endl;
    return 0;
}