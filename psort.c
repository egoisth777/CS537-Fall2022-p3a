#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <pthread.h>


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
    int num_level_thread; // total number of threads of current level
};

struct map myMap; // global myMap variable
int current_finished_threads;
// mutex: wait for current level child threads
pthread_cond_t condition_wait = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

// Divide and Conquer method in parallelism

void 
merge(struct map * myMap, int left, int mid, int right, int num_level_threads) {
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
    pthread_mutex_lock(&lock);
    current_finished_threads++;
    if (current_finished_threads >= num_level_threads)
        pthread_cond_signal(&condition_wait);
    pthread_mutex_unlock(&lock);
}

void 
merge2(struct map * myMap, int left, int mid, int right) {
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
    merge2(myMap, left, mid, right);
    //printf("after merge: map: \n");
    //printMap(myMap);
}

void*
thread_merge_sort(void* arg){
    // determine parts of the array
    struct arrsects * arg_sect = (struct arrsects *)arg;
    int left = arg_sect->l;
    int right = arg_sect->h;
    // believe in mr. ye
    // mergeDivide(&myMap, left, right);
    merge(&myMap, left, (left + right) / 2, right, arg_sect->num_level_thread);
}

void*
thread_merge_divide(void* arg){
    // determine parts of the array
    struct arrsects * arg_sect = (struct arrsects *)arg;
    int left = arg_sect->l;
    int right = arg_sect->h;
    // believe in mr. ye
    mergeDivide(&myMap, left, right);
    pthread_mutex_lock(&lock);
    current_finished_threads++;
    // printf("current finished num:%d\n", current_finished_threads);
    if (current_finished_threads >= arg_sect->num_level_thread)
        pthread_cond_signal(&condition_wait);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void mt_thread_sort() {
    // concurrent speed-up
    const int processor_no = get_nprocs();
    const int thread_no = processor_no > myMap.length ? myMap.length : processor_no; 
    int epthread = myMap.length/thread_no;           // elements per thread
    int initrun = 1;

    
    // radius size -> begin with epthread, * 2 each time, eventually come to myMap.length
    // imagine first radius = 6 (length = 20, thread_no = 3)
    int radius = epthread;

    // @TODO: merge the consequtive four pieces of arrays 
    while(radius < myMap.length) { // merge (thread_no - 1) times
        // int last_round = radius * 2 >= myMap.length ? 1 : 0;
        int num_pieces = myMap.length / radius;
        int num_threads = num_pieces > myMap.length ? myMap.length : num_pieces;
        // int last_alone = num_threads % 2 == 1;
        pthread_t threads[thread_no];                       // thread lists
        struct arrsects args_arr[thread_no];             //arrsection arg wrapper
        current_finished_threads = 0; // init current level

        for(int i = 0; i < num_threads; i++) {
                args_arr[i].section = i;
                args_arr[i].l = i * radius;
                args_arr[i].num_level_thread = num_threads;
                if (i == num_threads - 1)
                    args_arr[i].h = myMap.length - 1;
                else
                    args_arr[i].h = (i + 1) * radius - 1;
                int rc;
                if (initrun == 1)
                    rc = pthread_create(&threads[i], NULL, thread_merge_divide, (void*)(&args_arr[i]));
                else
                    rc = pthread_create(&threads[i], NULL, thread_merge_sort, (void*)(&args_arr[i]));
                if(rc){
                    printErrMsg("Error while creating threads");
                    exit(1);
                }
        }
        pthread_mutex_lock(&lock);
        while (current_finished_threads < num_threads)
            pthread_cond_wait(&condition_wait, &lock);
        pthread_mutex_unlock(&lock);
        radius = radius * 2;
        // printMap(&myMap);
    }
}

int 
main(int argc, char const *argv[])
{
    // const char* filename = argv[1];
    const char* filename = "output_extreme.bin";
    const char* output = "output2.bin";
    
    //initialize mymap
    readin(filename);
    printMap(&myMap);
    printf("之后\n");
    mt_thread_sort();
    printMap(&myMap);
    //writeOut(output, &myMap);
    // freeMap(&myMap);
    // printf("终末\n");
    // struct map myMap2 = readin(output);
    // printf("再读：\n");
    // printMap(&myMap2);
    return 0;
}
