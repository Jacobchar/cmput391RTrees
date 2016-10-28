

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

/* Function Prototypes */
void ParseAndStoreBLOB(char *);
int getBLOBData(sqlite3 *, sqlite3_stmt *, int);
int getDepth(sqlite3 *, sqlite3_stmt *, int *);

/* Structure Declarations */
typedef struct {
	int id;
	double minx;
	double maxx;
	double miny;
	double maxy;
} NodeStuff; 


/* Global Variables */
int rtreedepth;
int nodecount = 0;
NodeStuff ActiveBranchList[50000];


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



	return 0;
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
	
	 int i;
	for(i = 0; i < nodecount; i++) {
		printf("%d, %f, %f, %f, %f \n", ActiveBranchList[i].id,ActiveBranchList[i].minx,ActiveBranchList[i].maxx,ActiveBranchList[i].miny,ActiveBranchList[i].maxy);
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
	
	sqlite3_reset(stmt)) 
	return SUCCESS;
}