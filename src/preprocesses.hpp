#pragma once

#define _GLIBCXX_HAS_GTHREADS


#include<iostream>
#include<thread>
#include<chrono>
#include<string>
#include<cstring>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<fstream>
#include<conio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_NICKNAME "User"
#define DEFAULT_PORT 65374
#define BUFFER_SIZE 1024
#define CLIENT_CLOSE_CONNECTION_SYMBOL '#'