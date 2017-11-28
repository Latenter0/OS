// Михалищев Артем КБ-401

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>

char * confpath;
const char confname[]="watch.conf";

struct row
{
    char str[300];
    int tp;
    int k;
    pid_t pid;
};

int gettype(char * tmp)
{
    int i=0;
    switch (tmp[strlen(tmp)-2])
    {
        case 'W':
            i=0;
            break;
        case 'R':
            i=1;
            break;
        default:
            i=-1;
            break;
    }
    tmp[strlen(tmp)-2]=0;
    return i;
}

int work()
{
    // Считаем кол-во строк в конф. файле=кол-во процессов
    FILE *fd = fopen(confpath, "r");
    if (!fd) {
        fprintf(stderr, "Can't open configuration file\n");
        exit(2);
    }
    int K=0;
    while (1)
    {
        char symb = fgetc(fd);
        if (symb == EOF) break;
        if (symb == '\n') ++K;
    }
    struct row cmds[K];
    fclose(fd);
    // Читаем, парсим данные о процессах
    fd = fopen(confpath, "r");
    if (!fd) {
        fprintf(stderr, "Can't open configuration file\n");
        exit(2);
    }
    int i=0;
    while (1)
    {
        char * tmp = fgets(cmds[i].str, 300, fd);
        if (!tmp) break;
        cmds[i].tp = gettype(tmp);
        cmds[i].k = 50;
        if (cmds[i].tp < 0) {
            fprintf(stderr, "Bad configuration file\n");
            exit(3);
        }
        ++i;
    }
    fclose(fd);
    char *rtmp;
    char tmp[310];
    char *rargs[310];
    char configfile[25];
    for(int i=0; i<K; ++i)
    {
        switch (cmds[i].pid=fork())
        {
            case -1:
                fprintf(stderr, "Can't start fork\n");
                exit(1);
            // Запускаем каждый процесс execvp
            case 0:
                strcpy(tmp, cmds[i].str);
                rtmp = strtok(tmp, " ");
                for (int i=0; i<300; ++i)
                {
                    if (rtmp != NULL) {
                        rargs[i] = (char *) malloc(strlen(tmp));
                        strcpy(rargs[i], rtmp);
                        rtmp = strtok(NULL, " ");
                    }
                    else rargs[i] = NULL;
                }
                if (execvp(rargs[0], rargs) < 0) {
                    fprintf(stderr, "Exec error\n");
                    exit(4);
                }
                exit(0);
                break;
             // Создаём и записываем идентификатор в файл
        default:
            sprintf(configfile, "/tmp/watch.%d.pid", cmds[i].pid);
            FILE *fd = fopen(configfile, "w");
            if (!fd) syslog(LOG_WARNING, "Can't create /tmp/watch.%d.pid", cmds[i].pid);
            else syslog(LOG_WARNING, "Created /tmp/watch.%d.pid for %s", cmds[i].pid, cmds[i].str);
            if (fprintf(fd, "%i", cmds[i].pid) < 0) syslog(LOG_WARNING, "Can't write in /tmp/watch.%d.pid", cmds[i].pid);
            fclose(fd);
            break;
        }
    }
    // Обработчик HUP, сбрасываем настройку и запускаем work снова
    void hup (int sig)
    {
        for (int i=0; i<K; ++i)
        {
            if (cmds[i].pid > 0) {
                kill(cmds[i].pid, SIGKILL);
                sprintf(configfile, "/tmp/watch.%d.pid", cmds[i].pid);
                if (remove(configfile) < 0) syslog(LOG_WARNING, "Can't remove /tmp/watch.%d.pid", cmds[i].pid);
            }
        }
        work();
    }
    signal(SIGHUP, hup);
    // Обработка завершённых
    while (1)
    {
        int cur = 0;
        for (int i=0; i<K; ++i) if (cmds[i].pid) ++cur;
        if (!cur) exit(0);
        int status;
        pid_t pid = wait(&status);
        for (int i=0; i<K; ++i)
        {
            if (pid == cmds[i].pid) {
                sprintf(configfile, "/tmp/watch.%d.pid", cmds[i].pid);
                if (remove(configfile) < 0) syslog(LOG_WARNING, "Can't remove /tmp/watch.%d.pid", cmds[i].pid);
                if (status) cmds[i].k -= 1;
                if ((!status) && (!cmds[i].tp)) cmds[i].pid=0;
                else if (cmds[i].tp == 1) {
                    // флаг для сна
                    int sleepfl=0;
                    if (cmds[i].k < 0) {
                        cmds[i].k = 50;
                        sleepfl = 1;
                        syslog(LOG_NOTICE, "%s exec failed, wait for 1 hour", cmds[i].str);
                    }
                    // перезапуск соответсвующик R процессов
                    switch(cmds[i].pid=fork())
                    {
                        case -1:
                            fprintf(stderr, "Can't start fork\n");
                            exit(1);
                            break;
                        case 0:
                            strcpy(tmp, cmds[i].str);
                            rtmp = strtok(tmp, " ");
                            for(int i=0; i<300; ++i)
                            {
                                if (rtmp) {
                                    rargs[i] = (char *)malloc(strlen(tmp));
                                    strcpy(rargs[i], rtmp);
                                    rtmp = strtok(NULL, " ");
                                }
                                else rargs[i] = NULL;
                            }
                            if (sleepfl) sleep(3600);
                            if (execvp(rargs[0], rargs) < 0) {
                                fprintf(stderr, "Exec error\n");
                                exit(4);
                            }
                            exit(0);
                            break;
                        default:
                            sprintf(configfile, "/tmp/watch.%d.pid", cmds[i].pid);
                            FILE *fd = fopen(configfile, "w");
                            if (!fd) syslog(LOG_WARNING, "Can't create /tmp/watch.%d.pid", cmds[i].pid);
                            if (fprintf(fd, "%i", cmds[i].pid) < 0) syslog(LOG_WARNING, "Can't write in /tmp/watch.%d.pid", cmds[i].pid);
                            fclose(fd);
                            break;
                    }
                }
            }
        }
    }
    return 0;
}


int main(int argcnt, char *argv[])
{
    // Путь до файла конфигурации
    char *cpath = realpath(argv[0], NULL);
    int i = strlen(cpath)-1;
    for(; i>=0; --i) if (cpath[i] == '/') break;
    if (i == -1) {
        fprintf(stderr, "Can't find configuration\n");
        return 1;
    }
    else {
        char *confpth = (char *) malloc(i+12*sizeof(char *));
        for (int j=0; j<=i; ++j) confpth[j] = cpath[j];
        for (int j=0; j<11; ++j) confpth[j+i+1] = confname[j];
        confpath = confpth;
    }
    int pid;
    switch (pid=fork())
    {
        case -1:
            fprintf(stderr, "Can't start fork\n");
            return 1;
        case 0:
            umask(0);
            setsid();
            chdir("/");
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            work();
            break;
        default:
            break;
    }
    return 0;
}