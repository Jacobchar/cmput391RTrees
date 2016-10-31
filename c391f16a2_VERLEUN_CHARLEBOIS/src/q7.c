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
	double minDist;
	double minMaxDist;
	int depth;
} NodeStuff; 

typedef struct {
	int id;
	double dist;
} Nearest;

/* Function Prototypes */
int generateBranchList(sqlite3 *, sqlite3_stmt *, int, NodeStuff*, int);
int Parse_StoreBLOBData(char *, NodeStuff*, int);
int getDepth(sqlite3 *, sqlite3_stmt *, int *);
void getDistances(NodeStuff *, double, double);

void nearestNeighbourSearch(NodeStuff*, int);
int getLeafCount(sqlite3 *, sqlite3_stmt *, NodeStuff *);
void getLeaves(sqlite3 *, sqlite3_stmt *, int, int *, NodeStuff *);
double leafDist(sqlite3 *, sqlite3_stmt *, int);
void sortBranchList(NodeStuff *, int);
int downpruneBranchList(NodeStuff *, int);
int uppruneBranchList(NodeStuff *, int);

void quickSort(NodeStuff *, int, int);
int partition(NodeStuff *, int, int);
int cmpfunc (const void *, const void *);
void printBranchList(NodeStuff*, int);

/* Global Variables */
double x, y;
int rtreeDepth;
Nearest nearest;
sqlite3 *db;
sqlite3_stmt *stmt;


int main(int argc, char **argv) {

	if (argc != 3) {
	  fprintf(stderr, "Usage: %s <x> <y> \n", argv[0]);
	  return FAILURE;
	} 

		int rc;
		x = atof(argv[1]);
		y = atof(argv[2]);

		NodeStuff root;
		root.id = 1;
		
		nearest.dist = INFINITY;

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

		nearestNeighbourSearch(&root, 0);

		printf("Nearest: %d\n", nearest.id);

	return SUCCESS;
}

int generateBranchList(sqlite3 *db, sqlite3_stmt *stmt, int nodeno, NodeStuff *ABL, int depth) {
	int rc;
	int index;
	char *sql_stmt = 	"SELECT rtreenode(2, data) "
						"FROM poibox_node "
						"WHERE nodeno = ?;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (generateBranchList): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_bind_int(stmt, 1, nodeno);

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		index = Parse_StoreBLOBData(sqlite3_column_text(stmt, 0), ABL, depth);
	}

	sqlite3_clear_bindings(stmt);
	sqlite3_reset(stmt);

	return index;
}

int Parse_StoreBLOBData(char *BLOB, NodeStuff *ABL, int depth) {
	BLOB++;
	NodeStuff Node;
	char* sNode;
	int index = 0;
	sNode = strtok(BLOB, "}");	
	while(1) {
		sscanf(sNode, "%d %lf %lf %lf %lf",
			&Node.id, &Node.minx, &Node.maxx, &Node.miny, &Node.maxy);

		getDistances(&Node, x, y);
		Node.depth = depth;

		ABL[index] = Node;
		index++;

		sNode = strtok(NULL, "{");
		sNode = strtok(NULL, "}");

		if( sNode == NULL ) {
			return index;
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

void getDistances(NodeStuff *node, double x, double y) {
	
	double minDist = INFINITY;
	double minMaxDist = INFINITY;
	
	/* Used as temp values to find mindist */
	double minx, miny;
	
	/* Distances from point to vertex */
	double vertex1, vertex2, vertex3, vertex4;

	/* Find if our point is closer to minx or maxx */
	if (abs(x - node->minx) < abs(x - node->maxx)) {
		minx = (x - node->minx);
	} else {
		minx = (x - node->maxx);
	}

	/* Find if our point is closer to miny or maxy */
	if (abs(y - node->miny) < abs(y - node->maxy)) {
		miny = (y - node->miny);
	} else {
		miny = (y - node->maxy);
	}

	/* Check to see if our point is in out MBR */
	if (((x <= node->maxx) && (x >= node->minx)) && ((y <= node->maxy) && (y >= node->miny))) {
		minx = 0;
		miny = 0;
	} 

	/* MinDist calculations given our node and x1, y1 from input */
	minDist = pow(minx, 2) + pow(miny, 2);

	/* After extensive testing MinMaxDist will always be the second closest vertice of the MBR*/
	vertex1 = pow((x - node->minx), 2) + pow((y - node->miny), 2);
	vertex2 = pow((x - node->minx), 2) + pow((y - node->maxy), 2);
	vertex3 = pow((x - node->maxx), 2) + pow((y - node->maxy), 2);
	vertex4 = pow((x - node->maxx), 2) + pow((y - node->miny), 2);

	double vertices[] = {vertex1, vertex2, vertex3, vertex4};
	qsort(vertices, 4, sizeof(double), cmpfunc);

	minMaxDist = vertices[1];

	node->minDist = minDist;
	node->minMaxDist = minMaxDist;
}

void nearestNeighbourSearch(NodeStuff *Node, int depth){
	
	NodeStuff newNode;
	int i, last;
	double dist;
	if(depth == rtreeDepth) {	
		int leafCount = getLeafCount(db, stmt, Node);
		int *leaves = malloc(sizeof(int) * leafCount);
		getLeaves(db, stmt, leafCount, leaves, Node);

		for(i = 0; i < leafCount; i++) {
			dist = leafDist(db, stmt, leaves[i]);
			if(dist < nearest.dist) {
				nearest.dist = dist;
				nearest.id = leaves[i];
			}
		}
		free(leaves);
	} else {

		NodeStuff *ActiveBranchList = malloc(sizeof(NodeStuff) * 1000);

		last = generateBranchList(db, stmt, Node->id, ActiveBranchList, depth);
		
		sortBranchList(ActiveBranchList, last);
		
		last = downpruneBranchList(ActiveBranchList, last);

		for(i = 0; i < last; i++) {
			newNode = ActiveBranchList[i];
			nearestNeighbourSearch(&newNode, depth+1);
			last = uppruneBranchList(ActiveBranchList, last);
		}
		free(ActiveBranchList);
	}	
}

int getLeafCount(sqlite3 *db, sqlite3_stmt *stmt, NodeStuff *Node) {
	int rc;
	int count;

	char *sql_stmt = 	"SELECT COUNT(*) "
						"FROM poibox_rowid "
						"WHERE nodeno = ?;";

	
	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (getLeafCount): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_bind_int(stmt, 1, Node->id);

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		count = sqlite3_column_int(stmt, 0);
	}

	sqlite3_clear_bindings(stmt);	
	sqlite3_reset(stmt);

	return count;
}

void getLeaves(sqlite3 *db, sqlite3_stmt *stmt, int leafCount, int *leaves, NodeStuff *Node) {
	int rc, i;

	char * sql_stmt = 	"SELECT rowid "
						"FROM poibox_rowid "
						"WHERE nodeno = ?;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (getLeaves): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	sqlite3_bind_int(stmt, 1, Node->id);

	for(i = 0; i < leafCount; i++) {
		if(sqlite3_step(stmt) == SQLITE_ROW) {
			leaves[i] = sqlite3_column_int(stmt, 0);
		}
	}

	sqlite3_clear_bindings(stmt);
	sqlite3_reset(stmt);
}

double leafDist(sqlite3 *db, sqlite3_stmt *stmt, int leaf_id) {

	int rc;

	char * sql_stmt = 	"SELECT minx, maxx, miny, maxy "
						"FROM poibox "
						"WHERE id = ?;";

	rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Preparation failed (objectDist): %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return FAILURE;
	}

	sqlite3_bind_int(stmt, 1, leaf_id);


	double x1, x2, y1, y2;
	 if(sqlite3_step(stmt) == SQLITE_ROW) {
	 	x1 = sqlite3_column_double(stmt, 0);
	 	x2 = sqlite3_column_double(stmt, 1);
	 	y1 = sqlite3_column_double(stmt, 2);
	 	y2 = sqlite3_column_double(stmt, 3);
	 }

	 sqlite3_clear_bindings(stmt);
	 sqlite3_reset(stmt);


	 //calc distance to the top left corner
	 return pow((x - x1), 2) + pow((y - y1), 2);

}

void sortBranchList(NodeStuff *ABL, int end) {
	quickSort(ABL, 0, end - 1);	
}

int downpruneBranchList(NodeStuff *ABL, int last_index) {
	
	int i;
	double minMaxDist = INFINITY;
	for(i = 0; i< last_index; i++) {
		if(ABL[i].minMaxDist < minMaxDist) {
			minMaxDist = ABL[i].minMaxDist;
		}
	}

	for(i = 0; i < last_index; i++) {
		if(ABL[i].minDist > minMaxDist) {
			return i;
		}
	}
	return 0;
}

int uppruneBranchList(NodeStuff *ABL, int last_index) {

	int i;
	for(i = 0; i < last_index; i++) {
		if(ABL[i].minDist > nearest.dist) {
			return i;
		}
	}
	return 0;
}

void quickSort(NodeStuff* ABL, int l, int r) {
	int j;

	if(l < r) {
		j = partition(ABL, l, r);
		quickSort(ABL, l, j-1);
		quickSort(ABL, j+1, r);
	}	
}

int partition(NodeStuff* ABL, int l, int r) {
	int i, j;
	double pivot;
	NodeStuff temp;
	pivot = ABL[l].minDist;
	i = l;
	j = r+1;

	while(1) {
		do ++i; while( ABL[i].minDist <= pivot && i <= r );
   		do --j; while( ABL[j].minDist > pivot );
   		if( i >= j) break;
   		temp = ABL[i]; 
   		ABL[i] = ABL[j]; 
   		ABL[j] = temp;
   	}
   	temp = ABL[l];
   	ABL[l] = ABL[j];
   	ABL[j] = temp;
   	return j;
}

int cmpfunc (const void * a, const void * b)
{
   return ( *(double*)a - *(double*)b );
}

void printBranchList(NodeStuff *ABL, int index) {
	int i = 0;
	for(i = 0; i < index; i++) {
		printf("%d - %f - %f\n", ABL[i].id, ABL[i].minDist, ABL[i].minMaxDist);
	}
	printf("-----------------------\n");
}