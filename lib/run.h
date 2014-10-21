#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Log.h"

typedef sockaddr Address;
typedef sockaddr_in AddressIn;
typedef int Socket;

extern void run(Socket);