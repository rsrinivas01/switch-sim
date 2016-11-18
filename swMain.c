#include "swLib.h"

extern swSimIntf *swSimIfList[10];


void* packetProcessingThread (void *temp)
{
	int ret;
	int i;
	unsigned char rcvdPacket[PACKTET_LENGTH];
	swSimIntf *swSimIf;

	while(1)
	{
		for (i = 0; i < 10; i++)
		{
			swSimIf = swSimIfList[i];

			if(!swSimIf)
			{
				continue;
			}

			processMemPoolData(swSimIf->ingPktBuf);
		}	
	}
	
}

void* packetReceiveThread (void *ifName)
{
	int ret;
	int i;
	unsigned char rcvdPacket[PACKTET_LENGTH];
	memPool *mPtr;

	swSimIntf  *swSimIf = ifLookUpByName(ifName);

	if(!swSimIf)
	{
		printf("%s() cannot find interface  %s  \n", __FUNCTION__, (char *) ifName);
		exit(0);
	}	

	memset(&swSimIf->ifStats, 0, sizeof(swSimIf->ifStats));

	while(1)
	{
		ret = mq_receive(swSimIf->ingMsgQId, rcvdPacket, PACKTET_LENGTH, NULL);

		if (ret < 0)
		{
			perror("error in mq_receive");
			continue;
		}
#ifdef DEBUG
		printf("%s(): received packet on %s \n", __FUNCTION__, swSimIf->ifName);
#endif 
		swSimIf->ifStats.pktReceived++;

#ifdef DEBUG
		for(i=0; i < PACKTET_LENGTH; i++)
		{
			printf("%x",rcvdPacket[i]);
		}
		printf("\n");
#endif 
		mPtr =  insertMemPoolData(swSimIf->ingPktBuf, PACKTET_LENGTH, rcvdPacket);
		if (mPtr == NULL)
		{
#ifdef DEBUG
			printf("%s() buffer full, packet dropped at ingress for interface %s\n",
					__FUNCTION__, swSimIf->ifName);
#endif 
			swSimIf->ifStats.pktDropped++;
		}
	} 

}

int main(int argc, char *argv[])
{
	int i;
	int userInput;
	int numPorts = 2; 
	pthread_t ppThreadId, prThreadId[10];
	swSimIntf  *swSimIf;
	struct mq_attr swSimMsgQAttr;

    memset(&swSimMsgQAttr, 0,sizeof(struct mq_attr));

    swSimMsgQAttr.mq_flags = 0;
    swSimMsgQAttr.mq_maxmsg = 10;
    swSimMsgQAttr.mq_msgsize = PACKTET_LENGTH;
    swSimMsgQAttr.mq_curmsgs = 0;

	if(argc > 1)
	{
		numPorts = strtol(argv[1], NULL, 0);
		if(numPorts > 10)
		{
			printf("More than %d interfaces is not supported \n", numPorts);
			exit(0);
		}
	}

    printf("argc = %d numPorts = %d \n", argc, numPorts);

 	for (i = 0; i < numPorts; i++)
	{
		swSimIf = malloc(sizeof(swSimIntf));
		if(!swSimIf)
		{
			printf("%s() cannot create sim interface for port %d \n", 
					__FUNCTION__, i);
			exit(0);
		}
	
		sprintf(swSimIf->ifName, "%s%d", "eth", i);
		sprintf(swSimIf->ingressMsgQ, "/%s%d%s", "eth", i, "ingMsgQ");
		sprintf(swSimIf->egressMsgQ, "/%s%d%s", "eth", i, "egrMsgQ");
		swSimIf->ingPktBuf = createMemPool(10000);
		swSimIf->egrPktBuf = createMemPool(10000);
		swSimIf->ifIndex   = i + 1000;

		if ( (!swSimIf->ingPktBuf) || (!swSimIf->egrPktBuf) )
		{
			printf("%s() cannot create ingress/egress buffers for port %d \n",
                    __FUNCTION__, i);
			exit(0);
		}
		memset(&swSimIf->ifStats, 0, sizeof(swSimIntfStats));;

		swSimIf->ingMsgQId = mq_open(swSimIf->ingressMsgQ, 
										O_RDONLY | O_CREAT | O_EXCL, 0644, 
				                     	&swSimMsgQAttr);
		swSimIf->egrMsgQId = mq_open(swSimIf->egressMsgQ, 
										O_WRONLY | O_CREAT | O_EXCL, 0644, 
				                     	&swSimMsgQAttr);

		if ((swSimIf->ingMsgQId < 0) || (swSimIf->egrMsgQId < 0))
		{
			perror("mq_open() failed with error - - ");
			printf("Ingress Message Q Id = %d, Egress Message Q Id = %d \n",
					swSimIf->ingMsgQId, swSimIf->egrMsgQId);
			exit(0);
		}	

		swSimIfList[i] = swSimIf;
		pthread_create(&prThreadId[i], NULL, &packetReceiveThread, swSimIf->ifName);
	}

	pthread_create(&ppThreadId, NULL, &packetProcessingThread, NULL);

	while(1)
	{
		printf("---- Switch Simulator Menu ----\n");
		printf("1. Print Interfaces \n");
		printf("2. Print Interface Stats \n");
		printf("3. Exit Simulator \n");
		scanf("%d", &userInput);

		switch(userInput)
		{
			case 1:
					printf("Interfaces ...\n");
					for (i = 0; i < numPorts; i++)
					{
						swSimIf = swSimIfList[i];

						if(!swSimIf)
						{
							continue;
						}
						printf("--------------\n");
						printf("Interface Name:  %s\n", swSimIf->ifName);
						printf("Interface Index: %d\n", swSimIf->ifIndex);
#ifdef DEBUG
						printf("Ingress Queue:   %s\n", swSimIf->ingressMsgQ);
						printf("Egress Queue:    %s\n", swSimIf->egressMsgQ);
						printf("Ingress Buffer:  %p\n", swSimIf->ingPktBuf);
						printf("Egress Buffer:   %p\n", swSimIf->egrPktBuf);
#endif 
						printf("--------------\n");
					}
					
					break;
			case 2:
					printf("Enter interface name:\n");
					{
						char ifName[10];
						scanf("%s", ifName);
						swSimIntf  *swSimIf = ifLookUpByName(ifName);
						if(!swSimIf)
						{
							printf("%s(): Invalid interface name !!\n", __FUNCTION__);
							continue;
						}
					
						printf("Stats for interface %s\n", ifName);
						printf("	Packets Received :	%d\n", swSimIf->ifStats.pktReceived);
						printf("	Packets Forwarded:	%d\n", swSimIf->ifStats.pktForwarded);
						printf("	Packets Discarded:	%d\n", swSimIf->ifStats.pktDiscarded);
						printf("	Packets Dropped  :	%d\n", swSimIf->ifStats.pktDropped);
					}
					break;
			case 3:
					printf("Exiting Switch Simulator ...\n");
					for (i = 0; i < numPorts; i++)
					{
						swSimIf = swSimIfList[i];

						if(!swSimIf)
						{
							continue;
						}
						mq_unlink(swSimIf->ingressMsgQ);
						mq_unlink(swSimIf->egressMsgQ);
					}
					exit(0);
			default:
					printf("Invalid Input ...\n");
					break;
		}
	}

}
