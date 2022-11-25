/*
Author: Keaton Shelton
Date: October 26th, 2022
Language: C++20
Inputs: a port (string to int) and a logfile (string) 
Returns: A logfile containing message from server

Abstract: This program is a server program that allows
    a user to enter a port and a logfile name.
    It then will valilate the users input. At that point the program
    will then wait for incoming packets and respond if keyword is met.
    The program will then await the clients response.

Revisions:
01ks - November 20th, 2020 - Original
02ks - November 21st, 2020 - More progress, need to test
02ks - November 22nd-25th, 2020 - Finally working, this has been way more difficult than I thought.
*/
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include "packets.h"

using namespace std;



int main(int argc, char *argv[])
{
    //Setup Variables
    int new_socket, valread, server_fd, clientSockets[3], maxClients = 3, maxSD, SD, actMonitor, MAX_ITERATIONS;
    uint16_t port;
    char answer;
    ofstream outFile;
    struct sockaddr_in address;
    int addrlen;
    int opt = 1;
    char buffer[1024] = {0};
    fd_set readfds;
    bool RUN = true, command = false, reset = true;
    string packetPiece, temp2;
    Packet recievedPacket;

    //Input Verification
    if(argc == 3) 
    {
        //Good
    }
    else
    {
        cout << "Error: Wrong input arguments, please enter:" << endl
        << "./start PORT LOGFILE" << endl;
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
            << "./start PORT LOGFILE" << endl;
            return -1;
        }
    }
    cout << "Initial Variables have been auto checked as valid" << endl
    << "Please validate if these are correct:" << endl
    << "Port: " << argv[1] << endl
    << "Logfile Name: " << argv[2] << endl;
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
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    //Initialize Client Conns
    for (int i = 0; i < maxClients; i++)
    {
        clientSockets[i] = 0;
    }
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
    //Bind Socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        cout << "Error binding socket" << endl;
        return -1;
    }
    //Listen for packets, max queue 3
    if (listen(server_fd, maxClients) < 0)
    {
        cout << "Listening Error" << endl;
        return -1;
    }

    addrlen = sizeof(address);
    //Main Loop
    do
    {
        //Clear Buffer
        memset(buffer, 0, sizeof(buffer));
        //Setup Sets
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        maxSD = server_fd;
        //Add child sockets to set
        for (int i = 0; i < maxClients; i++)
        {
            SD = clientSockets[i];
            if (SD > 0)
            {
                FD_SET(SD, &readfds);
            }
            if (SD > maxSD)
            {
                maxSD = SD;
            }
        }
        //Wait for activity
        actMonitor = select(maxSD + 1, &readfds, NULL, NULL, NULL);
        if (actMonitor < 0 && errno != EINTR)
        {
            cout << "Select Error" << endl;
            return -1;
        }
        //Check Incoming connections
        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
            {
                cout << "Error accepting connection" << endl;
                return -1;
            }
            //Add new socket to array of sockets
            for (int i = 0; i < maxClients; i++)
            {
                if (clientSockets[i] == 0)
                {
                    clientSockets[i] = new_socket;
                    break;
                }
            }
        }
        //Other IO on existing socket
        for (int i = 0; i < maxClients; i++)
        {
            SD = clientSockets[i];
            if (FD_ISSET(SD, &readfds))
            {
                //Check if it was for closing, and read the incoming message
                if ((valread = read(SD, buffer, 1024)) == 0)
                {
                    //Close the socket and mark as 0 in list for reuse
                    close(SD);
                    clientSockets[i] = 0;
                }
                else
                {
                    //Check if the packet is a command
                    //Read message from client
                    string temp = buffer;
                    istringstream ss(temp);
                    //Version
                    getline(ss, packetPiece, ';');
                    recievedPacket.version = stoul(packetPiece, NULL, 10);
                    //Type
                    getline(ss, packetPiece, ';');
                    recievedPacket.type = stoul(packetPiece, NULL, 10);
                    //Length
                    getline(ss, packetPiece, ';');
                    recievedPacket.length = stoul(packetPiece, NULL, 10);
                    //Message
                    getline(ss, packetPiece, ';');
                    memset(recievedPacket.message, 0, sizeof(recievedPacket.message));
                    copy(packetPiece.begin(), packetPiece.end(), begin(recievedPacket.message));
                    //Output to screen
                    cout << "Recieved Connection from (IP,PORT): (" << inet_ntoa(address.sin_addr) << ", " << ntohs(address.sin_port) << ")" << endl;
                    cout << "Recieved Data: version: " << recievedPacket.version << " message_type: " << recievedPacket.type << " length: " << recievedPacket.length << endl;
                    //Save message to file
                    outFile.open(argv[2], ios_base::app | ios_base::out);
                    if(outFile.fail())
                    {
                        cout << "Error finding file, terminating program" << endl;
                        return -1;
                    }
                    outFile << buffer << endl;
                    outFile.close();
                    if(recievedPacket.version == 17 && recievedPacket.type == 1)
                    {
                        //Begin 1st stage
                        cout << "Version Accepted" << endl;
                        //Setup Packet
                        ostringstream convert;
                        for (int x = 0; x < recievedPacket.length; x++)
                        {
                            convert << recievedPacket.message[x];
                        }
                        temp = convert.str();
                        if (temp.find("HELLO") != string::npos)
                        {
                            cout << "Command Accepted: HELLO" << endl;
                            //Setup Packet
                            temp = "HELLO";
                            copy(temp.begin(),temp.end(), begin(recievedPacket.message));
                            recievedPacket.length = temp.size();
                            temp = to_string(recievedPacket.version) + ";" + to_string(recievedPacket.type) + ";" + to_string(recievedPacket.length) + ";" + temp;
                            //Send Reply
                            send(SD, temp.c_str(), strlen(temp.c_str()), 0);
                            //Goto stage 2
                            command = true;
                        }
                        else if (temp.find("LIGHTON") != string::npos && command)
                        {
                            cout << "Command Accepted: LIGHTON" << endl;
                            //Setup Packet
                            temp = "SUCCESS";
                            copy(temp.begin(),temp.end(), begin(recievedPacket.message));
                            recievedPacket.length = temp.size();
                            temp = to_string(recievedPacket.version) + ";" + to_string(recievedPacket.type) + ";" + to_string(recievedPacket.length) + ";" + temp;
                            //Send Reply
                            send(SD, temp.c_str(), strlen(temp.c_str()), 0);
                            //Goto stage 1
                            command = false;
                        }
                        else if (temp.find("KILL") != string::npos)
                        {
                            cout << "Command Accepted: Disconnect (KILL)" << endl;
                            cout << "Terminating Connection" << endl;
                            //Close Socket
                            close(SD);
                            clientSockets[i] = 0;
                            command = false;
                        }
                        else
                        {
                            cout << "Command Rejected" << endl;
                            //Setup Packet
                            temp = "Command Rejected";
                            copy(temp.begin(),temp.end(), begin(recievedPacket.message));
                            recievedPacket.length = temp.size();
                            temp = to_string(recievedPacket.version) + ";" + to_string(recievedPacket.type) + ";" + to_string(recievedPacket.length) + ";" + temp;
                            //Send Reply
                            send(SD, temp.c_str(), strlen(temp.c_str()), 0);
                            //Goto stage 1
                            command = false;
                        }
                    }
                    else if(recievedPacket.version == 17 && recievedPacket.type == 2) 
                    {
                        //Begin 2nd stage
                        cout << "Version Accepted" << endl;
                        //Check for Command
                        ostringstream convert;
                        for (int x = 0; x < recievedPacket.length; x++)
                        {
                            convert << recievedPacket.message[x];
                        }
                        temp = convert.str();
                        if (temp.find("HELLO") != string::npos)
                        {
                            cout << "Command Accepted: HELLO" << endl;
                            //Setup Packet
                            temp = "HELLO";
                            copy(temp.begin(),temp.end(), begin(recievedPacket.message));
                            recievedPacket.length = temp.size();
                            temp = to_string(recievedPacket.version) + ";" + to_string(recievedPacket.type) + ";" + to_string(recievedPacket.length) + ";" + temp;
                            //Send Reply
                            send(SD, temp.c_str(), strlen(temp.c_str()), 0);
                            //Goto stage 3
                            command = true;
                        }
                        else if (temp.find("LIGHTOFF") != string::npos && command)
                        {
                            cout << "Command Accepted: LIGHTOFF" << endl;
                            //Setup Packet
                            temp = "SUCCESS";
                            copy(temp.begin(),temp.end(), begin(recievedPacket.message));
                            recievedPacket.length = temp.size();
                            temp = to_string(recievedPacket.version) + ";" + to_string(recievedPacket.type) + ";" + to_string(recievedPacket.length) + ";" + temp;
                            //Send Reply
                            send(SD, temp.c_str(), strlen(temp.c_str()), 0);
                            //Return to stage 1
                            command = false;
                        }
                        else if (temp.find("KILL") != string::npos)
                        {
                            cout << "Command Accepted: Disconnect (KILL)" << endl;
                            cout << "Terminating Connection" << endl;
                            //Close Socket
                            close(SD);
                            clientSockets[i] = 0;
                            command = false;
                        }
                        else
                        {
                            //Failed 2nd stage
                            cout << "Command Rejected" << endl;
                            //Setup Packet
                            temp = "ERROR";
                            copy(temp.begin(),temp.end(), begin(recievedPacket.message));
                            recievedPacket.type = 3;
                            recievedPacket.length = temp.size();
                            temp = to_string(recievedPacket.version) + ";" + to_string(recievedPacket.type) + ";" + to_string(recievedPacket.length) + ";" + temp;
                            //Send Reply
                            send(SD, temp.c_str(), strlen(temp.c_str()), 0);
                            command = false;
                        }
                    }
                    else
                    {
                        //Failed 1st stage
                        //Bad message
                        cout << "Bad message recieved or out of order instructions, resetting to listening mode" << endl;
                        cout << "Command Rejected" << endl;
                        //Setup Packet
                        temp = "ERROR";
                        copy(temp.begin(),temp.end(), begin(recievedPacket.message));
                        recievedPacket.type = 3;
                        recievedPacket.length = temp.size();
                        //Send Reply
                        send(SD, temp.c_str(), strlen(temp.c_str()), 0);
                        command = false;
                    }
                }
            }
        }
    } while (RUN);
    //Shutdown
    for (int i = 0; i < maxClients; i++)
    {
        close(clientSockets[i]);
    }
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}