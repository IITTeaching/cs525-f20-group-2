CC=gcc  #compiler
TARGET1=test_assign1 #target file name
TARGET2=test_assign2 #additional test cases target file name

all:
	$(CC) storage_mgr.c dberror.c test_assign1_1.c -o $(TARGET1)
	$(CC) storage_mgr.c dberror.c test_assign1_2.c -o $(TARGET2)

clean:
	rm $(TARGET1) $(TARGET2) *.bin output_test1 output_test2 

run_test1:
	./test_assign1 > output_test1

run_test2:
	./test_assign2 > output_test2
