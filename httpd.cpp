#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "run.h"
#include "colors.h"

using namespace std;

void hello();
void abort(AddressIn);
void confirm(AddressIn);

int main(int argc, const char **argv){

	Socket server;
	AddressIn localaddr;

	hello();

	memset(&localaddr, 0, sizeof(AddressIn));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(argc > 1 ? atoi(argv[1]) : 8080);

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(server, (Address *)&localaddr, sizeof(AddressIn));
	listen(server, 20);

	errno ? abort(localaddr) : confirm(localaddr);

	run(server);
	close(server);

	style(RESETALL);

	return 0;

}

void hello(){

	style(BRIGHT);
	cout << endl;
	cout << SPACE << "HTTPd Hipertext Transfer Protocol Server." << endl;
	cout << endl;

	style(RESETALL);

}

void abort(AddressIn addr){

	foreground(RED);
	style(BRIGHT);
	cout << SPACE << "Erro:" << endl;
	cout << SPACE << "Não foi possível vincular-se ao endereço ";
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;

	style(RESETALL);
 	cout << endl;

 	exit(1);

}

void confirm(AddressIn addr){

	foreground(GREEN);
	style(BRIGHT);
	cout << SPACE << "Sucesso!" << endl;
	cout << SPACE << "Esperando conexões no endereço ";
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;

	style(RESETALL);
 	cout << endl;

}
