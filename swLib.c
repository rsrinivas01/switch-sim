#include "swLib.h"


swSimIntf  *swSimIfList[10];

struct hashArray *hash[MAC_TABLE_SIZE];

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

void *
switchSimMalloc (unsigned long size)
{
  void *ptr;
  ptr = malloc(size);
  if (ptr)
  {
    memset (ptr, 0, size);
    return ptr;
  }
  return NULL;
}

int
switchSimMacTableInit ()
{
  int i;

  for (i = 0; i < MAC_TABLE_SIZE; i++)
    hash[i] = (struct hashArray *) switchSimMalloc
                    (sizeof (struct hashArray));
  return SWITCHSIM_SUCCESS;
}

unsigned int
switchSimKeyGen (unsigned int ifIndex, char *mac,
                 unsigned int *index, unsigned int vid)
{
  unsigned int key = 0;

  /* Generate Hash Key and Hash Index */
  key = abs ((unsigned int) (((mac[0] + mac[1] + mac[2] +
                               mac[3] + mac [4] + mac[5]) << 2) + ifIndex + vid));
  *index = (unsigned int )(key % MAC_TABLE_SIZE);

  return key;
}

int
isMacSame (char *mac1, char *mac2)
{
  if (!mac1 || !mac2)
      return SWITCHSIM_FAILURE;

  if (mac1[0] == mac2[0] &&
      mac1[1] == mac2[1] &&
      mac1[2] == mac2[2] &&
      mac1[3] == mac2[3] &&
      mac1[4] == mac2[4] &&
      mac1[5] == mac2[5])
    return SWITCHSIM_SUCCESS;
  else
    return SWITCHSIM_FAILURE;
}

struct macTable *
switchSimMacTableLookup (unsigned int ifIndex, unsigned int vid,
                         char *mac)
{
  unsigned int key = 0;
  unsigned int index = 0;
  struct macTable *entry = NULL;

  /* Get hash key and index */
  key = switchSimKeyGen (ifIndex, mac, &index, vid);

  entry = (struct macTable *) hash[index]->data;

  if (entry)
  {
    if (entry->ifIndex == ifIndex && entry->vid == vid &&
        (isMacSame (entry->macAdd, mac) == SWITCHSIM_SUCCESS))
      return entry;
  }
  return NULL;
}

int
macTableEntryAdd (unsigned int ifIndex, char *mac,
                  unsigned int vid)
{
  struct macTable *entry = NULL;
  unsigned int key = 0;
  unsigned int index = 0;

  entry = switchSimMacTableLookup (ifIndex, vid, mac);
  if (entry)
  {
#ifdef DEBUG
    printf ("\n Duplicate entry not allowed !!! \n");
#endif
    return SWITCHSIM_FAILURE;
  }
  else
  {
    entry = (struct macTable *) switchSimMalloc
                       (sizeof (struct macTable));
    if (!entry)
    {
#ifdef DEBUG
      printf ("\n Memory allocation failed for mac_table_entry. \n");
#endif
      return SWITCHSIM_FAILURE;
    }
    /* Copy data into bucket */
    key = key = switchSimKeyGen (ifIndex, mac, &index, vid);
    entry->ifIndex = ifIndex;
    entry->vid = vid;
    memcpy (entry->macAdd, mac, MAC_ADD_LEN);
    hash[index]->key = key;
    hash[index]->data = (struct hashArray *) entry;
#ifdef DEBUG
    printf ("\n Entry added for ifIndex %d. \n", entry->ifIndex);
#endif
  }
  return SWITCHSIM_SUCCESS;
}

int
macTableEntryDel (unsigned int ifIndex, char *mac,
                  unsigned int vid)
{
  struct macTable *entry = NULL;
  unsigned int key = 0;
  unsigned int index = 0;

  entry = entry = switchSimMacTableLookup (ifIndex, vid, mac);
  if (entry)
  {
    key = switchSimKeyGen (ifIndex, mac, &index, vid);
    hash[index]->key = 0;
    hash[index]->data = NULL;
    free (entry);
#ifdef DEBUG
    printf ("\n Entry deleted for ifIndex %d \n", ifIndex);
#endif
    return SWITCHSIM_SUCCESS;
  }
  else
#ifdef DEBUG
    printf ("\n No such entry found !!! \n");
#endif
  return SWITCHSIM_FAILURE;
}

int
showMacTable ()
{
  struct macTable *entry = NULL;
  int i;
  int flag = 0;

  printf ("\n MAC Table. ");
  printf ("\n HashIndex \t IfIndex \t VID \t MAC-Add");
  printf ("\n ------------------------------------------------- \n");
  for (i = 0; i < MAC_TABLE_SIZE; i++)
  {
    entry = (struct macTable *) hash[i]->data;
    if (!entry)
      continue;

    printf ("\n %d \t \t %d \t \t %d \t %.02x%02x.%02x%02x.%02x%02x", i,
                                                   entry->ifIndex,
                                                   entry->vid,
                                                   entry->macAdd[0],
                                                   entry->macAdd[1],
                                                   entry->macAdd[2],
                                                   entry->macAdd[3],
                                                   entry->macAdd[4],
                                                   entry->macAdd[5]);
    flag = 1;
  }
  printf ("\n");
  if (!flag)
    printf ("\n MAC Table is Empty. \n");
  return SWITCHSIM_SUCCESS;
}
