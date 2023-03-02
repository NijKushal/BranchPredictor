#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sim_bp.h"
#include <iostream>
#include <bits/stdc++.h>
using namespace std;


int getPcValueForBimodal(unsigned int addr, int m)
{
    addr = addr >> 2;
    addr = addr << (32 - m);
    return addr >> (32 - m);
}

int getPcValueForGshare(unsigned int addr, int m, int n)
{
    unsigned int bhr_temp;
    addr = addr >> 2;
    bhr_temp = bhr << (m - n);
    addr = addr ^ bhr_temp;
    addr = addr << (32 - m);
    addr = addr >> (32 - m);
    return addr;
}

void updateBHR(char *outcome, int n)
{
    bhr = bhr >> 1;
    int result;
    unsigned int temp = 0xffffffff;
    if (*outcome == 't')
    {
        temp = temp << (n - 1);
        bhr = bhr | temp;
        bhr = bhr << (32 - n);
        bhr = bhr >> (32 - n);
    }
    else if (*outcome == 'n')
    {
        temp = temp >> (33 - n);
        bhr = bhr & temp;
    }
}

int performBranchTraining(int counter_value, char *outcome)
{
    if (*outcome == 't')
    {
        if (counter_value < 2)
        {
            num_of_mispredictions++;
        }
        if (counter_value < 3)
        {
            counter_value = counter_value + 1;
        }
    }
    else if (*outcome == 'n')
    {
        if (counter_value >= 2)
        {
            num_of_mispredictions++;
        }
        if (counter_value > 0)
        {
            counter_value = counter_value - 1;
        }
    }
    return counter_value;
}

int main(int argc, char *argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    int pc_value_bimodal;
    int pc_value_gshare;
    int pc_value_hybrid;
    char str[2];
    int x;
    int size;

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc - 1);
        exit(EXIT_FAILURE);
    }

    params.bp_name = argv[1];

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if (strcmp(params.bp_name, "bimodal") == 0) // Bimodal
    {
        if (argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.M2 = strtoul(argv[2], NULL, 10);
        trace_file = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if (strcmp(params.bp_name, "gshare") == 0) // Gshare
    {
        if (argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.M1 = strtoul(argv[2], NULL, 10);
        params.N = strtoul(argv[3], NULL, 10);
        trace_file = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);
    }
    else if (strcmp(params.bp_name, "hybrid") == 0) // Hybrid
    {
        if (argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.K = strtoul(argv[2], NULL, 10);
        params.M1 = strtoul(argv[3], NULL, 10);
        params.N = strtoul(argv[4], NULL, 10);
        params.M2 = strtoul(argv[5], NULL, 10);
        trace_file = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);
    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }

    FP = fopen(trace_file, "r");
    if (FP == NULL)
    {
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    int k =0, m1 = 0, m2 = 0;

    if(strcmp(params.bp_name, "bimodal") == 0){
        m2 = pow(2, params.M2);
    }
    if(strcmp(params.bp_name, "gshare") == 0){
        m1 = pow(2, params.M1);
    }
    if(strcmp(params.bp_name, "hybrid") == 0){
        m1 = pow(2, params.M1);
        k = pow(2, params.K);
        m2 = pow(2, params.M2);
    }

    int *array_of_counters_bimodal = new int[m2];
    for (x = 0; x < m2; x++)
    {
        array_of_counters_bimodal[x] = 2;
    }

    int *array_of_counters_gshare = new int[m1];
    for (x = 0; x < m1; x++)
    {
        array_of_counters_gshare[x] = 2;
    }

    int *array_of_counters_hybrid = new int[k];
    for (x = 0; x < k; x++)
    {
        array_of_counters_hybrid[x] = 1;
    }

    while (fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        num_of_predictions++;
        if (strcmp(params.bp_name, "bimodal") == 0)
        {
            pc_value_bimodal = getPcValueForBimodal(addr, params.M2);
            array_of_counters_bimodal[pc_value_bimodal] = performBranchTraining(array_of_counters_bimodal[pc_value_bimodal], str);
        }
        else if (strcmp(params.bp_name, "gshare") == 0)
        {
            pc_value_gshare = getPcValueForGshare(addr, params.M1, params.N);
            updateBHR(str, params.N);
            array_of_counters_gshare[pc_value_gshare] = performBranchTraining(array_of_counters_gshare[pc_value_gshare], str);
        }
        else if (strcmp(params.bp_name, "hybrid") == 0)
        {
            pc_value_hybrid = getPcValueForBimodal(addr, params.K);
            bool isBimodalPredictionCorrect = true;
            bool isGsharePredictionCorrect = true;

            pc_value_bimodal = getPcValueForBimodal(addr, params.M2);
            if ((array_of_counters_bimodal[pc_value_bimodal] < 2 && *str == 't') || (array_of_counters_bimodal[pc_value_bimodal] >= 2 && *str == 'n'))
            {
                isBimodalPredictionCorrect = false;
            }

            pc_value_gshare = getPcValueForGshare(addr, params.M1, params.N);
            if ((array_of_counters_gshare[pc_value_gshare] < 2 && *str == 't') || (array_of_counters_gshare[pc_value_gshare] >= 2 && *str == 'n'))
            {
                isGsharePredictionCorrect = false;
            }

            if (array_of_counters_hybrid[pc_value_hybrid] < 2)
            {
                if (*str == 't')
                {
                    if (array_of_counters_bimodal[pc_value_bimodal] < 2)
                    {
                        num_of_mispredictions++;
                    }
                    if (array_of_counters_bimodal[pc_value_bimodal] < 3)
                    {
                        array_of_counters_bimodal[pc_value_bimodal] = array_of_counters_bimodal[pc_value_bimodal] + 1;
                    }
                }
                else if (*str == 'n')
                {
                    if (array_of_counters_bimodal[pc_value_bimodal] >= 2)
                    {
                        num_of_mispredictions++;
                    }
                    if (array_of_counters_bimodal[pc_value_bimodal] > 0)
                    {
                        array_of_counters_bimodal[pc_value_bimodal] = array_of_counters_bimodal[pc_value_bimodal] - 1;
                    }
                }
            }
            else
            {
                if (*str == 't')
                {
                    if (array_of_counters_gshare[pc_value_gshare] < 2)
                    {
                        num_of_mispredictions++;
                    }
                    if (array_of_counters_gshare[pc_value_gshare] < 3)
                    {
                        array_of_counters_gshare[pc_value_gshare] = array_of_counters_gshare[pc_value_gshare] + 1;
                    }
                }
                else if (*str == 'n')
                {
                    if (array_of_counters_gshare[pc_value_gshare] >= 2)
                    {
                        num_of_mispredictions++;
                    }
                    if (array_of_counters_gshare[pc_value_gshare] > 0)
                    {
                        array_of_counters_gshare[pc_value_gshare] = array_of_counters_gshare[pc_value_gshare] - 1;
                    }
                }
            }
            updateBHR(str, params.N);
            if ((isBimodalPredictionCorrect && !isGsharePredictionCorrect) && (array_of_counters_hybrid[pc_value_hybrid] > 0))
            {
                array_of_counters_hybrid[pc_value_hybrid]--;
            }
            else if ((!isBimodalPredictionCorrect && isGsharePredictionCorrect) && (array_of_counters_hybrid[pc_value_hybrid] < 3))
            {
                array_of_counters_hybrid[pc_value_hybrid]++;
            }
        }
    }
    printf("OUTPUT\n");
    printf("number of predictions: %d \n", num_of_predictions);
    printf("number of mispredictions: %d \n", num_of_mispredictions);
    printf("misprediction rate: %0.2f\% \n", (float(num_of_mispredictions)/ float(num_of_predictions))*100);

    if(strcmp(params.bp_name, "hybrid") == 0){
        printf("FINAL CHOOSER CONTENTS\n");
        for (x = 0; x < pow(2, params.K); x++)
        {
            printf("%d %d \n", x, array_of_counters_hybrid[x]);
        }
    }

    if(strcmp(params.bp_name, "gshare") == 0 || strcmp(params.bp_name, "hybrid") == 0){
        printf("FINAL GSHARE CONTENTS\n");
        for (x = 0; x < pow(2, params.M1); x++)
        {
            printf("%d %d \n", x, array_of_counters_gshare[x]);
        }
    }

    if(strcmp(params.bp_name, "bimodal") == 0 || strcmp(params.bp_name, "hybrid") == 0){
        printf("FINAL BIMODAL CONTENTS\n");
        for (x = 0; x < pow(2, params.M2); x++)
        {
            printf("%d %d \n", x, array_of_counters_bimodal[x]);
        }
    }

    return 0;
}
