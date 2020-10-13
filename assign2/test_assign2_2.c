#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content 
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)			        \
  do {									\
    char *real;								\
    char *_exp = (char *) (expected);                                   \
    real = sprintPoolContent(bm);					\
    if (strcmp((_exp),real) != 0)					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
	free(real);							\
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
    free(real);								\
  } while(0)

// test and helper methods
static void testCreatingAndReadingDummyPages (void);
static void createDummyPages(BM_BufferPool *bm, int num);
static void checkDummyPages(BM_BufferPool *bm, int num);
static void testReadPage (void);
static void testClock (void);

// main method
int 
main (void) 
{
  initStorageManager();
  testName = "Additional Testing for Clock and GClock";

  testCreatingAndReadingDummyPages();
  testReadPage();
  testClock();
  return 0;
}

// create n pages with content "Page X" and read them back to check whether the content is right
void
testCreatingAndReadingDummyPages (void)
{
  BM_BufferPool *bm = MAKE_POOL();
  testName = "Creating and Reading Back Dummy Pages";

  CHECK(createPageFile("testClockbuffer.bin"));

  createDummyPages(bm, 22);
  checkDummyPages(bm, 20);

  createDummyPages(bm, 10000);
  checkDummyPages(bm, 10000);

  CHECK(destroyPageFile("testClockbuffer.bin"));

  free(bm);
  TEST_DONE();
}

void 
createDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();

  CHECK(initBufferPool(bm, "testClockbuffer.bin", 3, RS_CLOCK, NULL));
  
  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));
      sprintf(h->data, "%s-%i", "Page", h->pageNum);
      CHECK(markDirty(bm, h));
      CHECK(unpinPage(bm,h));
    }
  CHECK(shutdownBufferPool(bm));

  free(h);
}

void 
checkDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  char *expected = malloc(sizeof(char) * 512);
  
  CHECK(initBufferPool(bm, "testClockbuffer.bin", 3, RS_CLOCK, NULL));
  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));
      sprintf(expected, "%s-%i", "Page", h->pageNum);
      ASSERT_EQUALS_STRING(expected, h->data, "reading back dummy page content");

      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(expected);
  free(h);
}

void
testReadPage ()
{
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Reading a page";

  CHECK(createPageFile("testClockbuffer.bin"));
  CHECK(initBufferPool(bm, "testClockbuffer.bin", 3, RS_CLOCK, NULL));
  
  CHECK(pinPage(bm, h, 0));
  CHECK(pinPage(bm, h, 0));

  CHECK(markDirty(bm, h));

  CHECK(unpinPage(bm,h));
  CHECK(unpinPage(bm,h));

  CHECK(forcePage(bm, h));

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testClockbuffer.bin"));

  free(bm);
  free(h);

  TEST_DONE();
}

void
testClock(void)
{
  // expected results
	const char *poolContents[] = { 
			"[0 0],[-1 0],[-1 0]", // request Page-0 - 1 read
			"[0 0],[4 0],[-1 0]", // request Page-4 - 2 read
			"[0 0],[4 0],[1x0]", // request Page-1 - 3 read
			//////////////////////////
			"[0 0],[4 1],[1x0]", // request Page-4
			"[2 0],[4 1],[1x0]", // request Page-2 - 4 read
			"[2 0],[4 1],[1x0]", // request Page-4 - 1 write
			//////////////////////////	
			"[2 0],[4 0],[3x0]", // request Page-3 - 5 read
			"[2 0],[4 1],[3x0]", // request Page-4
			"[2 1],[4 1],[3x0]", // request Page-2
			//////////////////////////	
			"[2 1],[4 1],[3x0]", // request Page-4 - 2 write
			"[2 0],[4 0],[0 0]", // request Page-0 - 6 read
			"[2 0],[4 1],[0 0]", // request Page-4
			//////////////////////////	
			"[1 0],[4 1],[0 0]", // request Page-1 - 7 read
			"[1 0],[4 1],[0 0]", // request Page-4
			"[1 0],[4 0],[2 0]", // request Page-2 - 8 read
			//////////////////////////	
			"[1 0],[4x1],[2 0]", // request Page-4
			"[3x0],[4x1],[2 0]", // request Page-3 - 9 read
			"[3x0],[4x1],[2 0]", // request Page-4 
			"[3 0],[4 1],[2 0]" // forceFlushPool() - 4 write
			};

	const int requests[] = { 0, 4, 1, 4, 2, 4, 3, 4, 2, 4, 0, 4, 1, 4, 2, 4, 3, 4 };
	const int numLinRequests = 3;
	const int numChangeRequests = 3;

	int i;
	BM_BufferPool *bm = MAKE_POOL();
	BM_PageHandle *h = MAKE_PAGE_HANDLE();
	testName = "Testing CLOCK page replacement";

  CHECK(createPageFile("testClockbuffer.bin"));
  createDummyPages(bm, 100);
  CHECK(initBufferPool(bm, "testClockbuffer.bin", 3, RS_CLOCK, NULL));

	// Read first set of 3 pages
	for (i = 0; i < numLinRequests; i++) {
		pinPage(bm, h, requests[i]);
		if(requests[i] == 1)
			 markDirty(bm,h);
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}
	
	// Read second set of 3 pages
	for (i = numLinRequests; i < (numLinRequests*2); i++) {
		pinPage(bm, h, requests[i]);
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}
	
	// Read third set of 3 pages
	for (i = (numLinRequests*2); i < (numLinRequests*3); i++) {
		pinPage(bm, h, requests[i]);
		if(requests[i] == 3)
			 markDirty(bm,h);
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}
	
	// Read fourth set of 3 pages
	for (i = (numLinRequests*3); i < (numLinRequests*4); i++) {
		pinPage(bm, h, requests[i]);	 
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}
	
	// Read fifth set of 3 pages
	for (i = (numLinRequests*4); i < (numLinRequests*5); i++) {
		pinPage(bm, h, requests[i]);
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}
	
	// Read sixth set of 3 pages
	for (i = (numLinRequests*5); i < (numLinRequests*6); i++) {
		pinPage(bm, h, requests[i]);
		markDirty(bm,h);
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}

	forceFlushPool(bm);
	ASSERT_EQUALS_POOL(poolContents[i], bm, "pool content after flush");
	
	// check number of write IOs
	ASSERT_EQUALS_INT(4, getNumWriteIO(bm), "check number of write I/Os");
	ASSERT_EQUALS_INT(9, getNumReadIO(bm), "check number of read I/Os");

	CHECK(shutdownBufferPool(bm));
	CHECK(destroyPageFile("testClockbuffer.bin"));

	free(bm);
	free(h);
	TEST_DONE();
}
