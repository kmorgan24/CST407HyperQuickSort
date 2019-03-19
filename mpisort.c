//****************************************************
// MPI Parallel sort
//
// Author: Phil Howard

#ifndef _POSIC_C_SOURCE
#define _POSIX_C_SOURCE 200809
#endif

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "usec.h"

static int g_data_size = 1000000;
static int g_print_values = 0;

//***********************************************
static void parse_args(int argc, char **argv)
{
    int opt;

    while ( (opt = getopt(argc, argv, "hs:p:") ) != -1)
    {
        switch (opt)
        {
            case 'h':
                printf("%s\n"
                       "-h print this help message and exit\n"
                       "-s <size of dataset>\n"
                       "-p <print level> amount of info to print\n"
                       ,argv[0]);
                exit(1);
            case 's':
                // size
                g_data_size = atoi(optarg);
                break;
            case 'p':
                // print values
                g_print_values = atoi(optarg);
        }
    }
}

//***********************************************
static void print_data(int mask, char *msg, int *data, int size)
{
    if (g_print_values & mask)
    {
        int rank;

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        for (int ii=0; ii<size; ii++)
        {
            printf("%s: %2d %8d: %12d\n", msg, rank, ii, data[ii]);
        }
    }
}

//***********************************************
int compare(const void *v1, const void *v2)
{
    return *(int *)v1 - *(int *)v2;
}

//***********************************************
int main(int argc, char **argv)
{
    int *data = NULL;

    parse_args(argc, argv);

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Create the data
    if (world_rank == 0)
    {
        data = (int *)malloc(sizeof(int) * g_data_size);
        if (data == NULL)
        {
            printf("Insufficient memory\n");
            return 1;
        }
        unsigned int seed = (unsigned int)time(NULL);
        for (int ii=0; ii<g_data_size; ii++)
        {
            data[ii] = rand_r(&seed);
        }

        print_data(0x0002, "Original data", data, g_data_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    int64_t start_time = usecs();

    // ********************************************
    // You must hyperize the following line of code
    if (world_rank == 0) qsort(data, g_data_size, sizeof(int), compare);
    // ********************************************

    int64_t end_time = usecs();

    if (world_rank == 0)
    {
        double time = (end_time - start_time)/1000000.0;
        printf("Sorted %d values in %f seconds\n", g_data_size, time);

        print_data(0x0001, "sorted data", data, g_data_size);
    }

    MPI_Finalize();

    return 0;
}

