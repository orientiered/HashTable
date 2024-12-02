#include <stdio.h>
#include <stdlib.h>

#include <hashTable.h>

int main() {
    hashTable_t table = hashTableCtor(10);
    char buffer[128] = "";
    char buffer2[128] = "";
    int mode = 0;
    while(1) {
        scanf(" %c %s", &mode, buffer);
        if (mode == 'w') {
            scanf(" %s", buffer2);
            hashTableInsert(&table, buffer, buffer2, 128);
        } else if (mode == 'r') {
            hashTableRemove(&table, buffer);
        } else if (mode == 'f') {
            void *elem = hashTableFind(&table, buffer);
            if (elem)
                printf("%s: %s\n", buffer, elem);
            else
                printf("%s: null\n", buffer);
        } else {
            break;
        }
        fflush(stdout);
        fflush(stdin);
    }
    hashTableDtor(&table);
    return 0;
}

void example1(){
    hashTable_t table = hashTableCtor(3);

    const char *str = "hello world";
    double d = 10.5;
    int i = 4;
    hashTableInsert(&table, "str", str, 12);
    hashTableInsert(&table, "double", &d, 8);
    hashTableInsert(&table, "int", &i, 4);
    hashTableDump(&table);

    hashTableInsert(&table, "str2", str, 12);
    hashTableInsert(&table, "double2", &d, 8);
    hashTableInsert(&table, "int2", &i, 4);
    hashTableDump(&table);


    printf("str: %s\n", hashTableFind(&table, "str"));
    printf("double: %g\n", *(double*)hashTableFind(&table, "double"));
    printf("int: %d\n", *(int *)hashTableFind(&table, "int"));

    hashTableDump(&table);

    hashTableRemove(&table, "str");
    hashTableDump(&table);
    hashTableDtor(&table);
}
