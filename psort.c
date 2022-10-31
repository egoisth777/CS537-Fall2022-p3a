#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>


#define RC_LEN 100 // length of each records

// struct used to denote the map
struct map {
    int *keys;
    int **followings;
    int length;
};

// strcture passed into the thread function
struct arrsects{
    int section; // which section of the arrary it is 
    int l; // low index
    int h; // high index
};

struct map myMap; // global myMap variable

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
 * Helper
 * Print the error message
*/
void 
printErrMsg(char* msg) {
    printf("%s\n", msg);
    exit(1);
}

void 
readin(const char* filename)
{
    // open the file
    int fd = open(filename, O_RDONLY);
    if(fd == -1){ // safety reasons
        printf("open() syscall failed \n");
        exit(1);
    }
    
    // map file to memory, and read
    struct stat buffer;
    fstat(fd, &buffer);
    int size = buffer.st_size;
    int *ptr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    
    // read the file
    int rc_no = size/RC_LEN + 1;
    int* keys_arr = malloc(rc_no * sizeof(int)); // each 4-byte integer
    int** following_darr = malloc(rc_no * sizeof(int*)); // each 96-byte followings
    
    for (int i = 0; ptr[i] != '\0'; i++)
    {
        if (i % 25 == 0) {
            // DONE: get the key (4-byte int)
            // printf("key %d is %d\n", i, ptr[i]);
            int key = ptr[i];
            keys_arr[i / 25] = key;
            continue;
        }
        if (i % 25 == 1) {
            // DONE: get the following (96-byte data)
            int* following = malloc(24 * sizeof(int));
            for (int j = 0; j < 96/sizeof(int); j ++) {
                following[j] = ptr[i + j];
            }
            following_darr[i / 25] = following;
            i += 23;
            continue;
        }
        printErrMsg("Unexpected readin issue");
    }
      
    myMap.length = size / 100;
    myMap.keys = keys_arr;
    myMap.followings = following_darr;
    munmap(ptr, size);
    // printf("step 2\n");
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
}

// TODO: Divide and Conquer method in parallelism

void 
merge(struct map * myMap, int left, int mid, int right) {
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

void 
mergeDivide(struct map * myMap, int left, int right) {
    if (left >= right)
        return ;
    int mid = (left + right) / 2;
    mergeDivide(myMap, left, mid);
    mergeDivide(myMap, mid + 1, right);
    merge(myMap, left, mid, right);
    //printf("after merge: map: \n");
    //printMap(myMap);
}

// void 
// mergeSort(struct map * myMap) {
//     int length = myMap->length;
//     mergeDivide(myMap, 0, length - 1);
// }

void*
thread_merge_sort(void* arg){
    // determine parts of the array
    struct arrsects * arg_sect = (struct arrsects *)arg;
    int left = arg_sect->l;
    int right = arg_sect->h;
    // believe in mr. ye
    mergeDivide(&myMap, left, right);
}


int 
main(int argc, char const *argv[])
{
    // const char* filename = argv[1];
    const char* filename = "output.bin";
    const char* output = "output2.bin";
    
    //initialize mymap
    readin(filename);
    
    
    
    // concurrent speed-up
    const int processor_no = get_nprocs();
    const int thread_no = processor_no > myMap.length ? myMap.length : processor_no; 
    pthread_t threads[thread_no];                       // thread lists
    struct arrsects args_arr[thread_no];             //arrsection arg wrapper
    int epthread = myMap.length/thread_no;           // elements per thread
    
    // create the threads
    for(int i = 0; i < thread_no; i++){
        args_arr[i].section = i;
        args_arr[i].l = i * epthread;
        args_arr[i].h = (i + 1) * epthread - 1;
    
        int rc = pthread_create(&threads[i], NULL, thread_merge_sort, (void*)(&args_arr[i]));
        if(rc){
            printErrMsg("Error while creating threads");
            exit(1);
        }
    }
    
    // join the threads
    for(int i = 0; i < thread_no; i++){
        p_thread_join(threads[i], NULL);
    }
    
    // merge the consequtive four pieces of arrays 
    for(int i = 0; i < thread_no - 1; i++){ // merge (thread_no - 1) times
        
    }
    



    // printMap(&myMap);
    // printf("之后\n");
    // mergeSort(&myMap);
    // printMap(&myMap);
    // writeOut(output, &myMap);
    // freeMap(&myMap);
    // printf("终末\n");
    // struct map myMap2 = readin(output);
    // printf("再读：\n");
    // printMap(&myMap2);
    return 0;
}
