// CS260 Assignment 3
// Mason Kaschube
// HTTP Proxy Server

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>

void ConnectionLogic(SOCKET clientSock);

int main(int argc, char* argv[])
{
    int portNum = 0; 

    if (argv[1] != NULL)
    {
        int lengthCheck = 0;
        // make sure the user isn't trying to give us more data than we expect
        lengthCheck = (int)strlen(argv[1]);
        if (lengthCheck > 5)
        {
            std::cout << "The parameter you passed in is not a valid port. Try again" << std::endl;
            return 1;
        }

        // check for valid input
        for (int i = 0; i < lengthCheck; i++)
        {
            if (argv[1][i] > '9' || argv[1][i] < '0')
            {
                std::cout << "'" << argv[1] << "' is not a valid port number. Please try again" << std::endl;
                return 1;
            }
        }
    }
    else
    {
        // no port number parameter was passed in
        std::cout << "You must pass in a port number to run this application. Try again" << std::endl;
        return 1;
    }
    

    // parse the port number from the console
    if (argv[1] != NULL)
    {
        portNum = strtol(argv[1], NULL, 10);
    }
    


    // if the port is invalid, report that and quit out
    if (portNum < 0 || portNum > 65535)
    {
        std::cout << "Port binding error. Cannot connect to port '" << portNum << "'. Try a port in range [0, 65535]." << std::endl;
        return 1;
    }




    // Initialize WinSock
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
        std::cout << "Error in WSAStartup: " << WSAGetLastError() << std::endl;
        return 1;
    }



    // Create a listen socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cout << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }


    // make the socket non-blocking
    u_long iMode = 1;
    res = ioctlsocket(listenSocket, FIONBIO, &iMode);
    if (res != 0)
    {
        std::cout << "Error making socket non-blocking: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }


    // Create an Addressable Object
    sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    memset(&listenAddr.sin_zero, 0, 0);
    listenAddr.sin_port = htons(portNum);

    res = bind(listenSocket, (SOCKADDR*)&listenAddr, sizeof(listenAddr));
    if (res == SOCKET_ERROR)
    {
        std::cout << "Error in bind: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // establish a client socket variable we can use
    SOCKET clientSocket = INVALID_SOCKET;

    // tell the client what port we're listening on
    std::cout << "Listening on port: " << portNum << std::endl;

    res = listen(listenSocket, SOMAXCONN);
    if (res == SOCKET_ERROR)
    {
        std::cout << "Error on listen: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    while (1)
    {
        // listen infintely
        

        // try to accept a new connection
        clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET)
        {    
            // spawn a new thread to handle the client
            std::thread th1 = std::thread(ConnectionLogic, clientSocket);
            // the thread should resolve itself
            th1.detach();
        }

        // wait 0.1 seconds and continue
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // cleanup the entire process and return. This should theoretically never get called but just in case
    WSACleanup();
    return 0;
}

void ConnectionLogic(SOCKET clientSock)
{
    // create two buffers for reciving data from the client
    int bufLen = 1500;
    char* buf = new char[bufLen];
    char* tempBuf = new char[bufLen];
    int bufIndex = 0;
    int chunk = -1;
    int copyAmount = bufLen;
    memset(buf, 0, bufLen);
    memset(tempBuf, 0, bufLen);

    // listen for client's HTTP request
    while (chunk != 0)
    {
        chunk = recv(clientSock, tempBuf, copyAmount - 1, 0);

        if (chunk > 0)
        {
            memcpy(&buf[bufIndex], tempBuf, chunk);
            bufIndex += (chunk);
            if (bufIndex < bufLen)
            {
                buf[bufIndex] = '\0';
            }
            copyAmount -= chunk;
        }
    }

    // copy over to tempBuf so we can modify it without destroying the original message
    memcpy(tempBuf, buf, bufLen);

    // shutdown recieve
    int res = shutdown(clientSock, 0);
    if (res == SOCKET_ERROR)
    {
        std::cout << "Error shutting down recieving on the client socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // DEBUG: Print out the response we recieved
    //std::cout << "\nRecieved message: " << buf << std::endl;


    // set up variables to parse the HTTP request
    int process = 100;
    char* http = new char[process];
    char* host = new char[process];
    char* connection = new char[process];
    const char delim[4] = "\n\r";
    char* nextToken = NULL;

    // read in the first line (GET resource HTTP/1.1)
    http = strtok_s(tempBuf, delim, &nextToken);

    // read in the "Host: " line
    host = strtok_s(NULL, delim, &nextToken);
    host = host + 6;                            // 'Host: ' is 6 bytes so we can shift the pointer to only have the hostname

    // read in the "Connection: Close" line
    connection = strtok_s(NULL, delim, &nextToken);


    // create a new addressable object for the destination server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    memset(&serverAddr.sin_zero, 0, 0);
    serverAddr.sin_port = htons(80);    // always on 80 because HTTP
    

    res = inet_pton(AF_INET, host, &serverAddr.sin_addr);
    if (res != 1)
    {
        // inet_pton couldn't resolve the host name, so try getaddrinfo
        struct addrinfo hints;
        struct addrinfo* hostOut = new addrinfo();
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_CANONNAME;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // call getaddrinfo to translate an "http://..." address to an IP
        res = getaddrinfo(host, "80", &hints, &hostOut);

        // copy over the relevant data we need from hostOut to our serverAddr
        struct sockaddr_in temp = *(struct sockaddr_in*)hostOut->ai_addr;
        serverAddr.sin_addr = temp.sin_addr;

        if (res != 0)
        {
            // getaddrinfo and inet_pton failed
            std::cout << "Error parsing the host name: " << WSAGetLastError() << std::endl;
            freeaddrinfo(hostOut);
            WSACleanup();
        }
    }

    // create a new socket for the server
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cout << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // connect to the server
    res = connect(serverSocket, (struct sockaddr*)&serverAddr, (int)sizeof(serverAddr));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));        // connection not immediate, wait some time

    if (WSAGetLastError() != WSAEWOULDBLOCK && res != 0)
    {
        std::cout << "Error connecting to server: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // send to the server
    res = send(serverSocket, buf, bufLen - 1, 0);

    if (res == SOCKET_ERROR)
    {
        std::cout << "Error sending: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }


    // forward all information we get from the server directly to the client
    chunk = -1;
    while (chunk != 0)
    {
        chunk = recv(serverSocket, buf, bufLen, 0);
        if (chunk > 0)
        {
            // note: only send the amount of data we recieved, NOT all the data in the buffer
            send(clientSock, buf, chunk, 0);
        }
    }

    // shut down server socket for sending and recieving
    res = shutdown(serverSocket, 2);
    if (res == SOCKET_ERROR)
    {
        std::cout << "Error shutting down: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }


    // shutdown client socket for sending
    res = shutdown(clientSock, 2);
    if (res == SOCKET_ERROR)
    {
        std::cout << "Error shutting down: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }
}