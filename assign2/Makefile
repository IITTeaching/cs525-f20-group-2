CC=gcc  #compiler
TARGET1=test_assign2_1 #target file name
TARGET2=test_assign2_2

all:
	$(CC) storage_mgr.c buffer_mgr_stat.c buffer_mgr.c dberror.c test_assign2_1.c -o $(TARGET1)
	$(CC) storage_mgr.c buffer_mgr_stat.c buffer_mgr.c dberror.c test_assign2_2.c -o $(TARGET2)

clean:
	rm $(TARGET1) $(TARGET2) *.bin *.o *.txt

run_test1:
	./test_assign2_1 > output1.txt

run_test2:
	./test_assign2_2 > output2.txt
