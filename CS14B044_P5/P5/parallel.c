/*
 * Skeleton by: Krishna.
 * Fill in your code here.
 */
#include<stdio.h>
#include<stdlib.h>
#include <mpi.h>

#define UNBURNT 0
#define SMOLDERING 1
#define BURNING 2
#define BURNT 3

int distributed_delete_forest(int forest_size, int **forest){
	// Return 1, if you do the memory deallocation in a distributed manner.
	return 0;
}
int distributed_allocate(int ***forest, int forest_size, double **percent_burned, int n_probs){
    // Return 1, if you do the memory allocatation in a distributed manner.
    return 0;
}

int get_num_burned(int forest_size,int ** forest) {
    int i,j;
    int sum=0;

    // calculate pecrent burned
    for (i=0;i<forest_size;i++) {
        for (j=0;j<forest_size;j++) {
            if (forest[i][j]==BURNT) {
                sum++;
            }
        }
    }
    // printf("burn_sum:%d\n",sum);

    // return percent burned;
    return sum;
}

void do_parallel(int n_probs, double *prob_spread, double *percent_burned, int forest_size, int **forest, int n_trials){
	
	int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // for a number of probabilities, calculate
    // average burn and output
    double prob_step;
    double prob_min=0.0;
    double prob_max=1.0;
    int i_trial;
    int i_prob;
    prob_step = (prob_max-prob_min)/(double)(n_probs-1);
    // printf("rank:%d - prob_step:%f\n",rank,prob_step);
    if(rank==0){
    	printf("Probability of fire spreading, Average percent burned\n");
    }
    int total = forest_size*forest_size-1;
    for (i_prob = 0; i_prob < n_probs; i_prob+=1) {
    	// printf("rank:%d - i_prob:%d\n",rank,i_prob);
        //for a number of trials, calculate average
        //percent burn
        prob_spread[i_prob] = prob_min + (double)i_prob * prob_step;
        percent_burned[i_prob]=0.0;
        for (i_trial=0; i_trial < n_trials; i_trial++) {
            //start fire and burn until fire is gone
            burn_until_out(forest_size,forest,prob_spread[i_prob],forest_size/2,forest_size/2);
            int burn_sum = get_num_burned(forest_size,forest);
            double d = ((double)(burn_sum-1)/(double)total);
            percent_burned[i_prob]+=d;
            // printf("rank:%d - trail:%d - burn:%f - %f\n",rank,i_trial,d,percent_burned[i_prob]);
        }
        percent_burned[i_prob]/=n_trials;
        // print output
        if(rank==0){
        	printf("%lf , %lf\n",prob_spread[i_prob], percent_burned[i_prob]);
        }
    }
}

void print_time(double tm){
    printf("elapsed time = %f seconds\n", tm);
}
