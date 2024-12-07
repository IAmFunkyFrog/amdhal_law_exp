# Отчет 2. MPI

ФИО: Трефилов Степан Захарович

### Задание A-09

```
Написать программу вычисления матричного выражения:

A = 1 + <B x, y>/<x, y> + <B2 x, y>/<x, y>+ <B3 x, y>/<x, y> + <B4 x, y>/<x, y>

Распараллелить эту программу с помощью MPI.
Исследовать зависимость масштабируемости параллельной версии программы от ее
вычислительной трудоемкости (размера матриц).
Проверить корректность параллельной версии.
Проверка закона Амдала. Построить зависимость ускорение:число потоков для заданного
примера.
```

### Тестовый стенд
На тестовом стенде используется AMD Ryzen следующей модели
```
Architecture:            x86_64
  CPU op-mode(s):        32-bit, 64-bit
  Address sizes:         48 bits physical, 48 bits virtual
  Byte Order:            Little Endian
CPU(s):                  12
  On-line CPU(s) list:   0-11
Vendor ID:               AuthenticAMD
  Model name:            AMD Ryzen 5 7535HS with Radeon Graphics
```

### Описание метода решения задачи
Была написана программа на языке Си для вычисления предложенного выражения.

После написания последовательной версии, была написанная парараллельна версия. Параллельность обеспечивается в самом горячем коде - умножении матриц - за счет разделения задачи умножения матрицы между разными процессами. Параллельность делается на уровне вычисления различнх слагаемых в формуле выше, то есть каждое выражение вида $<B^i x, y>$ считается отдельным процессом.

Программный код доступен по [ссылке](https://github.com/IAmFunkyFrog/amdhal_law_exp)

Важное уточнение: программа компилируется с опцией -O0. То есть, в замерах никак не учитываются возмодности процессора по векторизации кода (SIMD).

### Результаты измерений

Каждый результат по времени исполнения некоторой конфигурации программы был получен как среднее 10 запусков. При необходимости можно сделать больше запусков, однако стандартное отклонение довольно низкое для десяти запусков, то есть результаты довольно стабильны.

```
Dims - размерность матрицы
Avg - среднее время вычисление формулы в секундах
Dev - дисперсия вычислений
```

Результаты последовательной версии программы:

| Dims | Avg       | Dev      |
|------|-----------|----------|
|  128 |  0.055277 | 0.000001 |
|  192 |  0.185606 | 0.000003 |
|  256 |  0.506891 | 0.000159 |
|  320 |  0.855422 | 0.000032 |
|  384 |  1.579105 | 0.000183 |
|  448 |  2.347348 | 0.000145 |
|  512 |  4.762909 | 0.000568 |
|  576 |  5.064281 | 0.003431 |
|  640 |  7.511645 | 0.006143 |
|  704 |  9.196446 | 0.009850 |
|  768 | 13.198684 | 0.005740 |
|  832 | 15.147677 | 0.019989 |
|  896 | 20.211919 | 0.013938 |
|  960 | 23.466168 | 0.089303 |
| 1024 | 36.042603 | 0.492844 |


Результаты параллельной версии программы:

| Dims | Avg       | Dev      |
|------|-----------|----------|
|  128 |  0.038594 | 0.000018 |
|  192 |  0.119908 | 0.000243 |
|  256 |  0.338809 | 0.001789 |
|  320 |  0.568833 | 0.004446 |
|  384 |  0.999728 | 0.002086 |
|  448 |  1.600199 | 0.008045 |
|  512 |  3.081107 | 0.022373 |
|  576 |  3.360849 | 0.039055 |
|  640 |  5.044900 | 0.049713 |
|  704 |  6.069065 | 0.046514 |
|  768 |  8.738168 | 0.089201 |
|  832 | 10.068957 | 0.048667 |
|  896 | 13.124639 | 0.134396 |
|  960 | 15.402067 | 0.054478 |
| 1024 | 23.981963 | 0.572130 |

Видно, что выбранный способ параллелизации способен ускорить вычисление формулы максимум в 4 раза (так как параллельно вычисляются 4 слагаемых). Фактически мы видим усорение даже меньше (в 1.5 раза) на выбранных размерностях. Это связано с накладными расходами на пересылку матриц между процессами, и при больших размерностях ускорение будет стремится к 4х.

### Использование OpenMP + MPI
```
Proc - количество потоков для OpenMP (аргумент для omp_set_num_threads(...))
AmdhalLimit - теоретический максимум ускорение
OpenMP - ускорение только с использованием OpenMP для умножения матриц
OpenMPAndMPI - ускорение с использованием одновроменно OpenMP и MPI
```

Так как с помощью MPI мы лишь распределяем работу по умножению различных матриц между различными прцоессами, можно также использовать OpenMP для параллелизации самого умножения матриц. Это позволяет достичь довольно интересного результата.

|------|-------------|----------|--------------|
| Proc | AmdhalLimit | OpenMP   | OpenMPAndMPI |
|------|-------------|----------|--------------|
|    1 |           1 | 0.877544 |     1.343544 |
|------|-------------|----------|--------------|
|    2 |           2 | 1.577031 |     2.432500 |
|------|-------------|----------|--------------|
|    3 |           3 | 2.255658 |     3.263160 |
|------|-------------|----------|--------------|
|    4 |           4 | 2.812781 |     3.938415 |
|------|-------------|----------|--------------|
|    5 |           5 | 3.378984 |     4.330193 |
|------|-------------|----------|--------------|
|    6 |           6 | 3.669126 |     4.718114 |
|------|-------------|----------|--------------|
|    7 |           7 | 4.043913 |     4.887995 |
|------|-------------|----------|--------------|
|    8 |           8 | 4.295161 |     5.071232 |
|------|-------------|----------|--------------|
|    9 |           9 | 4.801877 |     5.434607 |
|------|-------------|----------|--------------|
|   10 |          10 | 4.986025 |     5.589098 |
|------|-------------|----------|--------------|
|   11 |          11 | 5.201433 |     5.668449 |
|------|-------------|----------|--------------|
|   12 |          12 | 5.240946 |     5.648467 |
|------|-------------|----------|--------------|

Использование MPI позволило дополнительно ускорить вычисление заданной формулы примерно на 10%, что является довольно существенным результатом.


### Графическое сравнение

##### Среднее время выполнения параллельного MPI решения против обычного
![](./results/seq_vs_mpi.png)

##### Сравнение полученного ускорения против теоретического максимума по закону Амдала с использованием OpenMP и MPI
![](./results/amdhal_limit_mpi.png)

### Результаты

1. Как можно видеть по графикам, среднее время выполнения в параллельном режиме существенно ниже, чем в последовательном. Тем не менее, оба графика являются являются параболами, так как умножение матриц - кубический алгоритм.

2. Максимальное ускорение, которое позволил получить процессор с тестового стенда без использования компиляторных оптимизаций и с использованием OpenMP + MPI связки, оказалось равно 5.6. Теоретический максимум при этом равен 12. Максимально же ускорение которое удалось зафиксировать используя только MPI оказалось равно 1.5х, однако используя матрицы большей размерности оно могло достигнуть и 4х при выбранном способе параллелизации.

##### Вывод

Параллелизация вычисления предложенной формулы "по слагаемым" с помощью MPI позволяет существенно ускорить выполнение программы. Кроме того, совместное использование OpenMP и MPI может давать интересный эффект и ускорение будет больше, чем если использовать что-то одно.