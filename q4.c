#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

/* Define Statements */
#define SUCCESS 0
#define FAILURE 1

int main(int argc, char ** argv) {

	sqlite3 *db;
	sqlite3_stmt *stmt;

		int rc;
		int i;

		if(argc != 6) {
			fprintf(stderr, "Usage: %s x1, y1, x2, y2, POI_Class\n", argv[0]);
			return FAILURE;
		}

		rc = sqlite3_open("assignment2.db", &db);
		if( rc ) {
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
			return FAILURE;
		}

		char *sql_stmt = 	"WITH temp(id) AS ( "
								"SELECT id "
								"FROM poiboxes b"
								"WHERE b.minx >= ? AND "
								"b.maxx <= ? AND "
								"b.miny >= ? AND "
								"b.maxy <= ?) "
							"SELECT id "
							"FROM poi_tags p JOIN temp t on p.id = t.id "
							"WHERE p.value = ?;";

		rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return FAILURE;
		}

		for(i = 1; i < argc-1; i++) {
			sqlite3_bind_double(stmt, i, argv[i]);
		}
		sqlite3_bind_text(stmt, i+1, argv[i+1]);

		while(sqlite3_step(stmt) == SQLITE3_ROW) {
			printf("ID: %s\n", sqlite3_column_text(0));
		}

		sqlite3_finalize(stmt);

	return 0;
}