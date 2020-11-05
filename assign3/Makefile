CC=gcc  #compiler
TARGET1=test_assign3_1 #target file name
TARGET2=test_assign3_2 #additional test cases target file name

all:
<<<<<<< HEAD
	$(CC) record_mgr.c expr.c rm_serializer.c storage_mgr.c dberror.c buffer_mgr.c buffer_mgr_stat.c test_assign3_1.c -g -o $(TARGET1) -lm
	$(CC) record_mgr.c rm_serializer.c expr.c storage_mgr.c dberror.c buffer_mgr.c buffer_mgr_stat.c test_assign3_2.c -g -o $(TARGET2) -lm

clean:
	rm $(TARGET1) $(TARGET2) test_table_t test_table_r output_test1 output_test2
=======
	$(CC) record_mgr.c expr.c rm_serializer.c storage_mgr.c dberror.c buffer_mgr.c buffer_mgr_stat.c test_assign3_1.c -lm -g -o $(TARGET1) -lm
	$(CC) record_mgr.c rm_serializer.c expr.c storage_mgr.c dberror.c buffer_mgr.c buffer_mgr_stat.c test_assign3_2.c -lm -g -o $(TARGET2)

clean:
	rm $(TARGET1) $(TARGET2) *.bin output_test1 output_test2 test_table_t test_table_r
>>>>>>> c1deaebe019dc368056670e6a0d0c66ab270eccc

run_test1:
	./test_assign3_1 > output_test1

run_test2:
	./test_assign3_2 > output_test2
