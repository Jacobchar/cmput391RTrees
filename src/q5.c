#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>
#include "../sqlite3.h"

/* Define Statements */
#define SUCCESS 0
#define FAILURE 1

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

	if (argc != 2) {
	  fprintf(stderr, "Usage: %s <length> \n", argv[0]);
	  return(1);
	} 

	sqlite3 *db;
	sqlite3_stmt *stmt;

	int rc;

	rc = sqlite3_open("../assignment2.db", &db);
	if( rc ) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		return FAILURE;
	}

	char *sql_stmt = "SELECT * FROM poiboxes "
					 "WHERE minx >= ? "
					 "AND miny >= ?"
					 "AND maxx <= ?"
					 "AND maxy <= ?";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
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
			
			// Bind our wildcards
			sqlite3_bind_double(stmt, 1, boxes[j].x1);
			sqlite3_bind_double(stmt, 2, boxes[j].y1);
			sqlite3_bind_double(stmt, 3, boxes[j].x2);
			sqlite3_bind_double(stmt, 4, boxes[j].y2);

			// Start timer
			gettimeofday(&start, NULL);
			
			//run query
			while(sqlite3_step(stmt) == SQLITE_ROW) {}

			// End time, determine time to query 100 boxes
			gettimeofday(&end, NULL);
			query_time[j] = (end.tv_usec - start.tv_usec)/1000;

			// Clear Bindings
			rc = sqlite3_clear_bindings(stmt);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Could not clear bindings: %s\n", sqlite3_errmsg(db));
				return FAILURE;
			}
		}
		// For our 20 trials store the time for querying
		rtree_runtime = querytime_avg(query_time);
		runtimes[i] = rtree_runtime;
	}

	// Calculate our average time for querying the R-tree
	rtree_runtime = runtime_avg(runtimes);

	printf("The total avg runtime is: %ld ms\n", rtree_runtime);

	sqlite3_finalize(stmt);
	free(query_time);
	free(runtimes);
	free(boxes);

// Query without R-Tree
}























