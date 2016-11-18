#include "swLib.h"

typedef struct test
{
	int data;
	char packet[1000];
}test;

int main()
{
	memPool *myPool;
	test   myTest;
    int i;

	myTest.data = 100;

	for(i=0; i < 1000; i++)
		myTest.packet[i] = i;

	myPool = createMemPool(10000);
	
	printf("%s: myPool = %p \n", __FUNCTION__, myPool);

	if (!myPool)
		printf("Error in creating memory pool \n");

	insertMemPoolData(myPool, sizeof(test), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	insertMemPoolData(myPool, sizeof(myTest), &myTest, 0, 0);
	
	printf("%s: mPoolStartAddr = %p, mPoolNextAvailAddr = %p, mPoolEndAddr = %p UseCount = %d \n", 
			__FUNCTION__, myPool->mPoolStartAddr, myPool->mPoolNextAvailAddr, myPool->mPoolEndAddr, 
			myPool->mPoolInUseCount);
	
	return 0;
}

