#include "swLib.h"

swSimIntf  *swSimIfList[10];

swSimIntf  *ifLookUpByName(char *ifName)
{
	int i, numPorts = 2;
	swSimIntf *swSimIf; 

	for (i = 0; i < numPorts; i++)
	{
		swSimIf = swSimIfList[i];

		if(!swSimIf)
		{
			continue;
		}
		
		if( !strcmp(swSimIf->ifName,ifName))
		{
#ifdef DEBUG
			printf("%s(): Interface Found for ifname = %s \n", __FUNCTION__, ifName);
#endif 
			return swSimIf;
		}
	}
	
	return NULL;
}

memPool* createMemPool(int size)
{
	memPool *mPoolPtr = NULL;
	char  *memBuffer = NULL;	

	memBuffer = (char *) malloc(size);
	mPoolPtr  = (memPool *) malloc(sizeof(memPool));

	if ((memBuffer == NULL) || (mPoolPtr == NULL))
	{
		printf("%s() Failed to create memory pool with size = %d\n",
				__FUNCTION__, size);
		return NULL;
	}

	mPoolPtr->mPoolInUseCount = 0;
	mPoolPtr->mPoolCurrentAddr = (msgBuffer *) memBuffer;
	mPoolPtr->mPoolStartAddr = (msgBuffer *) memBuffer; 
	mPoolPtr->mPoolNextAvailAddr = (msgBuffer *) memBuffer; 
	mPoolPtr->mPoolEndAddr  = (msgBuffer *) memBuffer+size-1;
#ifdef DEBUG
	printf("%s() Created memory pool with size = %d, memBuff = %p , mPoolPtr = %p \n",__FUNCTION__, size, memBuffer, mPoolPtr);

	printf("%s() mPoolStartAddr = %p, mPoolNextAvailAddr = %p, mPoolEndAddr = %p\n",
			__FUNCTION__, mPoolPtr->mPoolStartAddr, mPoolPtr->mPoolNextAvailAddr, mPoolPtr->mPoolEndAddr);
#endif /*DEBUG*/
	return mPoolPtr;
}

memPool *insertMemPoolData(memPool *mPoolPtr, int size, void *data)
{
	msgBuffer *msgData = (msgBuffer *) mPoolPtr->mPoolNextAvailAddr;

#ifdef DEBUG
	printf("%s() mPool = %p \n", __FUNCTION__, mPoolPtr);
	printf("mPoolStartAddr = %p, mPoolNextAvailAddr = %p, mPoolEndAddr = %p\n",
			mPoolPtr->mPoolStartAddr, mPoolPtr->mPoolNextAvailAddr, mPoolPtr->mPoolEndAddr);

	printf("%s: 1: mPoolNextAvailAddr = %p\n", __FUNCTION__, mPoolPtr->mPoolNextAvailAddr);
#endif

	if ((mPoolPtr->mPoolNextAvailAddr + size) >= mPoolPtr->mPoolEndAddr)
	{
		if (mPoolPtr->mPoolCurrentAddr == mPoolPtr->mPoolStartAddr)
		{
#ifdef DEBUG
			printf("%s(): Buffer full, can't insert data \n", __FUNCTION__);
#endif 
			return NULL;
		}
#ifdef DEBUG
	printf("%s: mPoolNextAvailAddr = %p is reset to start address = %p\n", 
			__FUNCTION__, mPoolPtr->mPoolNextAvailAddr, mPoolPtr->mPoolStartAddr);
#endif 
		mPoolPtr->mPoolNextAvailAddr = mPoolPtr->mPoolStartAddr;
	}
 
	memcpy(msgData->data, data, size);
	msgData->size = size;
	mPoolPtr->mPoolInUseCount++;
	mPoolPtr->mPoolNextAvailAddr++;

#ifdef DEBUG
	printf("%s: 2: mPoolNextAvailAddr = %p\n", __FUNCTION__, mPoolPtr->mPoolNextAvailAddr);

	printf("%s() mPoolStartAddr = %p, mPoolNextAvailAddr = %p, mPoolEndAddr = %p\n",
			__FUNCTION__, mPoolPtr->mPoolStartAddr, mPoolPtr->mPoolNextAvailAddr, mPoolPtr->mPoolEndAddr);
#endif 
} 			

int l2EngineProcess(msgBuffer *pktData)
{
	return 0;
}

int engineProcess(msgBuffer *pktData)
{
	l2EngineProcess(pktData);
	
	return 0;
}

int processMemPoolData(memPool *mPoolPtr)
{
	if (mPoolPtr->mPoolInUseCount == 0)
	{
#ifdef DEBUG
		printf("%s(): No packets to process \n", __FUNCTION__);
#endif 
		return 0;
	}

	do
	{	
		engineProcess(mPoolPtr->mPoolCurrentAddr);
	} while (mPoolPtr->mPoolInUseCount--);

	
} 			

