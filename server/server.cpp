/*
Author: Keaton Shelton
Date: October 26th, 2022
Language: C++20
Inputs: a server ip (string), a port (string to int), and a logfile (string)
Returns: A logfile containing message from server

Abstract: This program is a client program that allows
    a user to enter a server ip, a port, and a logfile name at launch.
    It then will valilate the users input. At that point the program
    will then take the users message, verify message, and finally send.
    The program will then await the servers response.

    Note: Order of execution specified by ProjectAssignment1 has been changed for 
        small optimizations during connection to shorten connection time

Revisions:
01ks - October 26th, 2022 - Original
02ks - October 27th, 2022 - Finish Parts 4 -- 6
03ks - October 27th, 2022 - Cleanup code
*/
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <vector>

using namespace std;



int main(int argc, char *argv[])
{

    //Setup Variables
    int new_socket, valread, server_fd, rv, MAX_ITERATIONS;
    uint16_t port;
    char answer;
    ifstream inFile;
    ofstream outFile;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    vector<string> messages;
    bool cont = true, iterations = false;

    //Input Verification
    if(argc == 3) 
    {
        //Good
        iterations = false;
    }
    else if(argc == 4)
    {
        //Good
        for (int i = 0; i < strlen(argv[3]); i++)
        {
            if(isdigit(argv[3][i])) 
            {
                //Good
                iterations = true;
            }
            else
            {
                cout << "Error: Wrong input arguments, please enter:" << endl
                << "./start PORT LOGFILE RUN-ITERATIONS(OPTIONAL)" << endl;
                return -1;
            }
        }
    }
    else
    {
        cout << "Error: Wrong input arguments, please enter:" << endl
        << "./start PORT LOGFILE RUN-ITERATIONS(OPTIONAL)" << endl;
        return -1;
    }
    for (int i = 0; i < strlen(argv[1]); i++)
    {
        if(isdigit(argv[1][i])) 
        {
            //Good
        }
        else
        {
            cout << "Error: Wrong input arguments, please enter:" << endl
            << "./start PORT LOGFILE RUN-ITERATIONS(OPTIONAL)" << endl;
            return -1;
        }
    }
    cout << "Initial Variables have been auto checked as valid" << endl
    << "Please validate if these are correct:" << endl
    << "Port: " << argv[1] << endl
    << "Logfile Name: " << argv[2] << endl;
    if(iterations)
    {
        cout << "Run Iterations: " << argv[3] << endl << endl;
    }
    else
    {
        cout << "Run Iterations: " << "Unlimited" << endl << endl;
    }
    //Verify Input
    do
    {
        cout << "Please enter Y/n: " << endl;
        cin >> answer;
    } while (!cin.fail() && answer!='y' && answer!='n' && answer != 'Y' && answer != 'N');
    cin.clear();

    if (answer == 'n' || answer == 'N') 
    {
        cout << "Wrong input validated, terminating program" << endl;
        return -1;
    }
    else
    {
        port = stoi(argv[1]);
        MAX_ITERATIONS = stoi(argv[3]);
    }


    //Load Messages
    inFile.open("quotes.txt", ios::in);
    if(!inFile.good()) 
    {
        cout << "Error finding quotes file, terminating program" << endl;
        return -1;
    }
    string temp;
    while(getline(inFile, temp))
    {
        messages.push_back(temp);
    }
    inFile.close();
    int size = messages.size();
    //Create Socket Object
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Socket Creation Error" << endl;
        return -1;
    }

    //Attach Port to Socket
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        cout << "Error attaching port to socket" << endl;
        return -1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    //Bind Socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        cout << "Error binding socket" << endl;
        return -1;
    }

    int counter = 0;
    //Main Loop for listening
    do
    {
        //Listen for packets, max queue 3
        if (listen(server_fd, 3) < 0) 
        {
            cout << "Listening Error" << endl;
            return -1;
        }
        //Get first connection from queue
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
        {
            cout << "Packet Queue Error" << endl;
            return -1;
        }
        //Reset Buffer
        memset(buffer, 0, sizeof(buffer));
        //Read message from client
        valread = read(new_socket, buffer, 1024);
        cout << "Message Recieved:" << endl
        << buffer << endl << endl;
        string temp = buffer;
        //Save message to file
        outFile.open(argv[2], ios_base::app | ios_base::out);
        if(outFile.fail())
        {
            cout << "Error finding file, terminating program" << endl;
            return -1;
        }
        outFile << buffer << endl;
        outFile.close();
        if(temp.find("network") != string::npos)
        {
            //Increase Count
            counter++;
            //Get Random Message
            int ranNum = rand()%30;
            const char* message = messages[ranNum].c_str();
            //Send Reply
            cout << "Sending Message: " << message << endl;
            cout << "Message Count: " << counter << endl << endl;
            send(new_socket, message, strlen(message), 0);
        }
        else
        {
            //Increase Count
            counter++;
            //Bad message
            cout << "Bad message recieved! Not sending a Reply" << endl;
            cout << "Message Count: " << counter << endl << endl;
        }
        if(iterations) 
        {
            if (counter == MAX_ITERATIONS)
            {
                cont = false;
            }
        }
    } while (cont);
    return 0;
}