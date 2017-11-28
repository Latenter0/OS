// Михалищев Артем КБ-401

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

int X;
int Y;
int xoff[] = {-1,  0,  1, -1, 1, -1, 0, 1};
int yoff[] = {-1, -1, -1,  0, 0,  1, 1, 1};
int ** gfield;
const char *tempfile = "/tmp/gol.tmp";

void serv()
{
    int socksrv = socket(AF_INET, SOCK_STREAM, 0);
    if (socksrv < 0) {
        fprintf(stderr, "Can't create socket\n");
        exit(9);
    }
    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(14285);
    if (bind(socksrv, (struct sockaddr *) &server, sizeof(server)) < 0) {
        fprintf(stderr, "Can't bind 14285\n");
        exit(10);
    }
    listen(socksrv, 10);
    unsigned int clientsize = sizeof(client);
    while (1)
    {
        int sockclnt = accept(socksrv, (struct sockaddr *) &client, &clientsize);
        if (sockclnt < 0) {
            fprintf(stderr, "Can't connect\n");
            close(socksrv);
            exit(11);
        }
        FILE *fd = fopen(tempfile, "r");
        if (!fd) {
            fprintf(stderr, "Can't open temp file\n");
            close(sockclnt);
            close(socksrv);
            exit(2);
        }
        while(1)
        {
            char symb = fgetc(fd);
            if (symb == EOF) break;
            int n = write(sockclnt, (void*)&symb, 1);
            if (n < 0) {
                fprintf(stderr, "Can't write to socket\n");
                fclose(fd);
                close(sockclnt);
                close(socksrv);
                exit(12);
            }
        }
        fclose(fd);
        close(sockclnt);
    }
    close(socksrv);
}

// Проверка на живую клетку
int isalive(int x, int y)
{
    if ((x<0) || (x>=X) || (y<0) || (y>=Y)) return 0;
    else return gfield[y][x];
}

// Пересчёт
void move()
{
    int ** gmap = (int **) malloc(Y * sizeof(int *));
    for(int i=0; i<Y; ++i) gmap[i] = (int *) malloc(X * sizeof(int));
    time_t start = time(NULL);
    for (int i=0; i<Y; ++i)
    {
        for (int j=0; j<X; ++j)
        {
            int alive = 0;
            for (int k=0; k<8; ++k) alive += isalive(j+xoff[k], i+yoff[k]);
            if ((gfield[i][j] == 0) && (alive == 3)) gmap[i][j] = 1;
            else if ((gfield[i][j] == 1) && ((alive < 2) || (alive > 3))) gmap[i][j] = 0;
            else gmap[i][j] = gfield[i][j];
        }
    }
    double seconds = difftime(time(NULL), start);
    if (seconds > 1.0) fprintf(stderr, "Fail (not in time)\n");
    else {
        int **tmp = gfield;
        gfield = gmap;
        gmap = tmp;
    }
    for (int i=0; i<Y; ++i) free(gmap[i]);
    free(gmap);
}

void work(int sgn)
{
    // Повторять каждую секунду
    alarm(1);
    // Запись в временный файл
    FILE *fd = fopen(tempfile, "w");
    if (!fd) {
        fprintf(stderr, "Error: Can't open temp file\n");
        exit(2);
    }
    for (int i=0; i<Y; ++i)
    {
        for (int j=0; j<X; ++j)
            if (fprintf(fd, "%i", gfield[i][j]) < 0) 
                fprintf(stderr, "Error: Can't write to temp file\n");
        if (fprintf(fd, "\n") < 0) fprintf(stderr, "Error: Can't write to temp file\n");
    }
    fclose(fd);
    move();
    return;
}

int main(int argcnt, char *argv[])
{
    // Проверка Аргументов
    if (argcnt != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }
    FILE *fd = fopen(argv[1], "r");
    if (!fd)
    {
        fprintf(stderr, "Can't open file\n", argv[1]);
        return 2;
    }
    int x=0;
    int y=0;
    int tmp=0;
    // Узнаём размер карты
    while (1)
    {
        char symb = fgetc(fd);
        if (symb == EOF) break;
        if ((symb == '0') || (symb == '1')) ++tmp;
        else if (symb == '\n') {
            if (!x) x = tmp;
            else if (x != tmp) {
                fprintf(stderr, "Bad map\n");
                return 3;
            }
            ++y;
            tmp = 0;
        }
        else {
            fprintf(stderr, "Only 0 or 1\n");
            return 4;
        }
    }
    fclose(fd);
    // Считываем карту
    fd = fopen(argv[1], "r");
    if (!fd)
    {
        fprintf(stderr, "Can't open file %s\n", argv[1]);
        return 2;
    }
    int ** gmap = (int **) malloc(y * sizeof(int *));
    for(int i=0; i<y; ++i)
    {
        gmap[i] = (int *) malloc(x * sizeof(int));
        for(int j=0; j<x; ++j)
        {
            char symb = fgetc(fd);
            if (symb == '\n') symb = fgetc(fd);
            gmap[i][j] = symb-'0';
        }
    }
    fclose(fd);
    X=x;
    Y=y;
    gfield=gmap;
    pid_t pid;
    switch (pid=fork())
    {
        case -1:
            fprintf(stderr, "Can't fork\n");
            return 7;
        case 0:
            serv();
            break;
        default:
            signal(SIGALRM, work);
            alarm(1);
            wait(NULL);
            break;
    }
}