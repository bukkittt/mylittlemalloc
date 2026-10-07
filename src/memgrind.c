#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mymalloc.h"

#define RUNS 50
#define NOBJS 120

int failures = 0;

void task1() {
	char *a = malloc(8);
	char *b = malloc(16);
	char *c = malloc(32);
	char *d = malloc(64);
	char *e = malloc(128);
	char *f = malloc(512);
	char *g = malloc(1024);
	if(a==NULL||b==NULL||c==NULL||d==NULL||e==NULL||f==NULL||g==NULL) failures++;
	free(g);
	free(f);
	free(e);
	free(d);
	free(c);
	free(b);
	free(a);
}

void task2(){
	char *p[NOBJS];
	int i, j;
	for(i=0; i < NOBJS; i++){
		p[i]=malloc(1);
		if (p[i] == NULL) failures++;
	}

	for(i=0; i< NOBJS; i++){
		for(j=0;j<NOBJS;j++) {
			if(p[j] != NULL){
				free(p[j]);
				p[j] = NULL;
				break;
			}
		}
	}
}

int count_live(char **p, int n){
	int i, count = 0;
	for(i = 0; i<n; i++){
		if(p[i]!=NULL) count++;
	}
	return count;
}

void task3() {
	char *p[NOBJS];
	int i, allocs = 0;
	for(i=0; i<NOBJS; i++) p[i]=NULL;

	while(allocs < NOBJS) {
		if(count_live(p,NOBJS) == 0 || rand() % 2 == 0){
			for(i=0; i<NOBJS; i++){
				if(p[i]==NULL){
					p[i] = malloc(1);
					if(p[i]==NULL) failures++;
					break;
				}
			}
			allocs++;
		} else {
			int k = rand() % NOBJS;
			while(p[k] == NULL) k = rand() % NOBJS;
			free(p[k]);
			p[k]=NULL;
		}
	}
	for(i = 0; i < NOBJS; i++) {
		if(p[i] != NULL) {
			free(p[i]);
			p[i]=NULL;
		}
	}
}

void task4() {
	char *p[40];
	int i, j, n;
	for(i = 0; i< 40; i++) {
		n = 1 + rand() % 64;
		p[i] = malloc(n);
		if (p[i] == NULL) failures++;
		else for (j=0; j<n; j++) p[i][j] = i;
	}
	for(i = 0; i < 40; i++){
		if(i%2 == 0){
			free(p[i]);
			p[i] = NULL;
		}
	}
	for(i = 0; i < 40; i++){
		if(i%2==0){
			p[i] = malloc(1 + rand() % 64);
			if (p[i] == NULL) failures++;
		}
	}
// odd indices descending, then even ones descending

	int parity;
	for(parity = 1; parity >= 0; parity--){
		while(1) {
			int best = -1;
			for(i = 0; i<40; i++){
				if (i % 2 == parity && p[i] !=NULL) best = i;
			}
			if (best == -1) break;
			free(p[best]);
			p[best] = NULL;
		}
	}
}

void task5() {
	int size = 8;
	int i;
	char *buf = malloc(size);
	if(buf==NULL) {failures++; return;}
	for(i=0; i< size; i++) buf[i] = 'x';

	while(size < 1024){
		char *bigger = malloc(size*2);
		if (bigger == NULL) {failures++; break;}
		for(i = 0; i < size; i++) bigger[i] = buf[i];
		for(i = size; i<size*2; i++) bigger[i] = 'x';
		free(buf);
		buf = bigger;
		size *= 2;
		for (i = 0; i < size; i++) if(buf[i] != 'x') failures++;
	}

	while(size > 8){
		char *smaller = malloc(size/2);
		if(smaller == NULL) {failures++; break;}
		for(i=0; i < size/2; i++) smaller [i] = buf[i];
		free(buf);
		buf=smaller;
		size /= 2;
		for (i = 0; i < size; i++) if (buf[i] != 'x') failures++;
	}

	free(buf);
}


int main(){
	struct timeval start, end, t0, t1;
	double times[5] = {0,0,0,0,0};
	int run;
	double mil = 1000000.0;
	srand(214);
	gettimeofday(&start, NULL);
	for ( run = 0; run < RUNS; run ++) {
		gettimeofday(&t0, NULL); task1(); gettimeofday(&t1, NULL);
		times[0] += (t1.tv_sec - t0.tv_sec) * mil + (t1.tv_usec - t0.tv_usec);
		gettimeofday(&t0, NULL); task2(); gettimeofday(&t1, NULL);
                times[1] += (t1.tv_sec - t0.tv_sec) * mil + (t1.tv_usec - t0.tv_usec);
		gettimeofday(&t0, NULL); task3(); gettimeofday(&t1, NULL);
                times[2] += (t1.tv_sec - t0.tv_sec) * mil + (t1.tv_usec - t0.tv_usec);
		gettimeofday(&t0, NULL); task4(); gettimeofday(&t1, NULL);
                times[3] += (t1.tv_sec - t0.tv_sec) * mil + (t1.tv_usec - t0.tv_usec);
		gettimeofday(&t0, NULL); task5(); gettimeofday(&t1, NULL);
                times[4] += (t1.tv_sec - t0.tv_sec) * mil + (t1.tv_usec - t0.tv_usec);
	}
	gettimeofday(&end, NULL);
	double elapsed = (end.tv_sec - start.tv_sec) * mil + (end.tv_usec - start.tv_usec);

	printf("memgrind: %d runs\n", RUNS);
	for (run = 0; run < 5; run++)
		printf("  task %d average: %8.2f microseconds\n", run + 1, times[run] /  RUNS);
	printf("Average time for workload: %.2f microseconds\n", elapsed / RUNS);
	if(failures > 0){
		printf("Warning: %d allocations failed\n", failures);
		return 1;
	}
	return 0;
}































