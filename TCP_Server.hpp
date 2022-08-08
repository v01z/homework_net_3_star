/* GNU General Public License v3.0 */
/******************************************************************************
*
* Name: TCP_Server.hpp
* TCP_Server class header for homework_net_3_star project
*
* Copyright (C) 2022, binary_cat, v01z@mail.ru
*
*****************************************************************************/

#ifndef LESSON_3_TCP_SERVER_HPP
#define LESSON_3_TCP_SERVER_HPP

#include "main.hpp"

//-----------------------------------------------------------------------------

class TCP_Server {
private:
    SOCKET _socket;

    ~TCP_Server();
    const std::pair<bool, std::string> isValidFile(std::string&) const;
    void sendAnswerToClient(const SOCKET*, const char*);

protected:
    TCP_Server(const char*);

public:
    TCP_Server() = delete;
    TCP_Server(const TCP_Server&) = delete;
    void operator=(const TCP_Server&) = delete;
    static TCP_Server* getInstance(const char*);
    void loop();
};

//-----------------------------------------------------------------------------

#endif //LESSON_3_TCP_SERVER_HPP
