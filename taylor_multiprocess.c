#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

double factorial(int n) {
	double result = 1.0;
	for (int i = 2; i <= n; i++) result *= i;
	return result;
}

int main() {
	int n_terms = 20;
	double x = 1.0;
	int n_procs = 4;
	int terms_per_proc = n_terms / n_procs;

	int pipefd[4][2];
	pid_t pids[4];

	for (int i = 0; i < n_procs; i++) {
		if (pipe(pipefd[i]) == -1) {
			perror("pipe");
			exit(1);
		}
	}

	for (int i = 0; i < n_procs; i++) {
		pids[i] = fork();

		if (pids[i] < 0) {
			perror("fork");
			exit(1);
		}

		if (pids[i] == 0) {
			close(pipefd[i][0]);

			int start = i * terms_per_proc;
			int end = (i == n_procs - 1) ? n_terms : start + terms_per_proc;

			double partial_sum = 0.0;
			for (int k = start; k < end; k++) {
				partial_sum += pow(x, k) / factorial(k);
			}

			write(pipefd[i][1], &partial_sum, sizeof(double));
			close(pipefd[i][1]);
			exit(0);
		} else {
			close(pipefd[i][1]);
		}
	}

	double total = 0.0;
	for (int i = 0; i < n_procs; i++) {
		double partial;
		read(pipefd[i][0], &partial, sizeof(double));
		total += partial;
		close(pipefd[i][0]);
		wait(NULL);
	}

	printf("Approximation of e^%.2f using %d terms = %.10f\n", x, n_terms, total);
	printf("Actual e^%.2f = %.10f\n", x, exp(x));

	return 0;
}
