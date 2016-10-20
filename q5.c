#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#include <unistd.h>

typedef struct {
	double x1, y1, x2, y2;
}box;

struct timeval start, end;

// Generates 100 boxes to be used to test querying the db
void generate_boxes(int length, box *boxes) {
	// random number generator
	srand(time(NULL));
	for(int i = 0; i < 100; i ++) {
		double x = fmod(rand(), (1000 - length));
		double y = fmod(rand(), (1000 - length));
		boxes[i].x1 = x;
		boxes[i].x2 = x + length;
		boxes[i].y1 = y;
		boxes[i].y2 = y + length;
	}
}

// Takes in a long array of our 20 runtimes
long runtime_avg(long *times) {
	long avg = 0;
	for(int i = 0; i < 20; i++){
		avg = avg + times[i];
	}
	avg = avg / 20;
}

// Takes in a long array of our 100 query times
long querytime_avg(long *times) {
	long avg = 0;
	for(int i = 0; i < 100; i++){
		avg = avg + times[i];
	}
	avg = avg / 100;
}

int main(int argc, char ** argv) {

	//sqlite3 *db;
	//sqlite3_stmt *stmt;

	if (argc != 2) {
	  fprintf(stderr, "Usage: %s <length> \n", argv[0]);
	  return(1);
	} 

	box *boxes = malloc(100*sizeof(box));
	long *query_time = malloc(100*sizeof(long));
	long *runtimes = malloc(20*sizeof(long));
	//time_t start, end;
	long rtree_runtime, nortree_runtime;

// Query with R-Tree
	for(int i = 0; i < 20; i ++) {
		generate_boxes(atoi(argv[1]), boxes);
		for(int j = 0; j < 100; j ++) {
			gettimeofday(&start, NULL);
			//run query


			gettimeofday(&end, NULL);
			query_time[j] = (end.tv_usec - start.tv_usec)/1000;
		}
		rtree_runtime = querytime_avg(query_time);
		runtimes[i] = rtree_runtime;
	}
	rtree_runtime = runtime_avg(runtimes);

	printf("The total avg runtime is: %ld ms\n", rtree_runtime);
	free(query_time);
	free(runtimes);
	free(boxes);

// Query without R-Tree
}























