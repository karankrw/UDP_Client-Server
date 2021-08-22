#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

//Server Responses
#define port 8080
#define paid 0XFFFB
#define notPaid 0XFFF9
#define doesNotExist 0XFFFA


// Request Packet Structure
struct RequestPacket {
	uint16_t startPacketId;
	uint8_t clientId;
	uint16_t Acc_Per;
	uint8_t segment_Num;
	uint8_t LENGTH;
	uint8_t technology;
	unsigned int SourceSubscriberNum;
	uint16_t endPacket;
};


// Response Packet Structure
struct ResponsePacket {
	uint16_t startPacketId;
	uint8_t clientId;
	uint16_t type;
	uint8_t segment_Num;
	uint8_t LENGTH;
	uint8_t technology;
	unsigned int SourceSubscriberNum;
	uint16_t endPacket;
};


// Function For Printing Packet Contents
void DisplayPacket(struct RequestPacket requestPacket) {
	printf("Packet ID: %x\n",requestPacket.startPacketId);
	printf("Client ID: %hhx\n",requestPacket.clientId);
	printf("Access Permission: %x\n",requestPacket.Acc_Per);
	printf("Segment Number: %d \n",requestPacket.segment_Num);
	printf("Length: %d\n",requestPacket.LENGTH);
	printf("Technology: %d \n", requestPacket.technology);
	printf("Subscriber Number: %u \n",requestPacket.SourceSubscriberNum);
	printf("End of DataPacket ID: %x \n",requestPacket.endPacket);
}


// Intializing Data To Request Packet
struct RequestPacket Initialize () {
	struct RequestPacket requestPkt;
	requestPkt.startPacketId = 0XFFFF;
	requestPkt.clientId = 0XFF;
	requestPkt.Acc_Per = 0XFFF8;
	requestPkt.endPacket = 0XFFFF;
	return requestPkt;

}

int main(int argc, char**argv){
	struct RequestPacket requestPkt;
	struct ResponsePacket responsePkt;
	char line[30];
	int i = 1;
	FILE *filePointer;
	int sockfd,n = 0;
	struct sockaddr_in clientAddr;
	socklen_t addr_size;
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	struct timeval timeValue;
	timeValue.tv_sec = 3;  // Timeout
	timeValue.tv_usec = 0;

	// Checking for connection
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeValue,sizeof(struct timeval));
	int counter = 0;
	if(sockfd < 0) {
		printf("Socket Connection Failed\n");
	}
	bzero(&clientAddr,sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port=htons(port);
	addr_size = sizeof clientAddr ;


	// Loading of data into Packet
	requestPkt = Initialize();

	//Read input file

	filePointer = fopen("input.txt", "rt");

	if(filePointer == NULL)
	{
		printf("Cannot Open The Input File!\n");
	}


	while(fgets(line, sizeof(line), filePointer) != NULL) {
		counter = 0;
		n = 0;
		printf(" \n***  ->  NEW REQUEST  <-  ***\n\n");
		char * words;
		//Split the line
		words = strtok(line," ");
		requestPkt.LENGTH = strlen(words);
		requestPkt.SourceSubscriberNum = (unsigned) atoi(words);
		words = strtok(NULL," ");
		requestPkt.LENGTH += strlen(words);
		requestPkt.technology = atoi(words);
		words = strtok(NULL," ");
		requestPkt.segment_Num = i;
		// Printing Contents of the Packet
		DisplayPacket(requestPkt);
		while(n <= 0 && counter < 3) { // Check if packet sent, if not tries again.
			sendto(sockfd,&requestPkt,sizeof(struct RequestPacket),0,(struct sockaddr *)&clientAddr,addr_size);
			//  Get response from Server
			n = recvfrom(sockfd,&responsePkt,sizeof(struct ResponsePacket),0,NULL,NULL);
			if(n <= 0 ) {
				// No response
				printf("Server TIMEOUT\n");
				counter ++;
			}
			else if(n > 0) {
				// Response recieved
				printf("\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
				printf("Status = ");
				if(responsePkt.type == notPaid) {
					printf("SUBSCRIBER HAS NOT PAID\n");
					printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
				}
				else if(responsePkt.type == doesNotExist ) {
					printf("SUBSCRIBER DOES NOT EXIST ON DATABASE\n");
					printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
				}
				else if(responsePkt.type == paid) {
					printf("PERMITTED TO ACCESS THE NETWORK\n");
					printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

				}
			}
		}
		// After 3 attempts
		if(counter >= 3 ) {
			printf("SERVER NOT RESPONDING");
			exit(0);
		}
		i++;
		printf("\n-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ \n");
	}
	fclose(filePointer);
}
