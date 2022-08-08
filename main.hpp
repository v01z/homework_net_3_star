/* GNU General Public License v3.0 */
/******************************************************************************
*
* Name: main.hpp
* Main header file for homework_net_3_star project
*
* Copyright (C) 2022, binary_cat, v01z@mail.ru
*
*****************************************************************************/

#ifndef LESSON_3_MAIN_HPP
#define LESSON_3_MAIN_HPP

#include <iostream>

#if defined(_WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif //_WIN32_WINNT

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)


#else //if not _WIN32 but __unix__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)

#define SOCKET int

#endif //_WIN32

#include <cstring>

//-----------------------------------------------------------------------------

#define BUFF_SIZE 1024
#define PORT_NUM "51511"

#define EXIT_COMMAND "exit"

//Two types of colors should be enough for our purpose
#define YELLOW_COLOR "\x1B[33m"
#define NORMAL_COLOR "\x1B[0m"

const std::string GET_FILE_COMMAND { "get_file:" };

const char *get_error_text();

//-----------------------------------------------------------------------------



#endif //LESSON_3_MAIN_HPP
