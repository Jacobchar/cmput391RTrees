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
#define NUMBER_OF_BOXES 100
#define NUMBER_OF_RUNS 20

/* Box struct */
typedef struct {
	double x1, y1, x2, y2;
}box;

/* Global timing variables */
struct timeval start, end;

/* Function Prototypes */
void generate_boxes(int, box*);
double runtime_avg(double *);
double querytime_avg(double *);
int time_rtree(sqlite3 *, sqlite3_stmt *, box*, double *, int);
int time_coordinate_indexes(sqlite3 *, sqlite3_stmt *, box *, double *, int);
int create_indexes(sqlite3 *, sqlite3_stmt *);
int drop_indexes(sqlite3 *, sqlite3_stmt *);

int main(int argc, char ** argv) {

	if (argc != 2) {
	  fprintf(stderr, "Usage: %s <length> \n", argv[0]);
	  return FAILURE;
	} 

	sqlite3 *db;
	sqlite3_stmt *stmt;

		int rc, i;

		/* Open the database */
		rc = sqlite3_open("../assignment2.db", &db);
		if( rc ) {
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
			return FAILURE;
		}
		
		/* Runtime memory for storing the runs, and a boxes array to generate all the boxes */
		double *rtree_runtimes = malloc(NUMBER_OF_RUNS * sizeof(double));
		double *index_coordinate_runtimes = malloc(NUMBER_OF_RUNS * sizeof(double));
		box *boxes = malloc(NUMBER_OF_BOXES * sizeof(box));

		/* Create indexes to compare to rtree */
		rc = create_indexes(db, stmt);
		if( rc ) {
			printf("Error creating indexes.\n");
			return FAILURE;
		}

		generate_boxes(atoi(argv[1]), boxes);

		/* Main execution loop */
		for(i = 0; i < NUMBER_OF_RUNS; i++) {		
			
			/* Create the boxes unique to this iteration */
			
			
			/* Calculate the rtree runtime for this set of boxes */
			rc = time_rtree(db, stmt, boxes, rtree_runtimes, i);
			if(rc) {
				printf("Error calculating rtree runtimes.\n");
				return FAILURE;
			}

			/* Calculate the indexes runtime for this set of boxes */
			rc = time_coordinate_indexes(db, stmt, boxes, index_coordinate_runtimes, i);
			if(rc) {
				printf("Error calculating index runtimes.\n");
				return FAILURE;
			}
		}

		/* Calculate the averages of the total runs for each trial */
		double rtree_result = runtime_avg(rtree_runtimes);
		double index_result = runtime_avg(index_coordinate_runtimes);

		printf("Parameter l: %s\n", argv[1]);
		printf("Average runtime with r-tree: %f ms\n", rtree_result);
		printf("Average runtime without r-tree: %f ms\n", index_result);

	/* Clean up */
	rc = drop_indexes(db, stmt);
	free(rtree_runtimes);
	free(index_coordinate_runtimes);
	free(boxes);

	return SUCCESS;
}

int time_rtree(sqlite3 *db, sqlite3_stmt *stmt, box *boxes, double *runtimes, int iteration) {

	int rc, j;

	double *query_time = malloc(NUMBER_OF_BOXES * sizeof(double));

	char *sql_stmt = 	"SELECT * "
						"FROM poiboxes "
					 	"WHERE minx >= ? "
					 	"AND miny >= ?"
					 	"AND maxx <= ?"
					 	"AND maxy <= ?;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	for(j = 0; j < NUMBER_OF_BOXES; j ++) {		
		
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

		query_time[j] = (end.tv_usec - start.tv_usec);

		// Clear Bindings
		rc = sqlite3_clear_bindings(stmt);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "Could not clear bindings: %s\n", sqlite3_errmsg(db));
			return FAILURE;
		}
	}

	runtimes[iteration] = querytime_avg(query_time);

	free(query_time);
	sqlite3_reset(stmt);
	
	return SUCCESS;	
}

int time_coordinate_indexes(sqlite3 *db, sqlite3_stmt *stmt, box *boxes, double *runtimes, int iteration) {
	
	int rc, j;

	/* Query the index and time the returns. */
	double *query_time = malloc(NUMBER_OF_BOXES * sizeof(double));

	char *sql_stmt = 	"SELECT * "
						"FROM poi2 "
			 			"WHERE minx >= ? "
					 	"AND miny >= ?"
					 	"AND maxx <= ?"
					 	"AND maxy <= ?;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (x1_index query): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	for(j = 0; j < NUMBER_OF_BOXES; j ++) {		
		
		// Bind our wildcards
		sqlite3_bind_double(stmt, 1, boxes[j].x1);
		sqlite3_bind_double(stmt, 2, boxes[j].y1);
		sqlite3_bind_double(stmt, 3, boxes[j].x2);
		sqlite3_bind_double(stmt, 4, boxes[j].y2);

		// Start timer
		gettimeofday(&start, NULL);
		
		//run query
		while(sqlite3_step(stmt) == SQLITE_ROW) {}
		for(int k = 0; k< 10000; k++){}
	
		// End time, determine time to query 100 boxes
		gettimeofday(&end, NULL);

		query_time[j] = (end.tv_usec - start.tv_usec);
		
		// Clear Bindings
		rc = sqlite3_clear_bindings(stmt);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "Could not clear bindings: %s\n", sqlite3_errmsg(db));
			return FAILURE;
		}
	}

	/* Store the averaged runtime of all the boxes for this iteration */
	runtimes[iteration] = querytime_avg(query_time);

	/* Clean up */
	free(query_time);
	sqlite3_reset(stmt);
	
	return SUCCESS;	
}

int create_indexes(sqlite3 *db, sqlite3_stmt *stmt) {
	int rc;

	/* Create the x1 index to test against */
	char *sql_stmt = 	"CREATE INDEX IF NOT EXISTS x1_index "
						"ON poi2(minx);";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		printf("%d\n", rc);
		fprintf(stderr, "Preparation failed (create x1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (create x1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);

	/* Create the x2 index to test against */
	sql_stmt = 	"CREATE INDEX IF NOT EXISTS x2_index "
				"ON poi2(maxx);";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (create x2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (create x2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);

	/* Create the x1 index to test against */
	sql_stmt = 	"CREATE INDEX IF NOT EXISTS y1_index "
				"ON poi2(miny);";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (create y1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (create y1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);

	/* Create the x1 index to test against */
	sql_stmt = 	"CREATE INDEX IF NOT EXISTS y2_index "
				"ON poi2(maxy);";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (create y2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (create y2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);

	return SUCCESS;
}

int drop_indexes(sqlite3 *db, sqlite3_stmt *stmt) {
	int rc;

	/* Drop the x1 index */
	char *sql_stmt = 	"DROP INDEX IF EXISTS x1_index;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		printf("%d\n", rc);
		fprintf(stderr, "Preparation failed (drop x1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (drop x1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);

	/* Drop the x2 index */
	sql_stmt = 	"DROP INDEX IF EXISTS x2_index;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		printf("%d\n", rc);
		fprintf(stderr, "Preparation failed (drop x2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (drop x2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);

	/* Drop the y1 index */
	sql_stmt = 	"DROP INDEX IF EXISTS y1_index;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		printf("%d\n", rc);
		fprintf(stderr, "Preparation failed (drop y1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (drop y1 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);

	/* Drop the y2 index */
	sql_stmt = 	"DROP INDEX IF EXISTS y2_index;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		printf("%d\n", rc);
		fprintf(stderr, "Preparation failed (drop y2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE) {
		fprintf(stderr, "Operation failed (drop y2 index): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_reset(stmt);	

	return SUCCESS;
}
/* Generates 100 boxes to be used to test querying the db */
void generate_boxes(int length, box *boxes) {
	/* random number generator */
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

/* Takes in a double array of our 20 runtimes */
double runtime_avg(double *times) {
	double avg = 0;
	for(int i = 0; i < NUMBER_OF_RUNS; i++){
		avg += times[i];
	}
	avg = avg / NUMBER_OF_RUNS;
	return avg;
}

/* Takes in a double array of our 100 query times */
double querytime_avg(double *times) {
	double avg = 0;
	for(int i = 0; i < NUMBER_OF_BOXES; i++){
		avg += times[i];
	}
	avg = (avg / (NUMBER_OF_BOXES))/1000;
	return avg;
}
