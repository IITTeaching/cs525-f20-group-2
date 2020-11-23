CC=gcc  #compiler
TARGET1=test_assign4_1 #target file name

all:
	$(CC) btree_mgr.c record_mgr.c expr.c rm_serializer.c storage_mgr.c dberror.c buffer_mgr.c buffer_mgr_stat.c test_assign4_1.c -g -o $(TARGET1) -lm
	
clean:
	rm $(TARGET1) testidx

run_test1:
	./test_assign4_1 > output_test1
