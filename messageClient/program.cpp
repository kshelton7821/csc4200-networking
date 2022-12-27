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

using namespace std;



int main(int argc, char *argv[])
{
    try
    {
    //Setup Variables
    int sock = 0, valread, client_fd, rv;
    uint16_t port;
    char answer;
    ofstream outfile;
    struct sockaddr_in serv_addr;
    string temp, temp3;
    char buffer[1024] = { 0 };
    fd_set set;
    struct timeval timeout;




    //1. Check inputs
    if (argc != 4)
    {
        cout << "Error: Wrong input arguments, please enter:" << endl
        << "./client ADDRESS PORT LOGFILE" << endl;
        return -1;
    }
    for (int i = 0; i <= strlen(argv[2]) - 1; i++)
    {
        if(isdigit(argv[2][0])) 
        {
            //Good
        }
        else
        {
            cout << "Error: Wrong input arguments, please enter:" << endl
            << "./client ADDRESS PORT LOGFILE" << endl;
            return -1;
        }
    }

    string temp2 = argv[1];

    //1 & 4. Take user input
    cout << "Initial Variables have been auto checked as valid" << endl
    << "Please validate if these are correct:" << endl
    << "Address: " << argv[1] << endl
    << "Port: " << argv[2] << endl
    << "Logfile Name: " << argv[3] << endl << endl;

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
        port = stoi(argv[2]);
    }

    cout << "What message would you like to send to the server?" << endl;
    getline(cin,temp);
    getline(cin,temp);

    const char* message = temp.c_str();
    temp3 = "./launchTCP.sh " + temp2 + " &";
    const char* launcher = temp3.c_str();
    //Launch TCPDUMP
    //system(launcher);

    //2. Create Socket Object
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        cout << endl << "Socket Creation Error, exiting program" << endl;
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    //Convert address to binary
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        cout << "Invalid Address Error, terminating program" << endl;
        return -1;
    }

    //3. Connect to server
    if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
    {
        cout << "Connection Failed Error, terminating program" << endl;
        return -1;
    }
    cout << "Connection Successfull" << endl;

    FD_ZERO(&set);
    FD_SET(sock, &set);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;



    //5. Send Message
    send(sock, message, strlen(message), 0 );
    cout << "Message Sent to server" << endl;
    //6. Recieve Message / Check for recieve
    rv = select(sock + 1, &set, NULL, NULL,  &timeout);
    if(rv == -1)
    {
        cout << "Error has occured, terminating program" << endl;
        return -1;
    }
    else if(rv == 0)
    {
        cout << "Timeout has occured, no message recieved, terminating program" << endl;
        return -1;
    }
    else
    {
       valread = read(sock, buffer, 1024); 
    }

    cout << "Message Recieved from Server:" << endl
    << buffer << endl;
    //7. Save Message
    outfile.open(argv[3], ios_base::app | ios_base::out);
    if(outfile.fail())
    {
        cout << "Error Finding file, terminating program" << endl;
        return -1;
    }
    outfile << buffer << endl;
    outfile.close();


    //8.Close Socket
    close(client_fd);

    return 0; 
    }
    catch(const exception& e)
    {
        cerr << e.what() << endl;
        return -1;
    }
}