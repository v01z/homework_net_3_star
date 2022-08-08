/* GNU General Public License v3.0 */
/******************************************************************************
*
* Name: TCP_Server.cpp
* TCP_Server class source code for homework_net_3_star project
*
* Copyright (C) 2022, binary_cat, v01z@mail.ru
*
*****************************************************************************/

#include "TCP_Server.hpp"
#include <sys/stat.h> //stat()

//-----------------------------------------------------------------------------

TCP_Server::TCP_Server(const char* port)
{
    struct addrinfo hints, *bindAddress;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // we can change this to AF_INET6 in case of using IPV6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int gaiRetValCode = getaddrinfo(0, port, &hints, &bindAddress);
    if( gaiRetValCode != 0 )
    {
        throw std::runtime_error("getaddrinfo() failed: " +
                                         std::string(gai_strerror(gaiRetValCode)));
    }

    _socket = socket(bindAddress->ai_family,bindAddress->ai_socktype,
                     bindAddress->ai_protocol);
    if (!ISVALIDSOCKET(_socket))
    {
        freeaddrinfo(bindAddress);

        throw std::runtime_error("Socket creation error.\n" +
                                 std::string(get_error_text()));
    }

#if defined _WIN32
    char optval;
#else //_unix_
    int optval;
#endif

    if(setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &optval,
    sizeof(optval)) == -1)
    {
        freeaddrinfo(bindAddress);
        CLOSESOCKET(_socket);

        throw std::runtime_error ("setsockopt() failed.\n" +
                                  std::string(get_error_text()));
    }

    if (bind(_socket, bindAddress->ai_addr, bindAddress->ai_addrlen))
    {
        freeaddrinfo(bindAddress);
        CLOSESOCKET(_socket);

        throw std::runtime_error ("Socket binding error.\n" +
                                  std::string(get_error_text()));
    }
    freeaddrinfo(bindAddress);

    if(listen(_socket, 10) < 0)
    {
        CLOSESOCKET(_socket);

        throw std::runtime_error("Socket listening error.\n" +
                                 std::string(get_error_text()));
    }
}

//-----------------------------------------------------------------------------

TCP_Server::~TCP_Server() {
    CLOSESOCKET(_socket); //If we have had an instance then _socket must be valid.
}

//-----------------------------------------------------------------------------

TCP_Server* TCP_Server::getInstance(const char* port) {

    static TCP_Server instance { port };

    return &instance;
}

//-----------------------------------------------------------------------------

void TCP_Server::loop() {

    while (true){

        struct sockaddr_storage clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        memset(&clientAddr, 0, sizeof(sockaddr_in));

        std::cout << std::endl << "Waiting for connection..." << std::endl;

        SOCKET clientSockD = accept(_socket, reinterpret_cast<struct sockaddr*>(&clientAddr),
                 &clientAddrLen);
        if (!ISVALIDSOCKET(clientSockD))
        {
            std::cerr << "accept() failed.\n" << get_error_text << std::endl;
            continue; //waiting for other client
        }

        std::cout << "Client connected from:" << std::endl;
        char clientHostName[NI_MAXHOST];
        char clientServiceName[NI_MAXSERV];

        int gniRetVal = getnameinfo(reinterpret_cast<sockaddr *>(&clientAddr),
     sizeof(clientAddr), clientHostName, sizeof(clientHostName),
                               clientServiceName, sizeof(clientServiceName),
                               NI_NUMERICHOST);
        if(!gniRetVal)
            std::cout << YELLOW_COLOR << clientHostName << " : "
                << clientServiceName << NORMAL_COLOR << std::endl;
        else
            std::cerr << "Couldn't get client's address: " << gai_strerror(gniRetVal) << std::endl;

        char receivedBuff [BUFF_SIZE];

        std::cout << "Receiving data from client.." << std::endl;

        int bytesReceived = recv(clientSockD, receivedBuff, BUFF_SIZE, 0);

        if ((bytesReceived == -1) || (bytesReceived == 0)){
            std::cerr << "recv()\n" << get_error_text << std::endl;

            CLOSESOCKET(clientSockD);

            continue; //connection is broken, let's wait for other client
        }
            receivedBuff[bytesReceived] = '\0';

            std::cout << "Received " << bytesReceived << " bytes from client:" << std::endl;
            std::cout << YELLOW_COLOR << receivedBuff << NORMAL_COLOR << std::endl;

            if (!strcmp(EXIT_COMMAND, receivedBuff))
            {
                std::cout << "Received " << YELLOW_COLOR << "exit" << NORMAL_COLOR
                    << " command, shutting down.." << std::endl;
                CLOSESOCKET(clientSockD);
                CLOSESOCKET(_socket);
                break;
            }

        std::cout << "Sending response to to client.." << std::endl;

        sendAnswerToClient(&clientSockD, receivedBuff);

        CLOSESOCKET(clientSockD);
    } //while
}

//-----------------------------------------------------------------------------

const std::pair<bool, std::string> TCP_Server::isValidFile(std::string &filename) const {

    std::pair<bool, std::string> retVal{ true, filename };

    int result{};

#if defined (_WIN32)
    struct _stat buffer;
    result = _stat( filename.c_str(), &buffer );
#else
    struct stat buffer;
    result = stat(filename.c_str(), &buffer);
#endif

    //An error detected
    if(result) {
        std::cerr << "Problem getting information.\n";
        retVal.first = false;

        retVal.second = "Error for " + filename + ": ";

        switch (errno) {
            case ENOENT:
                retVal.second.append("File not found.");
                break;
            case EINVAL:
                retVal.second.append("Invalid parameter to stat()");
                break;
            default:
                /* Should never be reached. */
                retVal.second.append("Unexpected error in stat()");
        }
    }

    //Directory doesnt fit for our purpose too
    if (buffer.st_mode & S_IFDIR)
    {
        retVal.first = false;
        retVal.second = "Error for " + filename + ": Is a directory.";
    }

    //Accessing error
    if (access(filename.c_str(), R_OK)) {
        retVal.first = false;
        retVal.second = "Error for " + filename + ": Permission denied.";
    }

    return retVal;
}

//-----------------------------------------------------------------------------

void TCP_Server::sendAnswerToClient(const SOCKET *clientSock, const char *receivedBuff) {

    const std::string receivedStr { receivedBuff };

    //First case.
    //Let's play like ordinary echo-server in this case and send back received message.

    if(std::string(receivedStr, 0, GET_FILE_COMMAND.length()).compare(GET_FILE_COMMAND))
    {
        size_t sentBytes = send(*clientSock, receivedStr.c_str(), receivedStr.length(), 0);
        if ((sentBytes) == -1 || (sentBytes != receivedStr.length()))
        {
            std::cerr << "Sending response to client failed.\n";
        }

        std::cout << "Sent " << sentBytes << " bytes: " << YELLOW_COLOR << receivedStr
            << NORMAL_COLOR << std::endl;
        return;
    }

    //Second case.
    //Client asked for a file and we have to give it to him if nothing disturbs us.

    std::string fileName { receivedStr.c_str() + GET_FILE_COMMAND.length() };

#if defined (_WIN32)
    for (unsigned i{}; i < fileName.length(); i++)
    {
       if (fileName.at(i) == '/')
           fileName[i] = '\\';
    }
#endif // _WIN32

    std::pair<bool, std::string> whetherFileIsValid =
            isValidFile(fileName);

    if (whetherFileIsValid.first)
    {
        std::cout << "File is " << YELLOW_COLOR << "valid\n" << NORMAL_COLOR;
        std::cout << "Sending file " << fileName << " to client:\n";

        if (FILE *fp = fopen(fileName.c_str(), "rb"))
        {
            size_t readBytes;
            char buffer[BUFF_SIZE];

            while ((readBytes = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0)
            {
                size_t bytesSent = send(*clientSock, buffer, readBytes, 0);
                if(bytesSent != readBytes)

                {

                    std::cerr << "Sending file to client failed. \n"
                              << get_error_text << std::endl;
                    break;
                }
                std::cout << "\nBytes sent: " << bytesSent << std::endl; //debug
                memset(buffer, 0, sizeof(buffer)); //clear to avoid tail of trash
            }
            fclose(fp);
        }
    }
    else {
        std::cout << "file " << fileName <<" is " << YELLOW_COLOR <<
            "not valid\n" << NORMAL_COLOR;
        size_t sentBytes = send(*clientSock, whetherFileIsValid.second.c_str(),
            whetherFileIsValid.second.length(), 0);
        if ((sentBytes) == -1 || (sentBytes != whetherFileIsValid.second.length()))
        {
            std::cerr << "Sending response to client failed.\n";
            return;
        }
    }
}

//-----------------------------------------------------------------------------

