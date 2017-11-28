#!/bin/bash

# Михалищев Артем КБ-401

function begin {
    # Инициализация поля
    for (( i=1; i <= 9; i++ ))
    do
        arr[i]=' '
    done
    # Первый подключившийся играет крестиками
    echo '' | ncat localhost 14285 2>/dev/null
    if [[ $? -ne 0 ]]; then
        symb='X'
        echo 'Ожидаем подключения второго игрока...'
        echo ' ' | ncat -l -p 14285
    else
        symb='O'
    fi
    # Счётчик ходов
    arr[0]=0
    # Символ игрока текущего хода
    cur='X'
}

# Отрисовка поля
function draw {
    clear
    echo "${arr[1]}|${arr[2]}|${arr[3]}"
    echo "-----"
    echo "${arr[4]}|${arr[5]}|${arr[6]}"
    echo "-----"
    echo "${arr[7]}|${arr[8]}|${arr[9]}"
    echo ""
    if [[ $symb == 'X' ]]; then
        echo "Вы играете крестиками"
    else
        echo "Вы играете ноликами"
    fi
}

# Чтение координат и проверка
function rdcoord {
    while true
    do
        read y x
        # Проверка координат на корректность
        if ! [[ $y -ge 1 && $y -le 3 && $x -ge 1 && $x -le 3 ]]; then
            echo 'Неверные координаты'
            continue
        fi
        # Переводим в индекс массива
        crd=$[($y-1)*3+$x]
        # Проверка на возможность сделать ход
        if [[ ${arr[$crd]} == 'X' || ${arr[$crd]} == 'O' ]]; then
            echo 'Клетка занята'
            continue
        fi
        break
    done
}

# Делает ход
function move {
    # Смотрим, чей ход, и получаем координаты
    if [[ $cur == $symb ]]; then
        echo 'Ваш ход'
        echo 'Введите координаты (например: "2 2")'
        # Читаем с клавиатуры
        rdcoord
        arr[$crd]=$cur
        echo "$crd" | ncat -l -p 14285
    else
        echo 'Ход противника'
        echo 'Ожидание...'
        # Читаем с сокета
        stty -echo
        # Ждём, пока придут данные
        while true
        do
            crd=`echo "" | ncat localhost 14285 2>/dev/null`
            if [[ $crd ]]; then
                break
            fi
        done
        arr[$crd]=$cur
        stty echo
    fi
    # Смена хода
    if [[ $cur == 'X' ]]; then
        cur='O'
    else
        cur='X'
    fi
    # Увеличиваем счётчик
    arr[0]=$[${arr[0]}+1]
}

# Проверка условий конца игры
function check {
    # Смотрим, есть ли победитель
    win=' '
    if [[ ${arr[1]} == ${arr[2]} && ${arr[2]} == ${arr[3]} && ${arr[1]} != ' ' ]]; then
        win=${arr[1]}
    elif [[ ${arr[1]} == ${arr[5]} && ${arr[5]} == ${arr[9]} && ${arr[1]} != ' ' ]]; then
        win=${arr[1]}
    elif [[ ${arr[1]} == ${arr[4]} && ${arr[4]} == ${arr[7]} && ${arr[1]} != ' ' ]]; then
        win=${arr[1]}
    elif [[ ${arr[7]} == ${arr[8]} && ${arr[8]} == ${arr[9]} && ${arr[7]} != ' ' ]]; then
        win=${arr[7]}
    elif [[ ${arr[7]} == ${arr[5]} && ${arr[5]} == ${arr[3]} && ${arr[7]} != ' ' ]]; then
        win=${arr[7]}
    elif [[ ${arr[3]} == ${arr[6]} && ${arr[6]} == ${arr[9]} && ${arr[3]} != ' ' ]]; then
        win=${arr[3]}
    elif [[ ${arr[4]} == ${arr[5]} && ${arr[5]} == ${arr[6]} && ${arr[4]} != ' ' ]]; then
        win=${arr[4]}
    elif [[ ${arr[2]} == ${arr[5]} && ${arr[5]} == ${arr[8]} && ${arr[2]} != ' ' ]]; then
        win=${arr[2]}
    fi
    # Если есть 3 в ряд, смотрим, кто победил
    if [[ $win != ' ' ]]; then
        draw
        if [[ $win == $symb ]]; then
            echo 'Победа'
            exit
        fi
        echo 'Поражение'
        exit
    fi
    # Если сделано 9 ходов, то ничья
    if [[ ${arr[0]} == 9 ]]; then
        draw
        echo 'Ничья'
        exit
    fi
}

begin
while true
do
    draw
    move
    check
done