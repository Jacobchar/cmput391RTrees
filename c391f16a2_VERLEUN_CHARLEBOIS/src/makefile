CC = gcc
CFLAGS = -g -o
CLFLAGS = -lpthread -ldl -lm -DSQLITE_ENABLE_RTREE


default: q4 q5 q7 q8

#q2: q2.c
#	@$(CC) $(CFLAGS) q2 q2.c sqlite3.c $(CLFLAGS)


#q3: q3.c
#	gcc -g -o q3 q3.c sqlite3.c -lpthread -ldl


q4: q4.c
	@$(CC) $(CFLAGS) q4 q4.c sqlite3.c $(CLFLAGS)


q5: q5.c
	@$(CC) $(CFLAGS) q5 q5.c sqlite3.c $(CLFLAGS)


#q6: q6.c
#	@$(CC) $(CFLAGS) q6 q6.c sqlite3.c $(CLFLAGS)


q7: q7.c
	@$(CC) $(CFLAGS) q7 q7.c sqlite3.c $(CLFLAGS)


q8: q8.c
	@$(CC) $(CFLAGS) q8 q8.c sqlite3.c $(CLFLAGS)


#q9: q9.c
#	@$(CC) $(CFLAGS) q9 q9.c sqlite3.c $(CLFLAGS)

clean: 
	@rm q4 q5 q7 q8

