/* GNU General Public License v3.0 */
/******************************************************************************
*
* Name: main_client.cpp
* Client source code for homework_net_3_star project
*
* Copyright (C) 2022, binary_cat, v01z@mail.ru
*
*****************************************************************************/

#include "main.hpp"

//-----------------------------------------------------------------------------

void tcpClient(int argc, char *argv[]) {
    if (argc < 3 || strcmp(argv[1], "--help") == 0) {
        std::cout << "Usage: " << argv[0] <<
                  " host-address message\n";
        std::cout << "Or, if u want to get a file, choose the following syntax:\n";
        std::cout << argv[0] << " host-address get_file:<FULL PATH OF FILENAME>\n";
        std::cout << "Do not forget to use two backward backslashes in case of Windoze-like pathes.\n";

        exit(EXIT_FAILURE);
    }

    std::cout << "Configuring remote address.." << std::endl;

    struct addrinfo hints, *serverAddr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    int gaiRetValCode{ getaddrinfo(argv[1],
                                  PORT_NUM, &hints, &serverAddr) };
    if (gaiRetValCode) {
        throw std::runtime_error("getaddrinfo() failed: " +
                                 std::string(gai_strerror(gaiRetValCode)));
    }

    char serverIP[NI_MAXHOST];

    int gniRetVal = getnameinfo(serverAddr->ai_addr,
                                serverAddr->ai_addrlen,
                                serverIP, NI_MAXHOST,
                                0, 0, NI_NUMERICHOST);
    if (!gniRetVal)
        std::cout << "Connecting to " << YELLOW_COLOR << serverIP <<
                  NORMAL_COLOR << " ... " << std::endl;
    else {
        freeaddrinfo(serverAddr);

        throw std::runtime_error("Couldn't resolve server address: " +
                                 std::string(gai_strerror(gniRetVal)));
    }

    SOCKET sockd = socket(serverAddr->ai_family,
                          serverAddr->ai_socktype,
                          serverAddr->ai_protocol);
    if (!ISVALIDSOCKET(sockd)) {
        freeaddrinfo(serverAddr);

            throw std::runtime_error("socket() failed.\n" +
                                std::string(get_error_text()));


    }

    if (connect(sockd, serverAddr->ai_addr, serverAddr->ai_addrlen)) {
        freeaddrinfo(serverAddr);
        CLOSESOCKET(sockd);

        throw std::runtime_error("Can't connect to a server \'" + std::string(argv[1]) +
                            "\'.\n" + std::string(get_error_text()));
    }

    freeaddrinfo(serverAddr);

    std::string msgStr{};

    for (int i{2}; i < argc; i++) {
        msgStr.append(argv[i]);
        if (!(i == argc - 1))
            msgStr.append(" ");
    }

    size_t msgLen{strlen(msgStr.c_str())};

    if (msgLen < BUFF_SIZE - 1) {
        int bytesSent = send(sockd, msgStr.c_str(), msgLen, 0);
        if (bytesSent != msgLen) {
            CLOSESOCKET(sockd);

            throw std::runtime_error("Can't send message \'" + msgStr +
                    "\'" + " to a server \'" + std::string(argv[1]) + "\'.\n"
                                    + std::string(get_error_text()));

        } else
            std::cout << "Message \'" << YELLOW_COLOR << msgStr << NORMAL_COLOR <<
                      "\' (" << bytesSent << " bytes) has been sent to a server " << YELLOW_COLOR
                      << argv[1] << NORMAL_COLOR << std::endl;
    } else {
        CLOSESOCKET(sockd);
        throw std::runtime_error("U've entered too long message ( >= " +
            std::to_string(BUFF_SIZE) + " ), sorry.\n");
    }

    char buff[BUFF_SIZE]{};

    std::cout << "Receiving data from server.." << std::endl;

    //message only
    if (std::string(msgStr, 0, GET_FILE_COMMAND.length()).compare(GET_FILE_COMMAND)) {
        int bytesReceived = recv(sockd, buff, BUFF_SIZE, 0);
        if (bytesReceived == -1) {
            CLOSESOCKET(sockd);
            throw std::runtime_error("recv() failed with error: " +
            //                         std::to_string(GETLASTERROR()));
                                    std::string(get_error_text()));
        }

        std::cout << "Received " << bytesReceived << " bytes from server:" << std::endl;
        std::cout << YELLOW_COLOR << buff << NORMAL_COLOR << std::endl;
    }
    else //File has been queried
    {
        char buffer[BUFF_SIZE]{};
        std::string fileName{ msgStr.substr(msgStr.find_last_of("/\\") + 1) };

        if (FILE *fp = fopen(fileName.c_str(), "wb"))
        {
            size_t readBytes;
            bool needCheckForError = true;
            while ((readBytes = recv(sockd, buffer, sizeof(buffer), 0)) > 0)
            {
                if(needCheckForError) {
                    std::string fullFileNameStr = msgStr.substr(GET_FILE_COMMAND.length(), msgStr.length());
                    std::cout << "\n" << fullFileNameStr << std::endl;
                    std::string errorStr = "Error for " + fullFileNameStr;
                    if (!errorStr.compare(std::string(buffer, errorStr.length())))
                            break;
                }
                needCheckForError = false;

                if (fwrite(buffer, sizeof(char), readBytes, fp) != readBytes)
                {

                    std::cerr << "Writing file " << fileName << " failed.\n" <<
                              get_error_text << std::endl;
                    break;
                }
                memset(buffer, 0, sizeof(buffer)); //clear to avoid tail of trash
            } //while

            fclose(fp);
            if(!needCheckForError)
            {
                std::cout << "File " << YELLOW_COLOR << fileName << NORMAL_COLOR <<
                          " has been downloaded.\n";
            }
            else
            {
                std::remove(fileName.c_str());
                std::cout << YELLOW_COLOR << buffer << NORMAL_COLOR << std::endl;
            }
        }

    }

    CLOSESOCKET(sockd);
}

//-----------------------------------------------------------------------------

int main(int argc, char *argv[]){
#ifdef _WIN32
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        std::cerr << "Failed to initialize WinSock. Error Code: "
            << WSAGetLastError() << std::endl;
        return 1;
    }
#endif //_WIN32

    try
    {
        tcpClient(argc, argv);
    }
    catch(std::runtime_error& e)
    {
        std::cout << e.what() << "\n";
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
