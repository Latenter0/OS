// Михалищев Артем КБ-401

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argcnt, char *argv[])
{
    // Проверка числа аргументов
    if (argcnt != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_WRONLY|O_EXCL|O_CREAT, 0666);
    // Проверка дескриптора
    if (fd < 0) {
        fprintf(stderr, "File exists or you can't write in");
        return fd;
    }
    char symb;
    int cnt=0; // Длина цепочки нулей
    while (read(STDIN_FILENO, &symb, 1) > 0)
    {
        if (symb == 0) ++cnt; // Увеличение смещения
        else if (cnt) {
            // Смещение
            if (lseek(fd, cnt, SEEK_CUR) <= 0) {
                fprintf(stderr, "Lseek error1");
                return 2;
            }
            cnt = 0;
            // Запись символа
            if (write(fd, &symb, 1) <= 0) {
                fprintf(stderr, "Write error1");
                return 3;
            }
        }
        // Запись символа x2
        else if (write(fd, &symb, 1) <= 0) {
            fprintf(stderr, "Write error2");
            return 4;
        }
    }
    // Если последний - 0
    if (symb == 0) {
        if (cnt > 1) if (lseek(fd, cnt-1, SEEK_CUR) <= 0) {
            fprintf(stderr, "Lseek error2");
            return 5;
        }
        if (write(fd, &symb, 1) <= 0) {
            fprintf(stderr, "Write error3");
            return 6;
        }
    }
    close(fd);
    return 0;
}