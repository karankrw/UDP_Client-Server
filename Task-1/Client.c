#include<sys/socket.h>
#include<netinet/in.h>			//sockaddr_in used. IP networking
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<time.h>
#include<stdint.h>

#define PORTNO 8081
#define TIMEOUT 3 // Set Timeout for ACK timer to 3 sec

//Primitives
#define STARTPACKETID 0XFFFF // Start of Packet ID
#define ENDPACKETID 0XFFFF // End of Packet ID
#define CLIENTID 0XFF // Client ID

//Packet Types
#define DATATYPE 0XFFF1 // Data - Packet Types
#define ACKPACKET 0XFFF2 // ACK - Packet Types
#define REJECTPACKETCODE 0XFFF3 // REJECT - Packet Types

//Reject sub codes
#define OUTOFSEQUENCECODE 0XFFF4 // Out of Sequence - REJECT sub codes
#define LENGTHMISMATCHCODE 0XFFF5 // Length Mismatch - REJECT sub codes
#define ENDPACKETIDMISSINGCODE 0XFFF6 // End of Packet missing - REJECT sub codes
#define DUPLICATECODE 0XFFF7 // Duplicate Packet - REJECT sub codes



struct Datapacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_number;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct ackpacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_number;
	uint16_t endpacketID;
};
struct rejectpacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t segment_number;
	uint16_t endpacketID;
};

// function to load the packet with the data
struct Datapacket initialise() {
	struct Datapacket data;
	data.startPacketID = STARTPACKETID;
	data.clientID = CLIENTID;
	data.type = DATATYPE;
	data.endpacketID = ENDPACKETID;
	return data;
}
struct ackpacket ackinitialise() {
	struct ackpacket data;
	data.startPacketID = STARTPACKETID;
	data.clientID = CLIENTID;
	data.type = ACKPACKET;
	data.endpacketID = ENDPACKETID;
	return data;
}
struct rejectpacket rejinitialise() {
	struct rejectpacket data;
	data.startPacketID = STARTPACKETID;
	data.clientID = CLIENTID;
	data.type = ACKPACKET;
	data.endpacketID = ENDPACKETID;
	return data;
}
// Print Client Packet Data
void print(struct Datapacket data) {
    printf("\n INFO: Sending packet:\n");
	printf("Packet ID: %x\n",data.startPacketID);
	printf("Client ID: %hhx\n",data.clientID);
	printf("Data: %x\n",data.type);
	printf("Segment Number: %d \n",data.segment_number);
	printf("Length: %d\n",data.length);
	printf("Payload: %s",data.payload);
	printf("End of Packet ID: %x\n",data.endpacketID);
	printf("\n");
}

// Print Server ACK Packet Data
void ackprint(struct ackpacket adata) {
    printf("\n INFO: Sending packet:\n");
	printf("Packet ID: %x\n",adata.startPacketID);
	printf("Client ID: %x\n",adata.clientID);
	printf("ACK of Packet: %x\n",adata.type);
	printf("Segment Number: %d \n",adata.segment_number+1);
	printf("End of Packet ID: %x\n",adata.endpacketID);
	printf("\n");
}

// Print Server REJECT Packet Data
void rejprint(struct rejectpacket rdata) {
    printf("\n INFO: Sending packet:\n");
	printf("Packet ID: %x\n",rdata.startPacketID);
	printf("Client ID: %hhx\n",rdata.clientID);
	printf("Reject Type of Packet: %x\n",rdata.type);
	printf("Segment Number: %d \n",rdata.segment_number+1);
	printf("Reject : fff3\n");
	printf("Subcode : %x\n",rdata.subcode);
	printf("End of Packet ID: %x\n",rdata.endpacketID);
	printf("\n");
}


int main(){
	struct Datapacket data;
	struct rejectpacket recievedpacket;
	struct sockaddr_in cliaddr;	// For IP networking, we use struct sockaddr_in, which is defined in the header netinet/in.h.
	struct ackpacket ackdata;
	struct rejectpacket rej;
	socklen_t addr_size;
	FILE *fp;


	char line[255];
	int sockfd;
	int n = 0;
	int counter = 0;
	int segmentNo = 1;
	int pacCount = 0;
	
	sockfd = socket(AF_INET,SOCK_DGRAM,0);  //establish connection to socket AF_INET: SOCK_DGRAM (UDP Datagram Service) 
	if(sockfd < 0) {
		printf("xxx Connection to Socket FAILED xxx\n");
	}
	bzero(&cliaddr,sizeof(cliaddr)); //Three key parts to Set and define connection
	cliaddr.sin_family = AF_INET;	//1. Address Family Ipv4.
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//2. Address for the socket. Machine's IP Address.
	cliaddr.sin_port=htons(PORTNO);	//3. The Port number.
	addr_size = sizeof cliaddr ;
	
	struct timeval tv;		// <time.h> library
	tv.tv_sec = TIMEOUT;	// Elapsed Time, in whole Seconds
	tv.tv_usec = 0;			// Rest of Elapsed Time, in Microseconds
	
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));  //Set socket options, used to allocated buffer space, control timeout, permit socket data broadcast
	
	
	fp = fopen("input.txt", "rt");
	if(fp == NULL)
	{
		printf("FILE OPEN EXCEPTION - Cannot Open input.txt\n");
		exit(0);
	}
	
	while(1)
	{
		printf("\n-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~\n");
		printf("Select any one option\n");
		printf("1. Send 5 Packets - all correct\n");
		printf("2. Send 5 Packets - 4 with error, 1 correct\n");
		printf("3. Single Packet with Length Mismatch Error\n");
		printf("4. Single Packet with End of Packet Error\n");
		printf("5. Single Packet with Duplicate Packet Error\n");
		printf("-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~\n");
		
		int i;
		scanf("%d",&i);
		
		switch (i)
		{
		case 1:
			for(pacCount=0;pacCount<5;pacCount++){
				data = initialise();
				ackdata = ackinitialise();
				rej=rejinitialise();
				if(fgets(line, sizeof(line), fp) != NULL) 
				{
					n = 0;
					counter = 0;
					printf("%s",line);
					data.segment_number = segmentNo;
					strcpy(data.payload,line);     
					data.length = strlen(data.payload);
					data.endpacketID = ENDPACKETID;
				}
				while(n<=0 && counter<3)
				{
					//SockADDR: Container that helps OS in identifying the Address family from the first 2 bytes. (Ipv4 here)
					sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
					n = recvfrom(sockfd,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
					if(n <= 0 )
					{
						printf("No response from server for %d seconds. Sending the packet again.\n",TIMEOUT);
						counter ++;
					}
					else if(recievedpacket.type == ACKPACKET  ) 
					{
						print(data);
						printf("ACKNOWLEDGEMENT packet recieved \n");
						ackdata.segment_number = data.segment_number-1;
						ackprint(ackdata);
						printf("\n");
					}
					else if(recievedpacket.type == REJECTPACKETCODE ) 
					{
						printf("REJECT Packet recieved \n");
						rej.subcode=recievedpacket.subcode;
						rej.segment_number = data.segment_number-1;
						if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
						{
							printf("xxx LENGTH MISMATCH ERROR xxx\n");
						}
						if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
						{
							printf("xxx END OF PACKET IDENTIFIER MISSING ERROR xxx\n");
						}
						if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
						{
							printf("xxx OUT OF SEQUENCE ERROR xxx\n");
						}
						if(recievedpacket.subcode == DUPLICATECODE) 
						{
							printf("xxx DUPLICATE PACKET ERROR xxx\n");
						}
						rejprint(rej);
					}
				}
				if(counter >= 3 ) 
				{
					printf("Server does not respond. Server is down for a while. Please try again later.\n");
					exit(0);
				}
				segmentNo++;
				printf("----------------------------------------------------------------------------------------\n");
			}
			break;
					
		case 3:
			data = initialise();
			ackdata = ackinitialise();
			rej=rejinitialise();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = ENDPACKETID;
			}
			data.length++;   // Changing length of data
			while(n<=0 && counter<3)
			{
				sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(sockfd,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("No response from server for %d seconds sending the packet again\n",TIMEOUT);
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) 
				{
					print(data);
					printf("ACKNOWLEDGEMENT packet recieved \n ");
					ackdata.segment_number = data.segment_number-1;
					ackprint(ackdata);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) 
				{
					printf("REJECT Packet recieved \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number-1;
					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
					{
						printf("xxx LENGTH MISMATCH ERROR xxx\n");
					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
					{
						printf("xxx END OF PACKET IDENTIFIER MISSING ERROR xxx\n");
					}
					if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
					{
						printf("xxx OUT OF SEQUENCE ERROR xxx\n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) 
					{
						printf("xxx DUPLICATE PACKET ERROR xxx\n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server does not respond. Server is down for a while. Please try again later.\n");
				exit(0);
			}
			segmentNo++;
			printf("----------------------------------------------------------------------------------------\n");
			break;

		case 4:
			data = initialise();
			ackdata = ackinitialise();
			rej=rejinitialise();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = ENDPACKETID;
			}
			data.endpacketID= 0;  
			while(n<=0 && counter<3)
			{
				sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(sockfd,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("No response from server for %d seconds sending the packet again\n",TIMEOUT);
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) 
				{
					print(data);
					printf("ACKNOWLEDGEMENT packet recieved \n ");
					ackdata.segment_number = data.segment_number-1;
					ackprint(ackdata);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) 
				{
					printf("REJECT Packet recieved \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number-1;
					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
					{
						printf("xxx LENGTH MISMATCH ERROR xxx\n");
					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
					{
						printf("xxx END OF PACKET IDENTIFIER MISSING ERROR xxx\n");
					}
					if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
					{
						printf("xxx OUT OF SEQUENCE ERROR xxx\n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) 
					{
						printf("xxx DUPLICATE PACKET ERROR xxx\n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server does not respond. Server is down for a while. Please try again later.\n");
				exit(0);
			}
			segmentNo++;
			printf("----------------------------------------------------------------------------------------\n");
			break;

		case 5:
			data = initialise();
			ackdata = ackinitialise();
			rej=rejinitialise();
			if(fgets(line, sizeof(line), fp) != NULL) 
			{
				n = 0;
				counter = 0;
				printf("%s",line);
				data.segment_number = segmentNo;
				strcpy(data.payload,line);
				data.length = strlen(data.payload);
				data.endpacketID = ENDPACKETID;
			}
			data.segment_number = 3;
			while(n<=0 && counter<3)
			{
				sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(sockfd,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0)
				{
					printf("ERROR! No response from server for %d seconds sending the packet again\n",TIMEOUT);
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) 
				{
					print(data);
					printf("ACKNOWLEDGEMENT packet recieved \n ");
					ackdata.segment_number = data.segment_number-1;
					ackprint(ackdata);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) 
				{
					printf("REJECT Packet recieved \n");
					rej.subcode=recievedpacket.subcode;
					rej.segment_number = data.segment_number-1;
					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
					{
						printf("xxx LENGTH MISMATCH ERROR xxx\n");
					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
					{
						printf("xxx END OF PACKET IDENTIFIER MISSING ERROR xxx\n");
					}
					if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
					{
						printf("xxx OUT OF SEQUENCE ERROR xxx\n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) 
					{
						printf("xxx DUPLICATE PACKET ERROR xxx\n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) 
			{
				printf("Server does not respond. Server is down for a while. Please try again later.\n");
				exit(0);
			}
			segmentNo++;
			printf("----------------------------------------------------------------------------------------\n");
			break;
		
		case 2:
			for(pacCount=0;pacCount<5;pacCount++){
				data = initialise();
				ackdata = ackinitialise();
				rej=rejinitialise();
				if(fgets(line, sizeof(line), fp) != NULL) 
				{
					n = 0;
					counter = 0;
					printf("%s",line);
					data.segment_number = segmentNo;
					strcpy(data.payload,line);
					data.length = strlen(data.payload);
					data.endpacketID = ENDPACKETID;
				}
				switch(pacCount+1){
					case 1:
						data.length++;
						break;
					case 2:
						data.endpacketID= 0;
						break;
					case 3:
						data.segment_number = 20;
						break;
					case 5:
						data.segment_number = 3;
						break;
				}
				
				while(n<=0 && counter<3)
				{
					sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
					n = recvfrom(sockfd,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
					if(n <= 0 )
					{
						printf("No response from server for %d seconds sending the packet again\n",TIMEOUT);
						counter ++;
					}
					else if(recievedpacket.type == ACKPACKET  ) 
					{
						print(data);
						printf("ACKNOWLEDGEMENT packet recieved \n ");
						ackdata.segment_number = data.segment_number-1;
						ackprint(ackdata);
					}
					else if(recievedpacket.type == REJECTPACKETCODE ) 
					{
						printf("REJECT Packet recieved \n");
						rej.subcode=recievedpacket.subcode;
						rej.segment_number = data.segment_number-1;
						if(recievedpacket.subcode == LENGTHMISMATCHCODE ) 
						{
							printf("xxx LENGTH MISMATCH ERROR xxx\n");
						}
						if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) 
						{
							printf("xxx END OF PACKET IDENTIFIER MISSING ERROR xxx \n");
						}
						if(recievedpacket.subcode == OUTOFSEQUENCECODE ) 
						{
							printf("xxx OUT OF SEQUENCE ERROR xxx\n");
						}
						if(recievedpacket.subcode == DUPLICATECODE) 
						{
							printf("xxx DUPLICATE PACKET ERROR xxx\n");
						}
						rejprint(rej);
					}
				}
				if(counter >= 3 ) 
				{
					printf("xxxx ERROR! Server is down. Please try again later. xxxx\n");
					exit(0);
				}
				segmentNo++;
				printf("----------------------------------------------------------------------------------------\n");
			}
			break;

		default:
			printf("\n \n Invalid Option \n \n!");
		}
	}
}
