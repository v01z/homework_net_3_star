/* GNU General Public License v3.0 */
/******************************************************************************
*
* Name: TCP_Server.cpp
* Errors decoding function for homework_net_3_star project
*
* Copyright (C) 2022, binary_cat, v01z@mail.ru
*
*****************************************************************************/

#include "main.hpp"

//-----------------------------------------------------------------------------

const char *get_error_text()
{
#if defined(_WIN32)
    static char message[256] = {};

    FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                0, WSAGetLastError(), 0, message, 256, 0);

    char *new_line = strchr(message, '\n');

    if(new_line) *new_line = 0;

    return message;

#else
    return strerror(errno);
#endif
}

//-----------------------------------------------------------------------------
