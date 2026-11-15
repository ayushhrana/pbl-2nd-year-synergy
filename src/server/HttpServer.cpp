#include "HttpServer.hpp"

#include <csignal>
#include <ctime>
#include <vector>

#include "Errors.hpp"
#include "StringHelper.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SocketHandle;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int SocketHandle;
#endif

using namespace std;

namespace expense_tracker
{

struct Client
{
    SocketHandle socket;
    string received;
    time_t lastActive;
};

static bool isBadSocket(SocketHandle socketHandle)
{
#ifdef _WIN32
    return socketHandle == INVALID_SOCKET;
#else
    return socketHandle < 0;
#endif
}

static void closeSocket(SocketHandle socketHandle)
{
#ifdef _WIN32
    closesocket(socketHandle);
#else
    close(socketHandle);
#endif
}

static string statusText(int status)
{
    if (status == 200)
    {
        return "OK";
    }
    if (status == 400)
    {
        return "Bad Request";
    }
    if (status == 404)
    {
        return "Not Found";
    }
    if (status == 413)
    {
        return "Payload Too Large";
    }
    return "Internal Server Error";
}

// checks if the whole request (headers and body) has arrived
static bool isRequestComplete(const string& data, int& headerEnd, int& bodyLength)
{
    size_t position = data.find("\r\n\r\n");
    if (position == string::npos)
    {
        return false;
    }
    headerEnd = (int)position + 4;

    bodyLength = 0;
    string headers = toLowerCase(data.substr(0, position));
    size_t lengthPosition = headers.find("content-length:");
    if (lengthPosition != string::npos)
    {
        size_t start = lengthPosition + 15;
        string number = "";
        while (start < headers.size() && (headers[start] == ' ' || (headers[start] >= '0' && headers[start] <= '9')))
        {
            if (headers[start] != ' ')
            {
                number += headers[start];
            }
            start++;
        }
        if (!number.empty() && number.size() < 9)
        {
            bodyLength = stoi(number);
        }
    }
    return (int)data.size() >= headerEnd + bodyLength;
}

static HttpRequest parseRequest(const string& data, int headerEnd, int bodyLength)
{
    HttpRequest request;
    size_t lineEnd = data.find("\r\n");
    string firstLine = data.substr(0, lineEnd);

    size_t firstSpace = firstLine.find(' ');
    size_t secondSpace = firstLine.find(' ', firstSpace + 1);
    if (firstSpace == string::npos || secondSpace == string::npos)
    {
        throw ExpenseError("Bad request line");
    }

    request.method = firstLine.substr(0, firstSpace);
    string target = firstLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    size_t questionMark = target.find('?');
    if (questionMark != string::npos)
    {
        target = target.substr(0, questionMark);
    }
    request.path = target;
    request.body = data.substr(headerEnd, bodyLength);
    return request;
}

static void sendAll(SocketHandle socketHandle, const string& data)
{
    size_t sent = 0;
    while (sent < data.size())
    {
        int count = (int)send(socketHandle, data.c_str() + sent, (int)(data.size() - sent), 0);
        if (count <= 0)
        {
            return;
        }
        sent += count;
    }
}

static void sendResponse(SocketHandle socketHandle, const HttpResponse& response)
{
    string text = "HTTP/1.1 " + to_string(response.status) + " " + statusText(response.status) + "\r\n";
    text += "Content-Type: " + response.contentType + "\r\n";
    text += "Content-Length: " + to_string(response.body.size()) + "\r\n";
    text += "Cache-Control: no-store\r\n";
    text += "Connection: close\r\n\r\n";
    text += response.body;
    sendAll(socketHandle, text);
}

void HttpServer::run(int port)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    SocketHandle listener = socket(AF_INET, SOCK_STREAM, 0);
    if (isBadSocket(listener))
    {
        throw ExpenseError("Cannot create the server socket");
    }

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons((unsigned short)port);
    for (size_t i = 0; i < sizeof(address.sin_zero); i++)
    {
        address.sin_zero[i] = 0;
    }

    if (bind(listener, (sockaddr*)&address, sizeof(address)) != 0)
    {
        closeSocket(listener);
        throw ExpenseError("Cannot start the server on port " + to_string(port) + " (is it already in use?)");
    }
    if (listen(listener, 16) != 0)
    {
        closeSocket(listener);
        throw ExpenseError("Cannot listen on port " + to_string(port));
    }

    vector<Client> clients;

    while (true)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listener, &readSet);
        SocketHandle biggest = listener;
        for (int i = 0; i < (int)clients.size(); i++)
        {
            FD_SET(clients[i].socket, &readSet);
            if (clients[i].socket > biggest)
            {
                biggest = clients[i].socket;
            }
        }

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int ready = select((int)biggest + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready < 0)
        {
            continue;
        }

        if (FD_ISSET(listener, &readSet))
        {
            SocketHandle accepted = accept(listener, nullptr, nullptr);
            if (!isBadSocket(accepted))
            {
                Client client;
                client.socket = accepted;
                client.lastActive = time(nullptr);
                clients.push_back(client);
            }
        }

        for (int i = 0; i < (int)clients.size(); i++)
        {
            if (!FD_ISSET(clients[i].socket, &readSet))
            {
                continue;
            }

            char buffer[8192];
            int count = (int)recv(clients[i].socket, buffer, sizeof(buffer), 0);
            if (count <= 0)
            {
                closeSocket(clients[i].socket);
                clients.erase(clients.begin() + i);
                i--;
                continue;
            }

            clients[i].received.append(buffer, count);
            clients[i].lastActive = time(nullptr);

            if (clients[i].received.size() > 2000000)
            {
                HttpResponse tooBig;
                tooBig.status = 413;
                tooBig.body = "Request is too big";
                sendResponse(clients[i].socket, tooBig);
                closeSocket(clients[i].socket);
                clients.erase(clients.begin() + i);
                i--;
                continue;
            }

            int headerEnd = 0;
            int bodyLength = 0;
            if (!isRequestComplete(clients[i].received, headerEnd, bodyLength))
            {
                continue;
            }

            HttpResponse response;
            try
            {
                HttpRequest request = parseRequest(clients[i].received, headerEnd, bodyLength);
                response = handler.handleRequest(request);
            }
            catch (const exception& error)
            {
                response.status = 400;
                response.contentType = "text/plain";
                response.body = error.what();
            }

            sendResponse(clients[i].socket, response);
            closeSocket(clients[i].socket);
            clients.erase(clients.begin() + i);
            i--;
        }

        time_t now = time(nullptr);
        for (int i = 0; i < (int)clients.size(); i++)
        {
            if (now - clients[i].lastActive > 10)
            {
                closeSocket(clients[i].socket);
                clients.erase(clients.begin() + i);
                i--;
            }
        }
    }
}

}
