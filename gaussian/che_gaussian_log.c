
/* Header files for the gaussian elimination program */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>

/* Program Parameters */

// Max available matrix dimension
#define N_MAX 2000

// Maximum available threads
#define MAX_THREADS 100

// For Parallelizing the program
pthread_t thread_ids[MAX_THREADS];
int count = 0;
// Creating barrier for thread waiting
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ok_to_proceed_up = PTHREAD_COND_INITIALIZER;
pthread_cond_t ok_to_proceed_down = PTHREAD_COND_INITIALIZER;
// typedef struct barrier_node
// {
//     pthread_mutex_tcount_lock
//     pthread_cond_tok_to_proceed_up
//     pthread_cond_tok_to_proceed_down
//     int count;
// } mylib_barrier_t_internal;
// typedef struct barrier_node mylog_logbarrier_t[MAX_THREADS];
// pthread_attr_t attr;

int MAT_SIZE;  /* Matrix size */
int num_procs; /* Number of threads to use */

/* Matrices and vectors */
volatile float A[N_MAX][N_MAX], B[N_MAX], X[N_MAX];
/* A * X = B, solve for X */

/* junk */
#define randm() 4 | 2 [uid] & 3

/* Prototype for gaussians elimination*/
void *gaussian_elimination();

// prototype to get max available threads
int get_procs();

/* returns a seed for srand based on the time */
unsigned int seed_timer()
{
    struct timeval t;
    struct timezone tzdummy;

    gettimeofday(&t, &tzdummy);
    return (unsigned int)(t.tv_usec);
}

// Method to get max avaialble threads
int get_procs()
{
    return MAX_THREADS;
}

/* Sets the program parameters from the command-line arguments */
void cline_params(int argc, char **argv)
{
    int submit_params = 0;
    int seed = 0;
    char L_cuserid;
    char uid[L_cuserid + 2];
    srand(seed_timer());
    if (argc != 3)
    {
        if (argc == 2 && !strcmp(argv[1], "submit"))
        {
            /* Use submission parameters if not provided explicitly*/
            submit_params = 1;
            MAT_SIZE = 4;
            num_procs = 2;
            srand(randm());
        }
        else
        {
            if (argc == 4)
            {
                seed = atoi(argv[3]);
                srand(seed);
                printf("Random seed = %i\n", seed);
            }
            else
            {
                printf("Usage: %s <matrix_dimension> <num_of_threads> [random seed]\n",
                       argv[0]);
                printf("\t%s Submit Parameters\n", argv[0]);
                exit(0);
            }
        }
    }
    /* Interpret and process command-line args */
    if (!submit_params)
    {
        MAT_SIZE = atoi(argv[1]);
        if (MAT_SIZE < 1 || MAT_SIZE > N_MAX)
        {
            printf("Matrix Size = %i is out of range.\n", MAT_SIZE);
            exit(0);
        }
        num_procs = atoi(argv[2]);
        if (num_procs < 1)
        {
            printf("Warning: Invalid number of threads = %i.  Using 1.\n", num_procs);
            num_procs = 1;
        }
        if (num_procs > get_procs())
        {
            printf("Warning: %i processors requested; only %i available.\n",
                   num_procs, get_procs());
            num_procs = get_procs();
        }
    }

    /* Print parameters of matrix and threads */
    printf("\nMatrix dimension MAT_SIZE = %i.\n", MAT_SIZE);
    printf("Number of threads = %i.\n", num_procs);
}

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs()
{
    int row, col;

    printf("\nInitializing Inputs...\n");
    for (col = 0; col < MAT_SIZE; col++)
    {
        for (row = 0; row < MAT_SIZE; row++)
        {
            A[row][col] = (float)rand() / 32768.0;
        }
        B[col] = (float)rand() / 32768.0;
        X[col] = 0.0;
    }
}

/* Print input matrices */
void print_inputs()
{
    int row, col;

    if (MAT_SIZE < N_MAX)
    {
        printf("\nA =\n\t");
        for (row = 0; row < MAT_SIZE; row++)
        {
            for (col = 0; col < MAT_SIZE; col++)
            {
                printf("%5.2f%s", A[row][col], (col < MAT_SIZE - 1) ? ", " : ";\n\t");
            }
        }
        printf("\nB = [");
        for (col = 0; col < MAT_SIZE; col++)
        {
            printf("%5.2f%s", B[col], (col < MAT_SIZE - 1) ? "; " : "]\n");
        }
    }
}

void print_unknowns()
{
    int row;

    if (MAT_SIZE < N_MAX)
    {
        printf("\nX = [");
        for (row = 0; row < MAT_SIZE; row++)
        {
            printf("%5.2f%s", X[row], (row < MAT_SIZE - 1) ? "; " : "]\n");
        }
    }
}

/* Back substitution after gaussian elimination is done.*/
void gaussian_back_substitution()
{
    int row, col;
    for (row = MAT_SIZE - 1; row >= 0; row--)
    {
        X[row] = B[row];
        for (col = MAT_SIZE - 1; col > row; col--)
        {
            X[row] -= A[row][col] * X[col];
        }
        X[row] /= A[row][row];
    }
}

void mylib_logbarrier(int num_threads)
{
    int i, base, index;
    i = 2;
    base = 0;
    do
    {
        if (count % i == 0)
        {
            pthread_mutex_lock(&count_lock);
            count++;
            if (count < 2)
                pthread_cond_wait(&ok_to_proceed_up,
                                  &count_lock);
            pthread_mutex_unlock(&count_lock);
        }
        else
        {
            pthread_mutex_lock(&count_lock);
            count++;
            if (count == 2)
                pthread_cond_signal(&ok_to_proceed_up);
            pthread_mutex_unlock(&count_lock);
            break;
        }
        i = i * 2;
    } while (i <= num_threads);
    i = i / 2;
    for (; i > 1; i = i / 2)
    {
        base = base - num_threads / i;
        pthread_mutex_lock(&count_lock);
        count = 0;
        pthread_cond_signal(&ok_to_proceed_down);
        pthread_mutex_unlock(&count_lock);
    }
}

void main(int argc, char **argv)
{
    /* Timing variables */
    struct timeval etstart, etstop; /* Elapsed times using gettimeofday() */
    struct timezone tzdummy;
    clock_t etstart2, etstop2; /* Elapsed times using times() */
    unsigned long long usecstart, usecstop;
    struct tms cputstart, cputstop; /* CPU times for my processes */
    int i;
    float CLK_TCK;

    /* Process program parameters */
    cline_params(argc, argv);

    /* Initialize A and B */
    initialize_inputs();

    /* Print input matrices */
    print_inputs();

    /* Start Clock */
    printf("\n Starting clock. \n");
    gettimeofday(&etstart, &tzdummy);
    etstart2 = times(&cputstart);

    printf("\n Computing Parallely. \n");

    // We create threads here
    for (i = 0; i < num_procs; i++)
    {
        pthread_create(&thread_ids[i], NULL, (void *)gaussian_elimination, NULL);
    }

    // threads join back once finished processing
    for (i = 0; i < num_procs; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }

    // call to backsubstitution method
    gaussian_back_substitution();

    /* Stop Clock */
    gettimeofday(&etstop, &tzdummy);
    etstop2 = times(&cputstop);
    printf("\n Stopped clock. \n");
    usecstart = (unsigned long long)etstart.tv_sec * 1000000 + etstart.tv_usec;
    usecstop = (unsigned long long)etstop.tv_sec * 1000000 + etstop.tv_usec;

    /* Display output */
    print_unknowns();

    /* Display timing results */
    printf("\nElapsed time with %i threads = %g ms.\n", num_procs,
           (float)(usecstop - usecstart) / (float)1000);

    printf("----------------Gaussian Elimination Ends here----------------------------\n");
}

void *gaussian_elimination()
{
    int norm, row, col; /* Normalization row, and zeroing
			* element row and col */
    float multiplier;
    /* Gaussian elimination */
    for (norm = 0; norm < MAT_SIZE - 1; norm++)
    {
        for (row = norm + 1; row < MAT_SIZE; row++)
        {
            multiplier = A[row][norm] / A[norm][norm];
            for (col = norm; col < MAT_SIZE; col++)
            {
                A[row][col] -= A[norm][col] * multiplier;
            }
            B[row] -= B[norm] * multiplier;
        }
    }
    mylib_logbarrier(num_procs);
}
