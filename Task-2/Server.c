#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define LENGTH 10

//Response Messages
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA

//Request Packet Structure
struct RequestPacket{
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t Acc_Per;
	uint8_t segment_Num;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNum;
	uint16_t endPacketID;
};


//Response Packet Structure
struct ResponsePacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_Num;
	uint8_t length;
	uint8_t technology;
	unsigned int SourceSubscriberNum;
	uint16_t endPacketID;
};


// Subscriber  Information
struct SubscriberDatabase {
	unsigned long subscriberNumber;
	uint8_t technology;
	int status;

};


// Function For Printing Packet Contents
void displayPacket(struct RequestPacket requestPacket ) {
	printf("\nPacket ID: %x\n",requestPacket.startPacketID);
	printf("Client ID: %hhx\n",requestPacket.clientID);
	printf("Access Permission: %x\n",requestPacket.Acc_Per);
	printf("Segment Number: %d \n",requestPacket.segment_Num);
	printf("Length: %d\n",requestPacket.length);
	printf("Technology: %d \n", requestPacket.technology);
	printf("Subscriber Number: %u \n",requestPacket.SourceSubscriberNum);
	printf("End of RequestPacket ID : %x \n",requestPacket.endPacketID);
}


// Generate  Response  Packet
struct ResponsePacket generateResponsePacket(struct RequestPacket requestPacket) {
	struct ResponsePacket responsePacket;
	responsePacket.startPacketID = requestPacket.startPacketID;
	responsePacket.clientID = requestPacket.clientID;
	responsePacket.segment_Num = requestPacket.segment_Num;
	responsePacket.length = requestPacket.length;
	responsePacket.technology = requestPacket.technology;
	responsePacket.SourceSubscriberNum = requestPacket.SourceSubscriberNum;
	responsePacket.endPacketID = requestPacket.endPacketID;
	return responsePacket;
}

// Read File for Mapping
void readFile(struct SubscriberDatabase subDb[]) {

	//Read file and store locally
	char line[30];
	int i = 0;
	FILE *filePointer;

	filePointer = fopen("Verification_Database.txt", "rt");

	if(filePointer == NULL)
	{
		printf("\n ERROR! File not found \n");
		return;
	}
	while(fgets(line, sizeof(line), filePointer) != NULL)
	{
		char * words=NULL;
		words = strtok(line," ");
		subDb[i].subscriberNumber =(unsigned) atol(words);     // long int
		words = strtok(NULL," ");
		subDb[i].technology = atoi(words);						// int
		words = strtok(NULL," ");
		subDb[i].status = atoi(words);
		i++;
	}
	fclose(filePointer);
}


// Check if Subscriber exists
int check(struct SubscriberDatabase subDb[],unsigned int subscriberNumber,uint8_t technology) {
	int value = -1;
	for(int j = 0; j < LENGTH;j++) {
		if(subDb[j].subscriberNumber == subscriberNumber && subDb[j].technology == technology) {
			return subDb[j].status;
		}
                else if (subDb[j].subscriberNumber == subscriberNumber && subDb[j].technology != technology)
                        return 2;
	}
	return value;
}


int main(int argc, char**argv){
	
    struct RequestPacket requestPkt;
	struct ResponsePacket responsePkt;
	
    struct SubscriberDatabase subDb[LENGTH];
	readFile(subDb);
	
    int sockfd,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	
    bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	
    bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server Running\n");
	
        for (;;) {
                // Get the packet
		n = recvfrom(sockfd,&requestPkt,sizeof(struct RequestPacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		displayPacket(requestPkt);
		if(requestPkt.segment_Num == 11) {
			exit(0);
		}

		if(n > 0 && requestPkt.Acc_Per == 0XFFF8) {
			// Response Packet
			responsePkt = generateResponsePacket(requestPkt);

			int value = check(subDb,requestPkt.SourceSubscriberNum,requestPkt.technology);
			if(value == 0) {
				
				responsePkt.type = NOTPAID;
				printf("SUBSCRIBER HAS NOT PAID\n");
			}
			else if(value == 1) {
				
				printf("SUBSCRIBER HAS PAID\n");
				responsePkt.type = PAID;
			}

			else if(value == -1) {
               
				printf("SUBSCRIBER DOES NOT EXIST ON THE DATABASE\n");
				responsePkt.type = NOTEXIST;
			}
                        
                        else{
                                
                                printf("SUBCRIBER TECHNOLOGY MISMATCH\n");
                                responsePkt.type = NOTEXIST;
                        }                        
			// sending the response packet
			sendto(sockfd,&responsePkt,sizeof(struct ResponsePacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		n = 0;
		printf("\n-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ \n");
	}
}
