#include <cstdlib>
#include <fstream>
#include <iostream>

#include "ApiHandler.hpp"
#include "Errors.hpp"
#include "HttpServer.hpp"

using namespace std;
using namespace expense_tracker;

int main(int argc, char* argv[])
{
    int port = 8080;
    if (argc > 1)
    {
        port = atoi(argv[1]);
        if (port < 1 || port > 65535)
        {
            cout << "Port must be between 1 and 65535" << endl;
            return 1;
        }
    }

    ifstream pageFile("web/index.html");
    if (!pageFile.is_open())
    {
        cout << "Cannot find the web folder. Please run the program from the PBL Project folder." << endl;
        return 1;
    }
    pageFile.close();

    try
    {
        ApiHandler handler("web", "data");
        HttpServer server(handler);

        cout << "PBL Project is running." << endl;
        cout << "Open http://localhost:" << port << " in your browser." << endl;
        cout << "Press Ctrl+C to stop." << endl;
        server.run(port);
    }
    catch (const ExpenseError& error)
    {
        cout << "Error: " << error.what() << endl;
        return 1;
    }
    return 0;
}
