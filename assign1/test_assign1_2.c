//https://pinetools.com/repeat-text
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "additional_test_pagefile.bin"
#define TESTPF1 "additional_tests_file.bin"

/* file and page handles*/
SM_FileHandle fh;
SM_PageHandle ph;

/* prototypes for test functions */
static void testCreateOpenClose(void);
static void additionalNegativeTestsCreateOpenClose(void);
static void additionalPositiveTestsForWrite(void);
static void additionalPositiveTestsForRead(void);
static void additionalNegativeTestsForReadWrite(void);

/* main function running all tests */
int
main (void)
{
  testName = "Starting with additional test cases";
  
  testCreateOpenClose();
  additionalNegativeTestsCreateOpenClose();
  additionalPositiveTestsForWrite();
  additionalPositiveTestsForRead();
  additionalNegativeTestsForReadWrite();

  return 0;
}

void
testCreateOpenClose(void)
{
  testName = "Creating file for additional test cases:";

  TEST_CHECK(createPageFile (TESTPF));
  
  TEST_CHECK(openPageFile (TESTPF, &fh));

  TEST_DONE();
}

void
additionalNegativeTestsCreateOpenClose(void)
{
  testName = "Additional negative tests for Create Open Close:";

  ASSERT_TRUE((openPageFile (TESTPF1, &fh) != RC_OK), "opening the file before creating should return an error.");

  ASSERT_TRUE((destroyPageFile (TESTPF1) != RC_OK), "destroying non-existing file should return an error.");
  
  ASSERT_TRUE((createPageFile (TESTPF) != RC_OK), "creating a file with same name multiple times should return an error.");

  TEST_DONE();
}

/* Try to write new Block of data at start/end/current position */
void
additionalPositiveTestsForWrite(void)
{ 	
	testName = "Additional positive tests for write:\n";
	
	int i;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);
	
	/*----------- Change ph to be all '1' and write ph to first page or first block ??? of file -----------*/
	for (i=0; i < PAGE_SIZE; i++) 
		ph[i]='1';
	
	TEST_CHECK(writeBlock(0, &fh, ph));

	/*----------- Change ph to be all '2' and write ph to current position of file -----------*/
	for (i=0; i < PAGE_SIZE; i++) 
		ph[i]='2';

  	TEST_CHECK(writeCurrentBlock(&fh, ph));

	/*----------- Change ph to be all '3' and write ph to first page OR second block???? of file -----------*/
	for (i=0; i < PAGE_SIZE; i++) 
		ph[i]='3';

  	// Writing string ph to first page of file.
  	TEST_CHECK(writeBlock(1, &fh, ph));

	/*----------- Change ph to be all '4' and write ph to current position of file -----------*/
	for (i=0; i < PAGE_SIZE; i++) 
		ph[i]='4';
 	// Writing string ph to current position of file.
  	TEST_CHECK(writeBlock(2, &fh, ph))

	/*----------- Change ph to be all '4' and write ph to file -----------*/
	for (i=0; i < PAGE_SIZE; i++) 
		ph[i]='5';
		
	// Writing string ph to fourth page of file. --> PAGE OR BLOCK ????
	TEST_CHECK(writeBlock(3, &fh, ph))

   TEST_DONE();
  
}

/* Try to read first block, last block, current block, next block and previous block */
void
additionalPositiveTestsForRead(void)
{
  int i;
  testName = "Additional positive tests for read:\n";
  
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  TEST_CHECK(openPageFile (TESTPF, &fh));

  /*----------- Read first page into handle. Should read page 1 with all values as '2' -----------*/
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '2'), "Character in page read from disk is the one we expected.");
  
  /*----------- Read next block from current page position. Should read page 2 with all values as '3' -----------*/
  TEST_CHECK(readNextBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '3'), "Character in next page read from disk is the one we expected.");

  /*----------- Read next block from current page position. Should read page 3 with all values as '4' -----------*/
  TEST_CHECK(readNextBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '4'), "Character in next page read from disk is the one we expected.");   
	
  /*----------- Read next block from current page position. Should read page 4 with all values as '5' -----------*/
  TEST_CHECK(readNextBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '5'), "Character in next page read from disk is the one we expected."); 
  
  /*----------- Read current page block. Should read page 4 with all values as '5' -----------*/
  TEST_CHECK(readCurrentBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '5'), "Character in next page read from disk is the one we expected."); 
      
  /*----------- Read previous block from current page position. Should read page 3 with all values as '4' -----------*/
  TEST_CHECK(readPreviousBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '4'), "Character in previous page read from disk is the one we expected.");
  
  /*----------- Read previous block from current page position. Should read page 2 with all values as '3' -----------*/
  TEST_CHECK(readPreviousBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '3'), "Character in previous page read from disk is the one we expected.");
  
  /*----------- Read last page into handle. Should read page 4 with all values as '5' -----------*/
  TEST_CHECK(readLastBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '5'), "Character in page read from disk is the one we expected."); 
  
  /*----------- Read third page -----------*/
  TEST_CHECK(readBlock (2, &fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '4'), "Character in specified page number read from disk is the one we expected.");
    
  TEST_DONE();
}

/* Try to read block before first block, block after last block and invalid file /Write to non-existing file */
void
additionalNegativeTestsForReadWrite(void)
{
  int i;
  
  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  TEST_CHECK(openPageFile (TESTPF, &fh));
  
  testName = "Additional negative tests for read:\n";
  
  /*----------- Read first page into handle -----------*/
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '2'), "Character in first page read from disk is the one we expected.");

  /*----------- Read previous block of first page. Error should be thrown - RC_READ_NON_EXISTING_PAGE -----------*/
  ASSERT_ERROR((readPreviousBlock (&fh, ph)) == RC_READ_NON_EXISTING_PAGE, "Accessing page before first page should return error");
    
  /*----------- Read first page into handle -----------*/
  TEST_CHECK(readLastBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == '5'), "Character in last page read from disk is the one we expected.");

  /*----------- Read next block of last page. Error should be thrown - RC_READ_NON_EXISTING_PAGE -----------*/
  ASSERT_ERROR((readNextBlock (&fh, ph)) == RC_READ_NON_EXISTING_PAGE, "Accessing page after last page should return error");
  
  /*----------- Destroy new page file -----------*/
  TEST_CHECK(destroyPageFile (TESTPF));
  
  /*----------- READ FROM NON EXISTING FILE -----------*/
  ASSERT_ERROR((readFirstBlock (&fh, ph) != RC_OK), "Reading non-existing file should return an error.");
  
  /*----------- Write block to invalid/non-existent file -----------*/
  ASSERT_ERROR((writeBlock(1, &fh, ph) != RC_OK), "Writing to non-existing file should return an error.");
  
  TEST_DONE();
}

