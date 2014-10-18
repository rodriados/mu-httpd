#include <unistd.h>
#include <iostream>
#include <cstdlib>

#include "run.h"
#include "colors.h"

using namespace std;

void run(Socket sock){

	Socket client;
	AddressIn remoteaddr;
	unsigned int length = sizeof(remoteaddr);
	int n;
	char buffer[1000];
	string content;

	while(true){
		content.clear();
		client = accept(sock, (Address *)&remoteaddr, &length);
		n = recv(client, buffer, 10, 0);

		while(n > 0){
			buffer[n] = '\0';
			content += buffer;
			n = recv(client, buffer, 10, MSG_DONTWAIT);
		}

		cout << content.size() << endl;
		cout << content << endl;

	}

}