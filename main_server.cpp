/* GNU General Public License v3.0 */
/******************************************************************************
*
* Name: main_server.cpp
* Server source code for homework_net_3_star project
*
* Copyright (C) 2022, binary_cat, v01z@mail.ru
*
*****************************************************************************/

#include "main.hpp"
#include "TCP_Server.hpp"

//-----------------------------------------------------------------------------

int main(){

#ifdef _WIN32
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        std::cerr << "Failed to initialize WinSock. Error Code: "
            << WSAGetLastError() << std::endl;
        return 1;
    }
#endif //_WIN32

    TCP_Server *udpServer;
    try
    {
        udpServer = TCP_Server::getInstance(PORT_NUM);
        udpServer->loop();
    }
    catch(std::runtime_error& e)
    {
        std::cout << e.what() << "\n";
    }

    std::cout << "The server has been shutdown." << std::endl;

#ifdef _WIN32
    WSACleanup();
#endif

    return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
