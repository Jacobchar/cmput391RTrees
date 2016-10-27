#include <stdio.h>
#include <stdlib.h>
#include "../sqlite3.h"

/* Define Statements */

/* Structs */

/* Function Prototypes */

int main(int argc, char **argv) {

	if(argc != 3) {
		fprintf(stderr, "Usage: %s <x> <y>\n", argv[0]);
		return 1;
	}

	sqlite3 *db;
	sqlite3_stmt *stmt;

		int rc;

		/* Open the database */
		rc = sqlite3_open("../assignment2.db", &db);
		if(rc) {
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
			return 1;
		}

	return 0;
}