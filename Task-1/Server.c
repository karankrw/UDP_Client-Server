#include<sys/socket.h>	//Main Sockets Header Library
#include<netinet/in.h>	//Internet Address Family Library. IP networking.
#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

#define PORT 8081 //Communication Port
#define PACKETID 0XFFFF // Start of Packet Identifier  ..... 0XFFFF
#define CLIENTID 0XFF // Client ID  ..... Maximum 0XFF (255 Decimal)
#define DATATYPE 0XFFF1 // Data Packet Type  ....0XFFF1
#define ENDPACKETID 0XFFFF // End of Packet Identifier  ..... 0XFFFF
#define TIMEOUT 3 // Set 3 second timer (recommended) for Timeout (ACK timer)
#define ACKPACKET 0XFFF2 // ACK Packet Type  ....0XFFF2
#define REJECTPACKETCODE 0XFFF3 // REJECT Packet Type  ....0XFFF3
#define LENGTHMISMATCHCODE 0XFFF5 // Length Mismatch: REJECT sub code
#define ENDPACKETIDMISSINGCODE 0XFFF6 // End of Packet missing: REJECT sub code
#define OUTOFSEQUENCECODE 0XFFF4 // Out of Sequence: REJECT sub code
#define DUPLICATECODE 0XFFF7 // Duplicate Packet: REJECT sub code

struct datapacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct ackpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint16_t endpacketID;
};
struct rejectpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t segment_No;
	uint16_t endpacketID;
};
// function to printout the packet information
void show(struct datapacket data) {
	printf("Received Packet Details\n");
	printf("Start of Packet ID: %hx\n",data.packetID);
	printf("Client ID : %hhx\n",data.clientID);
	printf("Data: %x\n",data.type);
	printf("Segment Number: %d\n",data.segment_No);
	printf("Length: %d\n",data.length);
	printf("Payload: %s\n",data.payload);
	printf("End of Packet ID: %x\n",data.endpacketID);
}
// function to generate the reject packet 
struct rejectpacket generaterejectpacket(struct datapacket data) {
	struct rejectpacket reject;
	reject.packetID = data.packetID;
	reject.clientID = data.clientID;
	reject.segment_No = data.segment_No;
	reject.type = REJECTPACKETCODE;
	reject.endpacketID = data.endpacketID;
	return reject;
}
// function to generate the ack packet
struct ackpacket generateackpacket(struct datapacket data) {
	struct ackpacket ack;
	ack.packetID = data.packetID;
	ack.clientID = data.clientID;
	ack.segment_No = data.segment_No;
	ack.type = ACKPACKET ;
	ack.endpacketID = data.endpacketID;
	return ack;
}

int main(int argc, char**argv)
{
	int sockfd,n;
	struct sockaddr_in serverAddr;	// For IP networking, we use struct sockaddr_in, which is defined in the header netinet/in.h.
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	struct datapacket data;
	struct ackpacket  ack;
	struct rejectpacket reject;

	// a pre defined buffer for all the packets, once the buffer is filled server will not send any response.
	int buffer[34];
	int j;	
	for(j=0;j<34;j++) 
	{
		buffer[j] = 0;
	}
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	int expectedPacket = 1;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//Number Conversions: Functions are used to change formats. Big Endian or Little Endian.
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);	//HTONL: reads IP address from a sockaddr structure.
	serverAddr.sin_port=htons(PORT);	//HTONS: stores a port number into a sockaddr structure.
	bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));	//BIND SYSTEM CALL
	//SOCKADDR: structure taken from <sys/socket.h>
	//SockFD: File Descriptor. Information about incoming packet.
	addr_size = sizeof serverAddr;	//Size of Internet Address.
	printf("Server running on port 8081\n");

	// continuously listen for the incoming packets
	for(;;) {
		printf("\n");
		// receiving the packet from client
		n = recvfrom(sockfd,&data,sizeof(struct datapacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		printf("New Packet Arriving........ \n");
		show(data);
		buffer[data.segment_No]++;
		int length = strlen(data.payload);

		if(buffer[data.segment_No] != 1) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = DUPLICATECODE;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("xxx DUPLICATE PACKET RECIEVED xxx\n\n");
		}

		else if(length != data.length) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = LENGTHMISMATCHCODE ;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("xxx LENGTH MISMATCH ERROR xxx\n\n");
		}
		else if(data.endpacketID != ENDPACKETID ) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = ENDPACKETIDMISSINGCODE ;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("xxx END OF PACKET IDENTIFIER MISSING xxx\n\n");
		}
		else if(data.segment_No != expectedPacket && data.segment_No != 10 && data.segment_No != 11) 
		{
			reject = generaterejectpacket(data);
			reject.subcode = OUTOFSEQUENCECODE;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("xxx OUT OF SEQUENCE ERROR xxx\n\n");
		}
		else 
		{
			ack = generateackpacket(data);
			sendto(sockfd,&ack,sizeof(struct ackpacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		expectedPacket++;
		printf("~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~\n");
	}
}
