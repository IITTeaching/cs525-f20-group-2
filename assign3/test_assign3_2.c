/*
	Menu based testing
	1. create schema
			enter number of columns
			enter column names (delimiter = ',')
			enter data types of columns in sequence as of columns (delimiter = ',')
			enter lengths for each data type  (delimiter = ',')
			enter number of pk columns
			enter pk column names
		or use existing schema
		show schema name, attribute names, types, lengths and pk
		
	2. create table
			enter table name
			enter schema name to be used for table
		or use existing table
		show table name and schema name.
		
	3. view table (get RID as well for further reference)
		enter table name
			condition 1=1
			and number of records. all records.
			
		need to handle if two tables are viewed one after another. then need to shutdown pool and start again with new table name
		
	4. insert record
		enter table name
		enter record details (delimiter = ',')		
		show RID after inserting record
				
	5. update record
		enter table name		
		enter rid of record
		enter record details (delimieter = ',')		
		
	6. delete record
		enter table name
		enter rid of record	
	
	7. update column in record
		enter table name
		enter rid
		enter column name
		enter column new value
		
	8. get specific column value from record 
		enter table name
		enter rid
		enter column name to retrieve value	
	
	9. delete from table
		enter table name
			
	10. drop table
		enter table name
*/

#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"

// test methods
static void createSchemaTest (void);
static void createTableTest (void);
static void viewTableTest (void);
static void insertRecordTest (void);
static void updateRecordTest (void);
static void deleteRecordTest(void);
static void updateColumnTest(void);
static void viewColumnTest(void);
static void deleteTableTest(void);
static void dropTableTest(void);

// main method
int 
main (void) 
{
	char *testName = "Menu based Record Manager Testing";

	int schemaOption=0, tableOption=0, menuOption=0;
	
	
	while(schemaOption!=3)
	{
		printf("\n################ Record Manager Testing ################");
		printf("\n########################################################");
		printf("\n Select a Schema option:");
		printf("\n########################################################");
		printf("\n1. Create Schema");
		printf("\n2. Use existing Schema");
		printf("\n3. Exit from program");
		printf("\n########################################################\n");
	
		scanf("%d",&schemaOption);
	
		while(tableOption!=3 && schemaOption!=3)
		{
			printf("\n########################################################");
			printf("\n Select a Table option:");
			printf("\n########################################################");
			printf("\n1. Create Table");
			printf("\n2. Use existing Table");
			printf("\n3. Exit to main menu");
			printf("\n########################################################\n");

			scanf("%d",&tableOption);
			
			while(menuOption!=11 && tableOption!=3)
			{
				printf("\n########################################################");
				printf("\n Select an option for operations:");
				printf("\n########################################################");
				printf("\n1. View Table Data");
				printf("\n2. Insert Record");
				printf("\n3. Update Record");
				printf("\n4. Delete Record");
				printf("\n5. Update Column");
				printf("\n6. View Column");
				printf("\n7. Delete Table");
				printf("\n8. Drop Table");
				printf("\n9. Back To Table Selection");
				printf("\n10. Back To Schema Selection");
				printf("\n11. Exit From Program");
				printf("\n########################################################\n");
				scanf("%d",&menuOption);
			}
		}
	}

	return 0;
}