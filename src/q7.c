

// Open the database

//COnfirm the arguments are of a point x1, y1

//Get the depth of the tree using rtreedepth - 0 is the root node, 1, 2.... then leaves represent max + 1

//Write a mindist calculation between the point and the MBR's

//Write a minmaxdist calculation between the point the MBR's

//Write a mindist calculation between the point and an object (10/10 square)


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../sqlite3.h"

#define SUCCESS 0
#define FAILURE 1
#define ROOT 1

/* Structure Declarations */
typedef struct {
	int id;
	double minx;
	double maxx;
	double miny;
	double maxy;
} NodeStuff; 

typedef struct {
	int id;
	double minDist;
	double minMaxDist;
} currentNearest;

/* Function Prototypes */
void ParseAndStoreBLOB(char *);
int getBLOBData(sqlite3 *, sqlite3_stmt *, int);
int getDepth(sqlite3 *, sqlite3_stmt *, int *);
void getDistances(NodeStuff, double, double);
int cmpfunc (const void *, const void *);

/* Global Variables */
int rtreeDepth;
int nodecount = 0;
NodeStuff ActiveBranchList[50000];
currentNearest curNear;


int main(int argc, char **argv) {

	if (argc != 3) {
	  fprintf(stderr, "Usage: %s <x> <y> \n", argv[0]);
	  return FAILURE;
	} 

	sqlite3 *db;
	sqlite3_stmt *stmt;

		int rc;

		/* Open the database */
		rc = sqlite3_open("../assignment2.db", &db);
		if( rc ) {
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
			return FAILURE;
		}

		/* Get the Depth of the R-Tree */
		rc = getDepth(db, stmt, &rtreeDepth);
		if( rc ) {
			fprintf(stderr, "Cannot get depth: %s\n", sqlite3_errmsg(db));
			return FAILURE;
		}

		/* Read the BLOB */
		rc = getBLOBData(db, stmt, ROOT);
		if( rc ) {
			fprintf(stderr, "Cannot read BLOB data: %s\n", sqlite3_errmsg(db));
			return FAILURE;
		}

		// Sample call of function to get mindist and minmaxdist and store it in our
		// currentNearest structure
		getDistances(ActiveBranchList[0], atof(argv[1]), atof(argv[2]));
		printf("id: %d minDist: %lf minMaxDist: %lf\n", curNear.id, curNear.minDist, curNear.minMaxDist);


	return SUCCESS;
}

int getBLOBData(sqlite3 *db, sqlite3_stmt *stmt, int nodeno) {
	int rc;
	char *sql_stmt = 	"SELECT rtreenode(2, data) "
						"FROM poibox_node "
						"WHERE nodeno = 1;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (getBLOBData): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		ParseAndStoreBLOB(sqlite3_column_text(stmt, 0));
	}



	sqlite3_reset(stmt);
	return SUCCESS;
}

void ParseAndStoreBLOB(char *BLOB) {
	BLOB++;
	NodeStuff Node;
	char* sNode;
	sNode = strtok(BLOB, "}");
	
	while(1) {
		sscanf(sNode, "%d %lf %lf %lf %lf",
			&Node.id, &Node.minx, &Node.maxx, &Node.miny, &Node.maxy);

		ActiveBranchList[nodecount] = Node;
		nodecount ++;

		sNode = strtok(NULL, "{");
		sNode = strtok(NULL, "}");

		if( sNode == NULL ) {
			break;
		}
	 }
}

int getDepth(sqlite3 *db, sqlite3_stmt *stmt, int *depth) {
	int rc;
	char *sql_stmt = 	"SELECT rtreedepth(data) "
						"FROM poibox_node "
						"WHERE nodeno=1;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed(get depth): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		*depth = sqlite3_column_int(stmt, 0);
	}
	
	sqlite3_reset(stmt);
	return SUCCESS;
}

void getDistances(NodeStuff node, double x, double y) {
	
	double minDist = INFINITY;
	double minMaxDist = INFINITY;
	/* Used as temp values to find mindist */
	double minx, miny;
	/* Distances from point to vertex */
	double vertex1, vertex2, vertex3, vertex4;

	/* Find if our point is closer to minx or maxx */
	if (abs(x - node.minx) < abs(x - node.maxx)) {
		minx = (x - node.minx);
	} else {
		minx = (x - node.maxx);
	}

	/* Find if our point is closer to miny or maxy */
	if (abs(y - node.miny) < abs(y - node.maxy)) {
		miny = (y - node.miny);
	} else {
		miny = (y - node.maxy);
	}

	/* Check to see if our point is in out MBR */
	if (((x <= node.maxx) && (x >= node.minx)) && ((y <= node.maxy) && (y >= node.miny))) {
		minx = 0;
		miny = 0;
	} 

	/* MinDist calculations given our node and x1, y1 from input */
	minDist = pow(minx, 2) + pow(miny, 2);

	/* After extensive testing MinMaxDist will always be the second closest vertice of the MBR*/
	vertex1 = pow((x - node.minx), 2) + pow((y - node.miny), 2);
	vertex2 = pow((x - node.minx), 2) + pow((y - node.maxy), 2);
	vertex3 = pow((x - node.maxx), 2) + pow((y - node.maxy), 2);
	vertex4 = pow((x - node.maxx), 2) + pow((y - node.miny), 2);

	double vertices[] = {vertex1, vertex2, vertex3, vertex4};
	qsort(vertices, 4, sizeof(double), cmpfunc);

	minMaxDist = vertices[1];

	curNear.id = node.id;
	curNear.minDist = minDist;
	curNear.minMaxDist = minMaxDist;

}

int cmpfunc (const void * a, const void * b)
{
   return ( *(double*)a - *(double*)b );
}
