#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#include <math.h>

#define PACKTET_LENGTH  100
#define MAC_ADD_LEN        6
#define MAC_TABLE_SIZE     100
#define SWITCHSIM_SUCCESS  0
#define SWITCHSIM_FAILURE  -1



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

struct macTable
{
  unsigned int ifIndex;
  unsigned int vid;
  unsigned char macAdd[MAC_ADD_LEN];
};

struct hashArray
{
  unsigned int key;
  void *data;
};

swSimIntf  *ifLookUpByName(char *ifName);

memPool* createMemPool(int size);

int processMemPoolData(memPool *mPoolPtr);

memPool *insertMemPoolData(memPool *mPoolPtr, int size, void *data);

int engineProcess(msgBuffer *pktData);

void * switchSimMalloc (unsigned long size);

int switchSimMacTableInit ();

unsigned int switchSimKeyGen (unsigned int ifIndex, char *mac,
                              unsigned int *index, unsigned int vid);

int isMacSame (char *mac1, char *mac2);

struct macTable * switchSimMacTableLookup (unsigned int ifIndex, unsigned int vid,
                                           char *mac);

int macTableEntryAdd (unsigned int ifIndex, char *mac,
                      unsigned int vid);

int macTableEntryDel (unsigned int ifIndex, char *mac,
                      unsigned int vid);

int showMacTable ();
