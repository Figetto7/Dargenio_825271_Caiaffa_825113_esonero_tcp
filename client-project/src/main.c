/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif


void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
    //Default Values
    char *server_ip = "127.0.0.1";
    int port = SERVER_PORT;
    char *request_str = NULL;

    // Parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            server_ip = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            if (port <= 0 || port > 65535) {
                printf("Error: port number invalid (1-65535)\n");
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            request_str = argv[i + 1];
            i++;
        }
        else {
            printf("Usage: %s [-s server] [-p port] -r \"type city\"\n", argv[0]);
            return 1;
        }
    }

    // Checks if -r is present
    if (request_str == NULL) {
        printf("Error: -r is mandatory!\n");
        printf("Usage: %s [-s server] [-p port] -r \"type city\"\n", argv[0]);
        return 1;
    }

    // Find type of request and city
    char type;
    char city[64];

    if (sscanf(request_str, "%c %s", &type, city) != 2) {
        printf("Error: invalid form of request\n");
        printf("Correct form: \"t city\" (es: \"t bari\")\n");
        return 1;
    }

    printf("Connection to a %s:%d...\n", server_ip, port);
    printf("Request: type = '%c', city = '%s'\n", type, city);


#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int c_socket;

	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (c_socket < 0) {
	    printf("Socket creation failed\n");
	    clearwinsock();
	    return 1;
	}

	printf("Socket created successfully\n");

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));

	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(server_ip);
	sad.sin_port = htons(port);

	printf("Server address configured: %s:%d\n", server_ip, port);

	if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
	   printf("Connection to server failed\n");
	   closesocket(c_socket);
	   clearwinsock();
	   return 1;
	}

	printf("Connected to server!\n\n");


	// TODO: Implement communication logic
	// send(...);
	// recv(...);

	// Prepara la struttura richiesta
	weather_request_t request;
	request.type = type;           // dal parsing
	strcpy(request.city, city);    // dal parsing

	// Invia al server
	if (send(c_socket, (char*)&request, sizeof(request), 0) < 0) {
	    printf("Errore: send() fallito\n");
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	// Ricevi la risposta
	weather_response_t response;
	int bytes_rcvd = recv(c_socket, (char*)&response, sizeof(response), 0);

	if (bytes_rcvd <= 0) {
	    printf("Errore: recv() fallito\n");
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	printf("Ricevuto risultato dal server ip %s. ", server_ip);

	if (response.status == STATUS_OK) {
	    if (response.type == 't')
	        printf("%s: Temperatura = %.1f°C\n", city, response.value);
	    else if (response.type == 'h')
	        printf("%s: Umidità = %.1f%%\n", city, response.value);
	    else if (response.type == 'w')
	        printf("%s: Vento = %.1f km/h\n", city, response.value);
	    else if (response.type == 'p')
	        printf("%s: Pressione = %.1f hPa\n", city, response.value);
	}
	else if (response.status == STATUS_CITY_NOT_FOUND) {
	    printf("Città non disponibile\n");
	}
	else if (response.status == STATUS_INVALID_REQUEST) {
	    printf("Richiesta non valida\n");
	}

	// Close socket
	closesocket(c_socket);  // ✅

	printf("Client terminated.\n");

	clearwinsock();
	return 0;
} // main end
