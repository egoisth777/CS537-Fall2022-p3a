#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>

struct map {
    int *keys;
    int **followings;
    int length;
};

/**
 * Print the content of the map, used for debugging
*/
void 
printMap(struct map * myMap) {

    printf("psort.map(\n");
    int length = myMap->length;
    printf("---length %d---,\n", length);
    for (int i = 0; i < length; i ++)
    {
        int* followings = myMap->followings[i];
        printf("%dth key: %d, following[0], [1], [2]: %d, %d, %d,\n", i, myMap->keys[i], followings[0], followings[1], followings[2]);
    }
    printf(")\n");
}

void 
freeMap(struct map * myMap) {
    int length = myMap->length;
    free(myMap->keys);
    for(int i = 0; i < length; i ++)
        free(myMap->followings[i]);
    free(myMap->followings);
}

/**
 * Print the error message
*/
void 
printErrMsg(char* msg) {
    printf("panic! There is an error : %s\n", msg);
    exit(20);
}

/**
 * Helper
 * @return number of processers available
*/
int 
num_processor(void) {
    return (get_nprocs());
}

struct map 
readin(const char* filename)
{
    int file = open(filename, O_RDONLY);
    struct stat buffer;
    fstat(file, &buffer);
    int *ptr = mmap(NULL, buffer.st_size, PROT_READ, MAP_SHARED, file, 0);
    // printf("sizeof char: %d \n", sizeof(char)); // should be 1
    close(file);
    // printf("step 1\n");
    int* record_keys = malloc((buffer.st_size / 100 + 1) * sizeof(int)); // each 4-byte integer
    int** record_followings = malloc((buffer.st_size / 100 + 1) * sizeof(int*)); // each 96-byte followings
    for (int i = 0; ptr[i] != '\0'; i++)
    {
        if (i % 25 == 0) {
            // DONE: get the key (4-byte int)
            // printf("key %d is %d\n", i, ptr[i]);
            int key = ptr[i];
            record_keys[i / 25] = key;
            continue;
        }
        if (i % 25 == 1) {
            // DONE: get the following (96-byte data)
            int* following = malloc(24 * sizeof(int));
            for (int j = 0; j < 96/sizeof(int); j ++) {
                following[j] = ptr[i + j];
            }
            record_followings[i / 25] = following;
            i += 23;
            continue;
        }
        printErrMsg("你不可能达到这里，除非文件有问题或者我有问题！");
        // memcpy(data, ptr, 4);
    }
    struct map myMap;
    myMap.length = buffer.st_size / 100;
    myMap.keys = record_keys;
    myMap.followings = record_followings;
    munmap(ptr, buffer.st_size);
    // printf("step 2\n");
    return myMap;
}

void writeOut(const char* filename, struct map *myMap) {
    FILE *fp;
    fp = fopen(filename, "w");
    int file = fileno(fp);
    int length = myMap->length;
    int copy[length * 25];
    for (int i = 0; i < length * 25; i ++)
    {
        if (i % 25 == 0)
        {
            copy[i] = myMap->keys[i / 25];
            continue;
        }
        if (i % 25 == 1)
        {
            for(int j = 0; j < 24; j ++)
                copy[i + j] = myMap->followings[i / 25][j];
            i += 23;
        }
    }
    fwrite(&copy, 4, length * 25, fp);
    fflush(fp);
    fclose(fp);
    /*
    lseek(file, length * 100, SEEK_END);
    write(file, "", 1);
    char *ptr = mmap(NULL, length * 100 + 1, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    
    memcpy(ptr, copy, length * 100);
    msync(ptr, length * 100, MS_SYNC);
    fsync(file);
    munmap(ptr, length * 100 );
    */
    
}

// TODO: Divide and Conquer method in parallelism

void merge(struct map * myMap, int left, int mid, int right) {
    int left_current = left;
    int right_current = mid + 1;

    int temp[right - left + 1];
    int *temp_following[right - left + 1];
    int current_index = 0;

    // 神的代碼
    while(left_current <= mid && right_current <= right) {
        //printf("myMap->keys[left_current]: %d\n", myMap->keys[left_current]);
        //printf("myMap->keys[right_current]: %d\n", myMap->keys[right_current]);

        temp[current_index] = myMap->keys[left_current] < myMap->keys[right_current] ? myMap->keys[left_current] : myMap->keys[right_current];
        temp_following[current_index++] = myMap->keys[left_current] < myMap->keys[right_current] ? myMap->followings[left_current++] : myMap->followings[right_current++];
    }

    while(left_current <= mid) {
        temp[current_index] = myMap->keys[left_current];
        temp_following[current_index++] = myMap->followings[left_current++];
    }

    while(right_current <= right) {
        temp[current_index] = myMap->keys[right_current];
        temp_following[current_index++] = myMap->followings[right_current++];
    }
    int copyIndex = 0;
    while (left <= right)
    {
        //printf("temp[copyIndex] %d\n", temp[copyIndex]);
        myMap->keys[left] = temp[copyIndex];
        //printf("myMap->keys[left]: %d\n", myMap->keys[left]);
        myMap->followings[left++] = temp_following[copyIndex++];
    }
}

void mergeDivide(struct map * myMap, int left, int right) {
    if (left >= right)
        return ;
    int mid = (left + right) / 2;
    mergeDivide(myMap, left, mid);
    mergeDivide(myMap, mid + 1, right);
    merge(myMap, left, mid, right);
    //printf("after merge: map: \n");
    //printMap(myMap);
}

void mergeSort(struct map * myMap) {
    int length = myMap->length;
    mergeDivide(myMap, 0, length - 1);
}

int main(int argc, char const *argv[])
{
    // const char* filename = argv[1];
    const char* filename = "output.bin";
    const char* output = "output2.bin";
    struct map myMap = readin(filename);
    printMap(&myMap);
    printf("之后\n");
    mergeSort(&myMap);
    printMap(&myMap);
    writeOut(output, &myMap);
    freeMap(&myMap);
    printf("终末\n");
    struct map myMap2 = readin(output);
    printf("再读：\n");
    printMap(&myMap2);
    return 0;
}
