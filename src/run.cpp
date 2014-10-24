#include <iostream>
#include <cstdlib>
#include <sstream>
#include <thread>

#include "run.h"
#include "colors.h"
#include "HTTP.h"

using namespace std;

void log();
void execute(Socket, const AddressIn&, const string&);
void process(Socket, const AddressIn&, thread *);

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

void process(Socket client, const AddressIn& remote, thread *actual){

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

	execute(client, remote, received);
	close(client);
}

void execute(Socket client, const AddressIn& remote, const string& received){

	HTTP::Request request(received, remote);
	HTTP::Response response(request);

	string content;
	int length = response.generate(content);

	stringstream logstr;

	logstr << inet_ntoa(remote.sin_addr);
	logstr << " " << request.method + " " + request.target;

	logall << logstr.str();

	send(client, content.c_str(), length, 0);

}