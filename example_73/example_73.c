/* part of the code has been referred from pthreads_findmin.c by john leidel */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <pthread.h>

pthread_mutex_t minimum_value_lock;
int minimum_value;
long partial_list_size;
void *find_min(void *list_ptr);

static double get_time()
{
    struct timeval tp;
    struct timezone tzp;
    int i = 0;

    i = gettimeofday(&tp, &tzp);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

// Main function referred from pthreads_findmin.c by john leidel
int main(int argc, char **argv)
{
    int no_of_threads = 10;
    int ret = 0;
    int i = 0;
    long l = 0;
    long cur = 0;
    int seed = 10;
    long no_of_elements = 100;
    double start = 0.;
    double end = 0.;
    int *list = NULL;
    pthread_t *tids = NULL;
    void *res = NULL;

    /* parse the command line args */
    while ((ret = getopt(argc, argv, "S:T:N:h")) != -1)
    {
        switch (ret)
        {
        case 'h':
            printf("Usage : %s%s", argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
            return 0;
            break;
        case 'S':
            seed = atoi(optarg);
            break;
        case 'N':
            no_of_elements = atol(optarg);
            break;
        case 'T':
            no_of_threads = atoi(optarg);
            break;
        case '?':
        default:
            printf("Unknown option : %s%s",
                   argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
            return -1;
            break;
        }
    }

    /* sanity check */
    if (no_of_threads < 1)
    {
        printf("error : not enough threads\n");
        return -1;
    }
    if (no_of_elements <= (long)(no_of_threads))
    {
        printf("error : not enough elements\n");
        return -1;
    }
    minimum_value = INT_MAX; //Always retrurns -2147483648 since we try to find minimum value
    //minimum_value = INT_MAX; //can be used to get real postive values

    /* init the mutex */
    pthread_mutex_init(&minimum_value_lock, NULL);

    /* init lists, list_ptr, partial_list_size */
    list = malloc(sizeof(int) * no_of_elements);
    if (list == NULL)
    {
        printf("Error : could not init the list\n");
        return -1;
    }
    tids = malloc(sizeof(pthread_t) * no_of_threads);
    if (tids == NULL)
    {
        printf("Error : could not init the tids\n");
        return -1;
    }
    srand(seed);
    for (l = 0; l < no_of_elements; l++)
    {
        list[l] = (long)(rand());
    }

    if (no_of_threads == 1)
    {
        partial_list_size = no_of_elements;
    }
    else
    {
        partial_list_size = (no_of_elements / (long)(no_of_threads)) + (no_of_elements % (long)(no_of_threads));
    }

    start = get_time();
    /* create threads */
    for (i = 0; i < no_of_threads; i++)
    {
        if (pthread_create(&tids[i], NULL, &find_min, &list[cur]) != 0)
        {
            printf("Error : pthread_create failed on spawning thread %d\n", i);
            return -1;
        }
        cur += partial_list_size;

        if ((cur + partial_list_size) > no_of_elements)
        {
            cur = no_of_elements - partial_list_size;
        }
    }

    /* join threads */
    for (i = 0; i < no_of_threads; i++)
    {
        if (pthread_join(tids[i], &res) != 0)
        {
            printf("Error : pthread_join failed on joining thread %d\n", i);
            return -1;
        }
    }

    end = get_time();

    printf("Minimum value found: %d\n", minimum_value);
    printf("Runtime of %d threads = %f seconds\n", no_of_threads, (end - start));

    free(list);
    free(tids);
    tids = NULL;
    list = NULL;

    return 0;
}
void *find_min(void *list_ptr)
{
    int *partial_list_pointer, my_min, i;
    my_min = INT_MAX;
    partial_list_pointer = (int *)(list_ptr);

    //my_min = partial_list_pointer[0];
    for (i = 0; i < partial_list_size; i++)
    {
        if (partial_list_pointer[i] < my_min)
        {
            my_min = partial_list_pointer[i];
        }
    }

    /* lock and update the global copy */
    pthread_mutex_lock(&minimum_value_lock);

    if (my_min < minimum_value)
    {
        minimum_value = my_min;
    }
    pthread_mutex_unlock(&minimum_value_lock);

    pthread_exit(0);
}
