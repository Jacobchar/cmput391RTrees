

default: clean q7

q4:
	gcc -g -o q4 q4.c sqlite3.c -DSQLITE_ENABLE_RTREE -lpthread -ldl -lm

q5:
	gcc -g -o q5 q5.c sqlite3.c -DSQLITE_ENABLE_RTREE -lpthread -ldl -lm

q7:
	gcc -g -o q7 q7.c sqlite3.c -DSQLITE_ENABLE_RTREE -lpthread -ldl -lm
clean:
	rm q7