/*
 * correctness.c -- tests the properties memtest.c does not cover:
 * free() deallocates, adjacent free chunks are coalesced, bad free() calls
 * are detected, and leaks are reported.
 *
 * Usage:
 *   ./correctness       run the functional tests, print PASS/FAIL for each,
 *                       exit 0 if all pass and 1 otherwise
 *   ./correctness N     run error/leak scenario N (1-6); see the README for
 *                       what each one should print
 *
 * Assumes MEMLENGTH 4096 and an 8-byte header, so the biggest possible
 * object is 4096 - 8 = 4088 bytes.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

#define MEMLENGTH 4096
#define HEADERSIZE 8
#define BIGGEST (MEMLENGTH - HEADERSIZE)

int failed = 0;

void check(char *name, int ok) {
	printf("%-45s %s\n", name, ok ? "PASS" : "FAIL");
	if (!ok) failed++;
}

/* free() deallocates: once the heap is full malloc must fail,
   and after freeing, the same request must work again */
int test_free_deallocates() {
	char *big = malloc(BIGGEST);
	if (big == NULL) return 0;
	char *extra = malloc(1);          /* heap is full: should be NULL */
	free(big);
	char *again = malloc(BIGGEST);    /* should work now */
	int ok = (extra == NULL && again != NULL);
	free(again);
	return ok;
}

/* free() makes the space reusable: the next malloc of the same size
   gets the freed object's address back */
int test_reuse() {
	char *a = malloc(100);
	char *b = malloc(100);
	free(a);
	char *c = malloc(100);
	int ok = (c == a);
	free(b);
	free(c);
	return ok;
}

/* coalescing: fill the heap with small 16-byte chunks, free them all,
   then the biggest object only fits if they were merged back together.
   if backwards is 1 the chunks are freed last-to-first. */
int test_coalesce(int backwards) {
	int n = MEMLENGTH / 16;           /* 256 chunks fill the heap exactly */
	char *p[MEMLENGTH / 16];
	int i;
	for (i = 0; i < n; i++) {
		p[i] = malloc(8);
		if (p[i] == NULL) return 0;
	}
	if (backwards) {
		for (i = n - 1; i >= 0; i--) free(p[i]);
	} else {
		for (i = 0; i < n; i++) free(p[i]);
	}
	char *big = malloc(BIGGEST);
	int ok = (big != NULL);
	free(big);
	return ok;
}

/* coalescing a chunk with free chunks on BOTH sides: a, b, c are next to
   each other; free a and c first, then b. all three should merge, so a
   64-byte object fits exactly where a was. */
int test_coalesce_middle() {
	char *a = malloc(16);
	char *b = malloc(16);
	char *c = malloc(16);
	char *rest = malloc(MEMLENGTH - 3 * 24 - HEADERSIZE);  /* fill the rest of the heap */
	if (a == NULL || b == NULL || c == NULL || rest == NULL) return 0;
	free(a);
	free(c);
	free(b);
	char *big = malloc(64);
	int ok = (big == a);
	free(big);
	free(rest);
	return ok;
}

/* the example from section 1.1: fill memory with 24-byte objects,
   free them all, then a 48-byte request must work */
int test_spec_example() {
	int n = MEMLENGTH / 32;           /* each 24-byte object is a 32-byte chunk */
	char *p[MEMLENGTH / 32];
	int i;
	for (i = 0; i < n; i++) {
		p[i] = malloc(24);
		if (p[i] == NULL) return 0;
	}
	for (i = 0; i < n; i++) free(p[i]);
	char *q = malloc(48);
	int ok = (q != NULL);
	free(q);
	return ok;
}

int global_var;

/* error and leak scenarios. 1-3 are the three errors from section 2.1 and
   should end with "free: Inappropriate pointer" and exit status 2. */
int run_scenario(int n) {
	if (n == 1) {                     /* address not from malloc */
		int x;
		free(&x);
	} else if (n == 2) {              /* address not at the start of a chunk */
		int *p = malloc(sizeof(int) * 2);
		free(p + 1);
	} else if (n == 3) {              /* freeing the same pointer twice */
		int *p = malloc(sizeof(int) * 100);
		int *q = p;
		free(p);
		free(q);
	} else if (n == 4) {              /* address of a global variable */
		free(&global_var);
	} else if (n == 5) {              /* leak: 8 + 24 + 96 = 128 bytes in 3 objects */
		malloc(8);
		malloc(20);                   /* rounded up to 24 */
		malloc(96);
		return 0;
	} else if (n == 6) {              /* no leak: should print nothing */
		char *a = malloc(10);
		char *b = malloc(20);
		free(a);
		free(b);
		return 0;
	} else {
		printf("unknown scenario %d (use 1-6)\n", n);
		return 1;
	}
	printf("ERROR NOT DETECTED\n");   /* scenarios 1-4 should never get here */
	return 1;
}

int main(int argc, char **argv) {
	if (argc == 2) return run_scenario(atoi(argv[1]));

	check("free() deallocates memory", test_free_deallocates());
	check("freed memory is reused", test_reuse());
	check("coalescing (freed first to last)", test_coalesce(0));
	check("coalescing (freed last to first)", test_coalesce(1));
	check("coalescing with free chunks on both sides", test_coalesce_middle());
	check("24-byte objects coalesce into 48 bytes", test_spec_example());

	printf("%d test(s) failed\n", failed);
	return failed ? 1 : 0;
}
