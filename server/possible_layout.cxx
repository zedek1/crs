int wmain() {
    Server s;
    s.run(1234);
    std::wcout << "successful back";
    return 0;
}

Server::run(int port) {
    // listen

    while (running) {
        
    }

    return 0;
}

class Server {
private:
    bool running = true;
}