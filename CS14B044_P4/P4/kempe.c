/*
 * =====================================================================================
 *
 *       Filename:  kempe.c
 *
 *    Description:  Code to do coloring using Kempes heuristic
 *
 *        Version:  1.0
 *        Created:  12/03/2018 09:59:42 IST
 *       Revision:  none
 *       Compiler:  gcc -std=c99
 *
 *         Author:  ?
 *
 * =====================================================================================
 */
#include<math.h>
#include<stdlib.h>
#include<strings.h>
#include"kempe.h"
#include"omp.h"

/* *
Graph coloring - Kempe's heuristic Algorithm dating back to 1879.
Input : G - the interference graph, K - number of colors
repeat
	repeat
		Remove a node n and all its edges from G , such that degree of n is less than K;
		Push n onto a stack;
	until G has no node with degree less than K;
	// G is either empty or all of its nodes have degree >= K
	if G is not empty then
		Take one node m out of G;
		push m onto the stack;
	end
until G is empty;
Take one node at a time from the stack and assign a non conflicting color (if possible, else spill).
 *
 */

void kempe_serial(struct node **B, int n, int col){
	stack_init(n);
	int ln_index;
	do {
		int flag;
		do {
			for (int i=0;i<n;++i){
				if (B[i]->deleted) continue;
				if (B[i]->deg < col) {
					push(B[i]);
					B[i]->deleted = 1;
					remove_incident_edges(B[i]);
				}
			}
			flag = non_critical_nodes_present(B, n, col);
		} while (flag);

		ln_index = live_nodes_present(B, n, col);
		if (ln_index != -1) {
			push(B[ln_index]);
			B[ln_index]->deleted = 1;
			B[ln_index]->color = -1; // spilled
			remove_incident_edges(B[ln_index]);
		} 
	} while (ln_index != -1);
	do_color(B, n, col);
}

void do_color(struct node **B, int n, int col){
	int *c = (int *)calloc (col, sizeof(int));
	for (int i=0;i<n;++i){
		struct node* nd = pop();
		if (nd->deg == 0) { // special case.
			nd->color = 0; 
			nd->deleted = 0;
			restore_incident_edges(nd);
			continue;
		}
		int j;
		for (j = 0; j < nd->nncnt; ++j){ // note which colors are taken by the neighbors.
			if (nd->neighbors[j]->deleted) continue;
			c[nd->neighbors[j]->color] = 1;
		}
		for (j=0;j<col; ++j){ // find the available color.
			if (c[j] == 0) break;
		}
		if (j == col) {nd->deleted = -1; nd->color = -1;} // spilled
		else {nd->deleted = 0; nd->color = j; }
		restore_incident_edges(nd);
		bzero(c, n*sizeof(int));
	}
}


void remove_incident_edges_parallel(struct node *x){
	#pragma omp parallel for
	for (int j=0;j<x->nncnt;++j){
		x->neighbors[j]->deg --;
	}
}

void kempe_parallel(struct node **A, int n, int col, int nproc){
	omp_set_num_threads(nproc);
	stack_init(n);
	int ln_index;
	do {
		int flag;
		do {
			for (int i=0;i<n;++i){
				if (A[i]->deleted) continue;
				if (A[i]->deg < col) {
					push(A[i]);
					A[i]->deleted = 1;
					remove_incident_edges_parallel(A[i]);
				}
			}
			flag = non_critical_nodes_present(A, n, col);
		} while (flag);

		ln_index = live_nodes_present(A, n, col);
		if (ln_index != -1) {
			push(A[ln_index]);
			A[ln_index]->deleted = 1;
			A[ln_index]->color = -1; // spilled
			remove_incident_edges_parallel(A[ln_index]);
		} 
	} while (ln_index != -1);
	do_color(A, n, col);
	return ;
}
