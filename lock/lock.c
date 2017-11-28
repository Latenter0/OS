// Михалищев Артем КБ-401

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argcnt, char *argv[])
{
    // Проверка аргументов
    if (argcnt != 3) {
        fprintf(stderr, "Usage: %s file text\n", argv[0]);
        return 1;
    }
    char *lckname;
    char *lckstr = ".lck";
    strcpy(lckname, argv[1]);
    strcat(lckname, lckstr);
    printf("Waiting for unlock\n");
    while (1)
    {
        int fd = open(lckname, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        if (fd >= 0) {
            pid_t pid = getpid();
            if (dprintf(fd, "w %d\n", pid) < 0) {
                fprintf(stderr, "Can't write to (%s)\n", lckname);
                return 2;
            }
            close(fd);
            break;
        }
    }
    printf("File is unlocked\n");
    FILE *fd = fopen(argv[1], "a");
    if (!fd) {
        fprintf(stderr, "Can't open %s\n", argv[1]);
        return 3;
    }
    // Дописываем текст в файл
    if (fprintf(fd, "%s\n", argv[2]) < 0) {
        fprintf(stderr, "Can't write to %s\n", argv[1]);
        return 4;
    }
    fclose(fd);
    // Для проверки
    sleep(5);
    // Удаляем файл блокировки
    if (remove(lckname) < 0) {
        fprintf(stderr, "Can't remove %s\n", lckname);
        return 5;
    }
    return 0;
}