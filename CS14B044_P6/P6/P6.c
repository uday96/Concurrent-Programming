#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

void seed_by_time(int);
int ** allocate_matrix(int rows, int cols);
void initialize_matrix(int **matrix, int seed, int rows, int cols, char *fname, bool read_from_file);
void delete_matrix(int **matrix, int rows, int cols);
int updated_value(int old_val, int alive_neighbors);

int main(int argc, char ** argv) {
    // initial conditions and variable definitions
    int threads = 2;
    int seed = 101;
    int rows = 8;
    int cols = 8;
    int generations = 10;
    char *fname = NULL;
    size_t flen;
    int **matrix;
    int **matrix_updated;
    int **tmp;
    int provided;

    MPI_Init_thread(&argc, &argv,MPI_THREAD_SERIALIZED,&provided);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // check command line arguments
    if (argc != 7 && argc !=6) {
	    if (rank == 0){
		    printf("Usage: mpirun -np <num procs> %s <threads per proc> [Seed] [Rows] [Columns] [Generations] [opt-filename]\n", argv[0]);
        }
	    exit(EXIT_FAILURE);
    }

    sscanf(argv[1],"%d",&threads);
    sscanf(argv[2],"%d",&seed);
    sscanf(argv[3],"%d",&rows);
    sscanf(argv[4],"%d",&cols);
    sscanf(argv[5],"%d",&generations);

    if(argc == 7){
        flen = strlen(argv[6]);
        fname = (char*)malloc((flen+1)*sizeof(char));
        if (fname == NULL) {
            printf("Cannot allocate %zu bytes for string\n", flen+1);
            exit(EXIT_FAILURE);
        }
        sscanf(argv[6],"%s",fname);
    }

    omp_set_num_threads(threads);

    int chunks = floor((double)rows/nprocs);
    int row_start, row_end;
    row_start = rank*chunks;
    row_end = row_start+chunks-1;
    if(rank == nprocs-1 && row_end != rows-1){
        row_end = rows-1;
    }
    // printf("rank:%d - row_start:%d - row_end:%d\n",rank,row_start,row_end);

    seed_by_time(seed);
    matrix = allocate_matrix(rows,cols);
    initialize_matrix(matrix, seed, rows, cols, fname, argc==7);
    matrix_updated = allocate_matrix(rows,cols);
    initialize_matrix(matrix_updated, seed, rows, cols, fname, argc==7);

    int g,i,j;
    for(g=0;g<generations;g++){
        // printf("rank:%d - generation:%d\n",rank,g);
        for(i=row_start+1;i<=row_end-1;i++){
            if(i>=rows-1){
                break;
            }
            #pragma omp parallel for num_threads(threads)
            for(j=0;j<cols;j++){
                int alive_neighbors=0;
                alive_neighbors += (matrix[i-1][j] + matrix[i+1][j]);
                if(j-1>=0){
                    alive_neighbors += (matrix[i-1][j-1] + matrix[i][j-1] + matrix[i+1][j-1]);
                }
                if(j+1<=cols-1){
                    alive_neighbors += (matrix[i-1][j+1] + matrix[i][j+1] + matrix[i+1][j+1]);
                }
                matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
            }
        }
        // printf("rank:%d - local chunks done\n",rank);
        if(rank<nprocs-1){  //row_end != rows-1
            MPI_Send(&(matrix[row_end][0]), cols, MPI_INT, rank+1, rank, MPI_COMM_WORLD);
        }
        if(rank > 0){  //row_start > 0
            MPI_Send(&(matrix[row_start][0]), cols, MPI_INT, rank-1, rank, MPI_COMM_WORLD);
        }

        if(row_start+1<=row_end){
            if(row_start==0){
                i=row_start;
                #pragma omp parallel for num_threads(threads)
                for(j=0;j<cols;j++){
                    int alive_neighbors=0;
                    alive_neighbors += (matrix[i+1][j]);
                    if(j-1>=0){
                        alive_neighbors += (matrix[i][j-1] + matrix[i+1][j-1]);
                    }
                    if(j+1<=cols-1){
                        alive_neighbors += (matrix[i][j+1] + matrix[i+1][j+1]);
                    }
                    matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
                }                
                // row_end == row_start == 0
            }
            else{
                i=row_start;
                if(rank>0){
                    MPI_Recv(&(matrix[row_start-1][0]), cols, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                #pragma omp parallel for num_threads(threads)
                for(j=0;j<cols;j++){
                    int alive_neighbors=0;
                    alive_neighbors += (matrix[i-1][j] + matrix[i+1][j]);
                    if(j-1>=0){
                        alive_neighbors += (matrix[i-1][j-1] + matrix[i][j-1] + matrix[i+1][j-1]);
                    }
                    if(j+1<=cols-1){
                        alive_neighbors += (matrix[i-1][j+1] + matrix[i][j+1] + matrix[i+1][j+1]);
                    }
                    matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
                }
            }
        }
        // printf("rank:%d - ghost start chunks done\n",rank);
        
        if(row_end-1>=row_start){
            if(row_end==rows-1){
                i=row_end;
                #pragma omp parallel for num_threads(threads)
                for(j=0;j<cols;j++){
                    int alive_neighbors=0;
                    alive_neighbors += (matrix[i-1][j]);
                    if(j-1>=0){
                        alive_neighbors += (matrix[i][j-1] + matrix[i-1][j-1]);
                    }
                    if(j+1<=cols-1){
                        alive_neighbors += (matrix[i][j+1] + matrix[i-1][j+1]);
                    }
                    matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
                }
                // row_end == row_start == 0
            }
            else{
                i=row_end;
                if(rank<nprocs-1){
                    MPI_Recv(&(matrix[row_end+1][0]), cols, MPI_INT, rank+1, rank+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                #pragma omp parallel for num_threads(threads)
                for(j=0;j<cols;j++){
                    int alive_neighbors=0;
                    alive_neighbors += (matrix[i-1][j] + matrix[i+1][j]);
                    if(j-1>=0){
                        alive_neighbors += (matrix[i-1][j-1] + matrix[i][j-1] + matrix[i+1][j-1]);
                    }
                    if(j+1<=cols-1){
                        alive_neighbors += (matrix[i-1][j+1] + matrix[i][j+1] + matrix[i+1][j+1]);
                    }
                    matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
                }
            }
        }
        // printf("rank:%d - ghost end chunks done\n",rank);

        if(row_start == row_end){
            i=row_start;
            if(rank>0){
                MPI_Recv(&(matrix[row_start-1][0]), cols, MPI_INT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            if(rank<nprocs-1){
                MPI_Recv(&(matrix[row_end+1][0]), cols, MPI_INT, rank+1, rank+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            if(row_start == 0){
                #pragma omp parallel for num_threads(threads)
                for(j=0;j<cols;j++){
                    int alive_neighbors=0;
                    alive_neighbors += (matrix[i+1][j]);
                    if(j-1>=0){
                        alive_neighbors += (matrix[i][j-1] + matrix[i+1][j-1]);
                    }
                    if(j+1<=cols-1){
                        alive_neighbors += (matrix[i][j+1] + matrix[i+1][j+1]);
                    }
                    matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
                }
            }
            else if(row_start == rows-1){
                #pragma omp parallel for num_threads(threads)
                for(j=0;j<cols;j++){
                    int alive_neighbors=0;
                    alive_neighbors += (matrix[i-1][j]);
                    if(j-1>=0){
                        alive_neighbors += (matrix[i][j-1] + matrix[i-1][j-1]);
                    }
                    if(j+1<=cols-1){
                        alive_neighbors += (matrix[i][j+1] + matrix[i-1][j+1]);
                    }
                    matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
                }
            }
            else{
                #pragma omp parallel for num_threads(threads)
                for(j=0;j<cols;j++){
                    int alive_neighbors=0;
                    alive_neighbors += (matrix[i-1][j] + matrix[i+1][j]);
                    if(j-1>=0){
                        alive_neighbors += (matrix[i-1][j-1] + matrix[i][j-1] + matrix[i+1][j-1]);
                    }
                    if(j+1<=cols-1){
                        alive_neighbors += (matrix[i-1][j+1] + matrix[i][j+1] + matrix[i+1][j+1]);
                    }
                    matrix_updated[i][j] = updated_value(matrix[i][j],alive_neighbors);
                }
            }
        }

        tmp = matrix;
        matrix = matrix_updated;
        matrix_updated = tmp;
    }

    if(rank>0){
        int r=0;
        for(r=row_start;r<=row_end;r++){
            MPI_Send(&(matrix[r][0]), cols, MPI_INT, 0, rank+nprocs+r, MPI_COMM_WORLD);
        }
    }
    else{
        int p=0;
        int p_row_start, p_row_end;
        for(p=1;p<=nprocs-1;p++){
            p_row_start = p*chunks;
            p_row_end = p_row_start+chunks-1;
            if(p == nprocs-1 && p_row_end != rows-1){
                p_row_end = rows-1;
            }
            int r=0;
            for(r=p_row_start;r<=p_row_end;r++){
                MPI_Recv(&(matrix[r][0]), cols, MPI_INT, p, p+nprocs+r, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }

    if(rank == 0){
        for(i=0;i<rows;i++){
            for(j=0;j<cols;j++){
                printf("%d\n",matrix[i][j]);
            }
            // printf("\n");
        }
    }

    MPI_Finalize();

    free(fname);
    fname = NULL;
    delete_matrix(matrix,rows,cols);
    delete_matrix(matrix_updated,rows,cols);
    return 0;
}

void seed_by_time(int offset) {
    srand(offset);
}

int ** allocate_matrix(int rows, int cols){
    int i;
    int ** matrix;
    matrix = (int **) malloc (sizeof(int*)*rows);
    #pragma omp parallel for
    for (i=0;i<rows;i++) {
        matrix[i] = (int *) malloc (sizeof(int)*cols);
    }
    return matrix;
}

void initialize_matrix(int **matrix, int seed, int rows, int cols, char *fname, bool read_from_file){
    int i,j;
    if(read_from_file){
        FILE *file;
        file=fopen(fname, "r");
        for(i=0;i<rows;i++){
            for(j=0;j<cols;j++){
                if(!fscanf(file, "%d", &matrix[i][j])){
                    break;
                }
            }
        }
        fclose(file);
    }
    else{
        for(i=0; i<rows; i++){
            for(j=0; j<cols;j++){
                matrix[i][j] = (int)(random()%2);
            }
        }
    }
}

void delete_matrix(int **matrix, int rows, int cols){
    int i;
    for (i=0;i<rows;i++){
        free(matrix[i]);
    }
    free(matrix);
}

int updated_value(int old_val, int alive_neighbors){
    if(old_val==1 && alive_neighbors<2){
        return 0;
    }
    if(old_val==1 && (alive_neighbors==2 || alive_neighbors==3)){
        return 1;
    }
    if(old_val==1 && alive_neighbors>3){
        return 0;
    }
    if(old_val==0 && alive_neighbors==3){
        return 1;
    }
    return old_val;
}