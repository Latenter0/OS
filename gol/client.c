// Михалищев Артем КБ-401

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int main() {
    struct sockaddr_in server;
    struct hostent *host;
    int sockclnt = socket(AF_INET, SOCK_STREAM, 0);
    if (sockclnt < 0) {
        fprintf(stderr, "Can't create socket\n");
        exit(9);
    }
    host = gethostbyname("localhost");
    if (!host) {
        fprintf(stderr, "Can't get host\n");
        close(sockclnt);
        exit(9);
    }
    server.sin_family = AF_INET;
    bcopy((char *)host->h_addr, (char *) &server.sin_addr.s_addr, host->h_length);
    server.sin_port = htons(14285);

    if (connect(sockclnt, (struct sockaddr*) &server, sizeof(server)) < 0) {
        fprintf(stderr, "Can't connect\n");
        close(sockclnt);
        exit(11);
    }
    char symb;
    while (read(sockclnt, &symb, 1) > 0) printf("%c", symb);
    close(sockclnt);
    return 0;
}