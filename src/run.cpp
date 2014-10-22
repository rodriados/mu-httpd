#include <iostream>
#include <cstdlib>
#include <thread>

#include "run.h"
#include "colors.h"
#include "HTTP.h"

using namespace std;

void log();
void execute(Socket, const string&);
void process(Socket, AddressIn, thread *);

void run(Socket server){

	Socket client;
	AddressIn remoteaddr;
	thread *parallel;
	unsigned int length = sizeof(remoteaddr);

	while(true){

		client = accept(server, (Address *)&remoteaddr, &length);
		parallel = new thread(process, client, remoteaddr, parallel);

	}

}

void process(Socket client, AddressIn remote, thread *actual){

	int bytes;
	char buffer[1000];
	string received;

	received.clear();
	bytes = recv(client, buffer, 999, 0);

	while(bytes > 0){
		buffer[bytes] = '\0';
		received += buffer;
		bytes = recv(client, buffer, 999, MSG_DONTWAIT);
	}

	execute(client, received);
	close(client);
}

void execute(Socket client, const string& received){

	HTTP::Request request(received);
	HTTP::Response response(request);

	string content;
	int length = response.generate(content);

	logall << (request.method+" "+request.target+" "+request.protocol);
	send(client, content.c_str(), length, 0);

}