#include <Windows.h>
#include "server.hxx"
#include <chrono>

DWORD WINAPI socket_thread1(LPVOID args)
{
    Server* s = (Server*)args;
    s->init();
    s->create_socket();
    s->bind_socket();
    s->accept_connections();
    return 0;
}

DWORD WINAPI handler_thread2(LPVOID args)
{
    Server* s = (Server*)args;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    s->start_handler(); // start the handler
    return 0;
}

int wmain()
{
    //i did it like this because it is so damn easy to read
    Server server;

    HANDLE array_of_thread_handles[2];

    server.thread1_handle = CreateThread(NULL, 0, socket_thread1, &server, 0, NULL);
    server.thread2_handle = CreateThread(NULL, 0, handler_thread2, &server, 0, NULL);

    array_of_thread_handles[0] = server.thread1_handle;
    array_of_thread_handles[1] = server.thread2_handle;

    // figure out how to close all threads without killing this program
    WaitForMultipleObjects(2, array_of_thread_handles, TRUE, INFINITE);

    CloseHandle(server.thread1_handle);
    CloseHandle(server.thread2_handle);
    
    std::wcout << "out";
    return 0;
}


// TODO: use original socket. dont make clientsocket = vector socket