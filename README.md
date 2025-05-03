# Hash table / хэш-таблица

English version (may be outdated): [README_en.md](README_en.md)

Ассоциативный массив на основе [хэш-таблицы](https://ru.wikipedia.org/wiki/Хеш-таблица) со строками в качестве ключей.

Проект является лабораторной работой по оптимизации хэш-таблицы для x86_64 с использованием различных способов: написание функций на [ассемблере](source/crc32.s), использование inline ассемблера и интринсиков. 

## Что оптимизируем?

Сценарий использования хэш-таблицы является определяющим фактором для направления оптимизаций. Иногда необходимо быстро добавлять и удалять элементы, иногда искать, а может и всё сразу.

В данной работе было принято решение оптимизировать функцию поиска элемента в таблице по его ключу. При этом операция добавления элемента также заметно ускорится, ведь одна из её частей - проверка на то, что элемента нет в таблице, т.е. его поиск.  

Сценарий использования будет следующим: посчитать частоты только тех слов из файла 2, которые есть в файле 1. Предполагается, что файл 1 много меньше файла 2.

## Компиляция

```bash
    git clone -b v2 https://github.com/orientiered/HashTable.git
    cd HashTable
    make BUILD=RELEASE
```

Для генерации конфига `clangd`, используйте `make compile_commands`.

## Хэш-функции

Есть несколько встроенных хэш-функций (все они написаны для работы с C-строками):

+ `checksum` - сумма ascii кодов букв строки
+ `djb2` - простой и достаточно быстрый алгоритм хеширования
+ `crc32` - очень популярный алгоритм хеширования, за основу взята версия с таблицей предпосчитанных значений

Теоретически можно использовать свою хэш-функцию, указав её в `include/hashTable.h` в `#define _HASH_FUNC`.

При этом она должна иметь следующий вид:

+ `hash_t your_hash(const void *ptr);`
+ `ptr` - указатель на начало C-строки

### Анализ распределения хэш-функций

Для эффективной работы таблицы элементы дожны быть равномерно распределены по бакетам. Для оценки распределения есть функция `hashTableCalcDistribution`, которая считает дисперсию количества элементов в бакетах и строит в консоли график.

TODO: добавить тесты других хэш-функций и графики

+ crc32: дисперсия 4.03

## Как тестируем

Машина: Lenovo XiaoXin X16 Pro (2024)

CPU: AMD Ryzen 7 8845H w/ Radeon 780M Graphics 3.80 GHz (8 cores, 16 threads)

Компилятор: g++ 13.3.0

OS: Linux Mint 22.1 x86_64

### Тестовые файлы

Указанные в сценарии использования файлы 1 и 2 будут сгенерированы из [полного собрания сочинений Шекспира](https://www.gutenberg.org/cache/epub/100/pg100.txt). [Файл](shakespeare.txt)

1.`testString.txt`: текст разделяется на слова, всё остальное убирается. Слова расположены по одному на строке.

2.`testRequests.txt`: набор из 100 миллионов слов (порядка 500МБ), 90% взяты из `testString.txt`, остальные сгенерированы рандомно с длиной от 3 до 14 символов.

Эти файлы генерируются при помощи команды `make test_file`. Можно поменять количество тестов при помощи `TESTS`(100 миллионов по умолчанию) и часть слов, взятых из файла 1 - `FOUND_PERCENT` (0.9 по умолчанию).

### Измерение времени

Для измерения времени использовались ~~14 индусов с секундомерами~~ `clock_gettime` в режиме `CLOCK_PROCESS_CPUTIME_ID` и `_rdtsc`. `_rdtsc` возвращает текущее количество тактов процессора, обладает большей точностью и меньшими накладными расходами, но поскольку время теста порядка 5-10 секунд, особо смысла в ней нет: гораздо большую погрешность вносит операционная система.

Поскольку оптимизируется функция поиска, то наибольший интерес представляет время, проведенное в цикле подсчёта слов из файла 2. Время, затраченное на подготовку данных и загрузку слов из файла 1 в таблицу, не учитывается, так как это уже совсем другая задача.

### О погрешностях

Для оценки погрешности будем запускать тест 6 раз подряд. 
Первый результат отбрасывается, от остальных пяти берётся среднее, в качестве погрешности берём среднеквадратичное отклонение:

```math
    \sigma_t = \sqrt{\frac{N}{N-1}} * \sqrt{<t^2> - <t>^2}
```

(первый множитель - поправка, связанная с малым количеством измерений)

### Запуск теста

`make run`.

Программа пересобирается в релизную версию и запускается на 1 ядре при помощи команды `taskset 0x1`.

### Троттлинг

Его нет. Тест достаточно короткий (около 10 секунд), поэтому процессор не успевает нагреться выше 60 градусов.

### Важно: Load factor

Load factor - среднее количество элементов в бакете таблицы. Оптимальным значением является 0.75 - 1.5 (в java 0.75, в C# 1.0), но в учебных целях таблица строится с заведомо большим коэффициентом "заполненности": 15-17. Это сделано для того, чтобы было проще увидеть части, которые занимают много времени при исполнении. В конце будет проведено сравнение с правильной "настройкой" хэш-таблицы.

## Оптимизации

Структура первой версии:

+ Память для ключей и значений выделяется при помощи `calloc` (простая наивная реализация)
+ В бакетах элементы хранятся в виде списка (**не массив**), ноды выделяются при помощи `calloc`
+ О длинах строк ничего не известно

### Тесты первой версии

(Odefault - компиляция без ключей оптимизации)

| Версия   | Время, с       | Время одного запроса, тактов |
|-------   |----            |-----                         |
| Odefault | 16.60 +- 0.31  | 630 +- 12                    |
| -O3      | 15.78 +- 0.34  | 599 +- 13                    |

Для профилирования программы будем использовать утилиту `perf` и программу `hotspot` для анализа полученного профиля.

Иногда будут встречаться т.н. `flamegraph`ы - визуализация стека вызовов, отражающая время исполнения каждой функции.

Для запуска профилирования есть команда `make perfTest` - она компилирует релизную версию с флагом `-fno-omit-frame-pointer`, который делает обязательным сохранение rbp в функциях (без этого perf может неправильно интерпретировать stack trace и пропускать названия функций). 

Затем запускает профилирование c флагами `-g` (записать трейс стека) и `--call-graph dwarf`. Частота выборок по умолчанию 10000 Гц, но её можно поменять при помощи опции `FREQ`. **Примечение**: реальная частота снэпшотов perf может отличаться.

#### Профиль 1


![zero_opt](docs/hotspot_0.png)

![no_opt](docs/flame_naive.svg)

Most part of the time takes strcmp, search function and crc32.

We will start optimizations with strcmp. Most of the words are shorter than 16 letters, so we can use SIMD instructions to compare short strings faster. In order to find short strings faster, i will add field with length of the string.

### Strcmp optimization

Changes:

+ `aligned_alloc` instead of `calloc` to ensure correct alignment for SIMD
+ When you know lengths of the strings, you could first check them on equality. This optimization is more algorithimic but it will be stupid not to implement it. This optimization is enabled with `#define CMP_LEN_FIRST`.
+ New hashTableFind algorithm:
    Length of the key is calculated first. If it is more than `SMALL_STR_LEN` (16 chars), than previous implementation is used. This optimization can be enabled with `#define FAST_STRCMP` Core of new implementation can be seen below:

    ```c
    if (keyLen >= SMALL_STR_LEN) {
        // Key doesn't fit in SIMD register
        while (node) {
            if (CMP_LEN_OPT(node->len == keyLen &&) strcmp(node->key, key) == 0 )
                return node;

            node = node->next;
        }

    } else {
        // Creating local aligned array of chars for key
        alignas(KEY_ALIGNMENT) char keyCopy[SMALL_STR_LEN] = "";
        // Copying key to it
        memcpy(keyCopy, key, keyLen+1);
        // Loading key to SIMD register
        MMi_t searchKey = _MM_LOAD((MMi_t *) keyCopy);

        while (node) {
            if (CMP_LEN_OPT(node->len == keyLen &&) 
                //! Alignment of key is guaranteed by aligned_calloc with KEY_ALIGNMENT
                fastStrcmp(searchKey, (MMi_t *) node->key) == 0)
                return node;

            node = node->next;

        }
    }
    ```

    Where `fastStrcmp` is

    ```c
    typedef __m128i MMi_t;
    #define _MM_LOAD(ptr) _mm_load_si128(ptr)
    #define _MM_CMP_MOVEMASK(a, b) _mm_movemask_epi8(_mm_cmpeq_epi8(a, b))
    static const uint32_t _MM_MASK_CONSTANT = 0xFFFF;
    static int fastStrcmp(MMi_t a, MMi_t *bptr) {
        MMi_t b = _MM_LOAD(bptr);
        //k-th bit of cmpMask = (a[k] == b[k])
        uint32_t cmpMask = (uint32_t) _MM_CMP_MOVEMASK(a, b); 
        //_MM_MASK_CONSTANT is 0xFFFF for SSE and 0xFFFFFFFFF for AVX2
        return (int) (cmpMask ^ _MM_MASK_CONSTANT); 
    }
    ```

### Only fastStrcmp, without `CMP_LEN_FIRST'

Execution time: 11.0 seconds, approx. 42 * 10^9 clock cycles.
![fastcmp_opt](docs/hotspot_faststrcmp.png)
![faststrcmp](docs/flame_faststrcmp.png)

Now strcmp takes approx. 30% of computing time - great improvement.

### Comparing length of the string first, without `FAST_STRCMP`

Execution time: 10.4 seconds, 39.5 * 10^9 clock cycles.
![cmplen_hotspot](docs/hotspot_cmpLenFirst.png)
![cmplen_flame](docs/flame_cmpLenFirst.png)

In that case algorithimic optimization yields better result, is not hardware specific and can be implemented easier.

### Both strcmp optimizations (`CMP_LEN_FIRST` + `FAST_STRCMP`)

Execution time: 9.5 seconds, 36 * 10^9 clock cycles.
![bothcmp_opt](docs/hotspot_BothStrcmp.png)
![bothcmp_opt](docs/flame_BothStrcmp.png)

These optimizations work well together.

### Crc32 written in asm

`CRC32` takes almost 15% of computing time. This hashing algorithm is so widely used, that CPU's have dedicated instruction to calculate it: `crc32`.

```asm
;========================================================
; Crc32 hashing algorithm for C strings
; Args:
;   rdi - memory address
; Ret:
;   rax - crc32 hash
;========================================================
fastCrc32u:
    xor  rax, rax
    dec  rax        ; rax = all ones
    .hash_loop:
        mov   sil, BYTE [rdi]
        inc   rdi
        crc32 rax, sil
        test  sil, sil
        jnz   .hash_loop

    ret
```

Execution time: 9.2 seconds, 34.9 * 10^9 clock cycles

![crc32u_hotspot](docs/hotspot_crc32u.png)

![crc32u_flame](docs/flame_crc32u.png)

Program now works faster, but hashing function takes more cycles. Probable explanation: `crc32` uses different polynom, which has better distribution. On my test file dispersion of elements in bucket decreased from 4.03 to 3.93.
