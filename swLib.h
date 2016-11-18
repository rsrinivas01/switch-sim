#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>

#define PACKTET_LENGTH  100


typedef struct ethHdr
{
	char dstMac[6];
	char srcMac[6];
	char ethType[4];
	char ethTypeLen[2];
}ethHdr;

typedef struct msgBuffer
{
	int  size;
	char data[PACKTET_LENGTH];
} msgBuffer;

typedef struct memPool
{
	int  mPoolInUseCount;
    msgBuffer *mPoolStartAddr;
    msgBuffer *mPoolCurrentAddr;
    msgBuffer *mPoolNextAvailAddr;
    msgBuffer *mPoolEndAddr;
}memPool;

typedef struct swSimIntfStats
{
	int pktReceived;
	int pktForwarded;
	int pktDiscarded;
	int pktDropped;
}swSimIntfStats;

typedef struct swSimIntf
{
	char ifName[30];
	char ingressMsgQ[30];
	char egressMsgQ[30];
	int  ifIndex;
	mqd_t ingMsgQId;
	mqd_t egrMsgQId;
	memPool *ingPktBuf;
	memPool *egrPktBuf;
	swSimIntfStats	ifStats;
}swSimIntf;

swSimIntf  *ifLookUpByName(char *ifName);
memPool* createMemPool(int size);
int processMemPoolData(memPool *mPoolPtr);
memPool *insertMemPoolData(memPool *mPoolPtr, int size, void *data);
int engineProcess(msgBuffer *pktData);
