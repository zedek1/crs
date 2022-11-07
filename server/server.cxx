#include "server.hxx"


// i am keeping server_free before this needs to happen before deconstruction
int Server::server_free()
{
    // wait for socket thread to stop before WSACleanup
    bRunning = false;
    WaitForSingleObject(thread1_handle, INFINITE);

    if (!connections.empty())
    {   // shutdown all clients
        for (auto conn : connections)
        {
            iResult = shutdown(conn, SD_SEND);
            if (iResult == SOCKET_ERROR) {
                std::wcerr << failed << L"shutdown failed with error: " << WSAGetLastError();
                closesocket(conn);
                WSACleanup();
                return -1;        
            }
            else { std::wcout << success << L"connections closed\n"; }
        }
    }
    std::wcout << warn << L"cleaning up...\n";
    WSACleanup();
    std::wcout << success << L"goodbye :)";
    return 0;
}


void Server::init()
{
    // initialize winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if ( iResult != 0 ) {
        std::wcerr << failed << L"WSAStartup failed with error: " << iResult;
    }

    //wipe the struct then set what we want
    SecureZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;
    
    // all this does is translate the hostname from unicode to an address
    if (GetAddrInfoW(L"127.0.0.1", lport, &hints, &addrInfo) != 0) {
        std::wcerr << failed << L"getaddrinfo failed with error: " << WSAGetLastError();
    }
    std::wcout << L"\nAddr Info Found | ";
}


void Server::create_socket()
{
    // create listener socket   
    // a socket needs three integers to be created - af, type, protocol
    // which was declared in hints in the function above but is now stored in addrInfo
    // im also using the flag WSA_FLAG_OVERLAPPED to allow overlapped i/o <- REMOVE IF BAD
    listenSocket = WSASocketW(addrInfo->ai_family,
                              addrInfo->ai_socktype,
                              addrInfo->ai_protocol,
                              0, NULL, WSA_FLAG_OVERLAPPED);

    if (listenSocket == INVALID_SOCKET) {
        std::wcerr << failed << L"socket creation failed with error: " << WSAGetLastError();
        FreeAddrInfoW(addrInfo); 
    }

    int error_code = 1; // modify socket settings for TCP
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&error_code, sizeof(error_code)) < 0) {
        std::wcerr << failed << L"Error setting TCP socket options! Error: " << WSAGetLastError();
        FreeAddrInfoW(addrInfo); 
    }
    std::wcout << L"Socket Created | ";
}


void Server::bind_socket()
{
    while (1) { // bind socket
        iResult = bind(listenSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);
        
        if (iResult == SOCKET_ERROR || iResult == INVALID_SOCKET) {
            std::wcerr << failed << L"socket binding failed with error: " << WSAGetLastError();
            std::this_thread::sleep_for(std::chrono::seconds(10)); // sleep 10 seconds
            continue;
        }
        else {
            //printf("socket bind successful\n");
            // start listening for connections here
            if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
                std::wcerr << failed << L"socket listen failed with error: " << WSAGetLastError();
                server_free();
            }
            else {
                // free memory
                FreeAddrInfoW(addrInfo);
                break;
            }
        }
    }
    std::wcout << L"Socket Bind | ";
}


void Server::accept_connections()
{
    // nice looking start display
    std::wcout << L"Starting Handler Thread\n";
    std::wcout << L"========================================================================\n" <<
                    "Starting Listener -> \x1B[38;5;111mLHOST: \x1B[38;5;221m127.0.0.1\033[0m | " <<
                    "\x1B[38;5;111mLPORT: \x1B[38;5;221m" << lport << "\033[0m\n\n";

    // basic socket info for WSAAccept
    SOCKET client_socket = INVALID_SOCKET; // switch to local variable
    struct sockaddr_in clientInfo = {0};
    int addrsize = sizeof(clientInfo);

    // select() vars
    timeval timeout = {4, 0};
    fd_set fds;

    while (bRunning) 
    {
        // using select to wait 4 seconds then run WSAAccept only if the socket is acceptable, 
        // thus stopping WSAAccept from blocking, and allowing the loop to reiterate to check bRunning
        FD_ZERO(&fds);
        FD_SET(listenSocket, &fds);
        select(listenSocket+1, &fds, NULL, NULL, &timeout);

        if (FD_ISSET(listenSocket, &fds)) // if socket is acceptable
        {
            client_socket = WSAAccept(listenSocket, (struct sockaddr*)&clientInfo, &addrsize, NULL, NULL);
            if (client_socket == INVALID_SOCKET) 
            {
                std::wcerr << failed << L"Error accepting connections: " << WSAGetLastError();
                continue;
            }
            else 
            {
                connections.push_back(client_socket);
                std::wcout << L"\n\n" << success << L"Connection has been established \n";
                // reprint handler line because input was interrupted
                std::wcout << L"\n\x1B[38;5;159mhandler\x1B[38;2;231;72;86m> \033[0m";
            }
        }
        else 
        {
            continue;
        }
    }
}