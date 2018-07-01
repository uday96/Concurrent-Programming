1. Distributed the matrix into chunks of rows with all the coloumns and assigned to each mpi process
2. For rows in the chunks which are not at the border, all its neoghbors are present within the same node
3. For border rows, we need data from the neighboring mpi process border rows to get all the neighbors details so we use MPI Send and Recv
4. Within an MPI process, launched threads across coloumns for computing the survival of an element