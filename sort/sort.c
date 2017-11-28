// Михалищев Артем КБ-401

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int cmp(const void *a, const void *b)
{
    if (*(long long*)a < *(long long *)b) return -1;
    else if (*(long long*)a == *(long long *)b) return 0;
    else return 1;
}

int findnum(int fd, long long *n)
{
    char decarr[19];
    char symb;
    int j=0;
    int skip=0, sgn=0;
    for(int i=0; i<19; ++i) decarr[i]=0;
    while (read(fd, &symb, 1) > 0)
    {
        // Пропускаем слишком большие числа
        if (skip) {
            if ((symb >= '0') && (symb <= '9')) continue;
            else {
                for(int i=0; i<19; ++i) decarr[i]=0;
                j = 0;
                skip = 0;
                sgn = 0;
            }
        }
        // Собираем число
        if ((!j) && (symb == '-')) sgn = 1;
        else if ((symb >= '0') && (symb <= '9')) {
            if (j < 19) {
                decarr[j] = symb;
                ++j;
            }
            else {
                fprintf(stderr, "Found too big number\n");
                skip = 1;
            }
        }
        // Возврат числа
        else {
            if (!j) {
                sgn = 0;
                continue;
            }
            else {
                *n = atoll(decarr);
                if (sgn) *n = -*n;
                return 0;
            }
        }
    }
    if (j) {
        *n = atoll(decarr);
        if (sgn) *n = -*n;
        return 0;
    }
    // Конец файла
    return 1;
}

int main(int argcnt, char *argv[])
{
    // Проверка аргументов
    if (argcnt < 3)
    {
        fprintf(stderr, "Usage: %s input_file [input_file2 ...]  output_file\n", argv[0]);
        return 1;
    }
    // Открываем выходной файл
    int fdout = open(argv[argcnt-1], O_WRONLY|O_EXCL|O_CREAT, 0666);
    if (fdout < 0)
    {
        fprintf(stderr, "Can't open output file\n");
        return 2;
    }
    // Переменные для обработки
    long long n;
    long long arrlen=0;
    long long *arr = (long long *) malloc(0 * sizeof(long long));
    if (!arr)
    {
        fprintf(stderr, "Malloc error\n");
        close(fdout);
        return 3;
    }
    // Цикл по именам файлов
    for (int i=1; i<argcnt-1; ++i)
    {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Can't open input file %s\n", argv[i]);
            continue;
        }
        // Разбор чисел
        while (findnum(fd, &n) == 0)
        {
            ++arrlen;
            arr = realloc(arr, arrlen*sizeof(long long));
            if (!arr) {
                fprintf(stderr, "Realloc error\n");
                close(fdout);
                close(fd);
                return 4;
            }
            arr[arrlen-1] = n;
        }
        close(fd);
    }
    // Сортировка и вывод
    qsort(arr, arrlen, sizeof(long long), cmp);
    for (long long int i=0; i<arrlen; ++i) dprintf(fdout, "%lld\n", arr[i]);
    free(arr);
    close(fdout);
    return 0;
}