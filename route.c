/***************************
 *****Madison Brooks********
 *****Bailey Freund********* 
 *******CIS 457-20**********
 ********Project 3**********
 ****************************/

#include <sys/socket.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
/*Added code*/
#include <arpa/inet.h>
#include <netinet/ip_icmp.h> 
#include <stdlib.h>
#include <net/if_arp.h>
#include <net/if_ppp.h>
#include <netinet/ip.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1500
#define ARP_ETHERTYPE 0x0806
#define IP_ETHERTYPE 0x800
#define MAC_LENGTH 6
#define IPV4_LENGTH 4

/*A struct that is for the arp header - it is inorder of how the arp header is*/
struct arp_header
{
        unsigned short hardware_type;          // 16 bits
        unsigned short protocol_type;          // 16 bits
        unsigned char hardwareAddr_len;        // 6              has 8  bits?
        unsigned char protocolAddr_len;        // 4              has 8  bits?
        unsigned short opcode;                 // 16 bits
        unsigned char sender_mac[MAC_LENGTH];  // 6 bits
        unsigned char sender_ip[IPV4_LENGTH];  // 4 bits
        unsigned char target_mac[MAC_LENGTH];  // 6 bits
        unsigned char target_ip[IPV4_LENGTH];  // 4 bits
};


//help from
//https://stackoverflow.com/questions/16710040/arp-request-and-reply-using-c-socket-programming
//http://www.networksorcery.com/
//http://opensourceforu.com/2015/03/a-guide-to-using-raw-sockets/

//function headers
void makeArpReply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]);

void makeICMPreply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]);
unsigned short checksum(void *b, int len);

int main(int argc, char *argv[]){
  int packet_socket;
  
  //get list of interfaces (actually addresses)
  struct ifaddrs *ifaddr, *tmp; //gets ALL addresses in a convient link list
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs"); 
    return 1;
  }
  
  //have the list, loop over the list
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    //Check if this is a packet address, there will be one per
    //interface.  There are IPv4 and IPv6 as well, but we don't care
    //about those for the purpose of enumerating interfaces. We can
    //use the AF_INET addresses in this list for example to get a list
    //of our own IP addresses
    if(tmp->ifa_addr->sa_family==AF_PACKET){/*MAC ADDRESS*/
      printf("Interface: %s\n",tmp->ifa_name); /*If they have the same name then the mac and ip addr goes together*/
      

	  
      //create a packet socket on interface r?-eth1
		if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){ /* want this for all sockets, including eht 2*/
			printf("Creating Socket on interface %s\n",tmp->ifa_name);
	
		//create a packet socket
		//AF_PACKET makes it a packet socket
		//SOCK_RAW makes it so we get the entire packet
			//could also use SOCK_DGRAM to cut off link layer header
		//ETH_P_ALL indicates we want all (upper layer) protocols -- we could specify just a specific one			
		/*AF_packet indecates that we want the headers of the packet, SOCK_RAW gets all headers, 
			htons() is a filture saying it wants everything*/
			//OPENS SOCKET
		packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); 

		if(packet_socket<0){
		  perror("socket");
		  return 2;
		}
			//Bind the socket to the address, so we only get packets
			//recieved on this specific interface. For packet sockets, the
			//address structure is a struct sockaddr_ll (see the man page
			//for "packet"), but of course bind takes a struct sockaddr.
			//Here, we can use the sockaddr we got from getifaddrs (which
			//we could convert to sockaddr_ll if we needed to)      
	  //getting the Mac address for an interface?
      	//struct sockaddr_ll *s = (struct sockaddr_ll*) tmp->ifa_addr;
	  	//unsigned char macAddress[6];
	  	//memcpy(macAddress, s->sll_addr, 6);
	  
		if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
	  		perror("bind"); /*binds soccet and mac addr*/
		}
      }
    }
  }
  //free the interface list when we don't need it anymore
 // freeifaddrs(ifaddr); 
  /*MOST STUDENT CONFUSION LINES because if you have pointers to things in this list then do not free the memory- concider commenting*/

  //loop and recieve packets. We are only looking at one interface,
  //for the project you will probably want to look at more (to do so,
  //a good way is to have one socket per interface and use select to
  //see which ones have data)
  printf("Ready to recieve now\n");
  
  while(1){
    char buf[BUFFER_SIZE];    //buffer for the message
    struct sockaddr_ll recvaddr; // given/*"man packet" has the stucture for sockaddr_ll*/
    int recvaddrlen=sizeof(struct sockaddr_ll); //length of the structure     
    //we can use recv, since the addresses are in the packet, but we
    //use recvfrom because it gives us an easy way to determine if
    //this packet is incoming or outgoing (when using ETH_P_ALL, we
    //see packets in both directions. Only outgoing can be seen when
    //using a packet socket with some specific protocol)
    int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen); //returns the length of the message in bytes
    
    unsigned char* eth_start = buf; //pointer to the start too the buffer
    struct ethhdr *eth = (struct ethhdr *)eth_start; //pointer to the ethernet header
    //have the ethernet header
    struct sockaddr_ll *s = (struct sockaddr_ll*) tmp->ifa_addr;
    unsigned char myMacAddr[MAC_LENGTH];
	memcpy(myMacAddr, s->sll_addr, 6); //copy in the mac address
	//unsigned char myIPAddr[IPV4_LENGTH]; //trying to get the IP address
	char * myIPAddr = inet_ntoa(((struct sockaddr_in *) tmp->ifa_addr)->sin_addr); // my ip address 
    
    //@TODO: Make the interfaces
//Source Address ?? eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5] //trying to get myMac address

    
   	//ignore outgoing packets (we can't disable some from being sent
    //by the OS automatically, for example ICMP port unreachable
    //messages, so we will just ignore them here)
    if(recvaddr.sll_pkttype==PACKET_OUTGOING)
      continue; /*Dont care about packets the router is sending*/
    //start processing all others
    
    printf("Got a %d byte packet\n", n);
    /*NOW GOT A PACKET AND ALL OF ITS HEADERS*/
    //what else to do is up to you, you can send packets with send,
    //just like we used for TCP sockets (or you can use sendto, but it
    //is not necessary, since the headers, including all addresses,
    //need to be in the buffer you are sending)
	//SO the message and contents of the buffer are in buf, need to minipulate this buffer to send something back to the client

    
    if(ntohs(eth->h_proto) == ARP_ETHERTYPE){
    	//this is a ARP message according to the ethernet header
    	unsigned char* arp = buf + 14; //pointer to the spot in the NEW buffer where the arp header is
		struct arp_header *arphead = (struct arp_header *)arp;  //NEW pointer to the header as a struct that we defined at the top
		//have arp header
    	if (ntohs(arphead->opcode) == ARPOP_REQUEST) { //if the arp header says it is an arp request
    		makeArpReply(buf,packet_socket,myMacAddr, myIPAddr); 
    		//this function above makes the packet headers and sends the packet
    	}
    }
    
    if(ntohs(eth->h_proto) == IP_ETHERTYPE){ 
    	//this is a packet where the IP header will tell if it is a ICMP packet
    	makeICMPreply(buf,packet_socket,myMacAddr, myIPAddr);
    	
   }
    
  }
  //exit
  return 0;
}

void makeArpReply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]){
		/*The arp reply needs 
    		-ethernet header
    		-arp header
    			-like the struct made at the top
    		-data??
    	*/		
    	int i; //variable for for loops
    	
    	//pointers to the old buffer to use (in the begining tried to just edit this buffer but decided to make a new buffer)
    	unsigned char* ethO = (unsigned char*) buf; //pointer to the OLD buffer
		struct ethhdr *ethHeaderO = (struct ethhdr *) ethO;	//OLD pointer to the ethernet header of the old buffer as a stuct
		unsigned char* arpO = buf + 14; //pointer to the spot in the OLD buffer where the arp header is
		struct arp_header *arphead = (struct arp_header *)arpO;  //OLD pointer to the header as a struct that we defined at the top
    	
    	//new buffer creation so we can send a new message
    	unsigned char Arp_Buf[42]; //the buffer we are going to send back
    	
    	for(i = 1; i<43; i++){
    		Arp_Buf[i] = 0; //populating the buffer with zeros
    	}
    	
    	//make the new buffer headers
 		unsigned char* eth = (unsigned char*) Arp_Buf; //pointer to the NEW buffer
		struct ethhdr *ethHeader = (struct ethhdr *) eth;	//NEW pointer to the ethernet header of the old buffer as a stuct
		unsigned char* arp = Arp_Buf + 14; //pointer to the spot in the NEW buffer where the arp header is
		struct arp_header *arpHead = (struct arp_header *)arp;  //NEW pointer to the header as a struct that we defined at the top
 		
	/**************making the ethernet header**/
			//destination is the source from the old packet
			unsigned char oldSource[6];//might need to pass this as a perameter
			
			for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
				//putting the old source into an array so that it can be used to set the destination
				oldSource[i] = ethHeaderO->h_source[i];
			}
			
			ethHeader->h_proto = htons(ARP_ETHERTYPE); //setting the type to be arp 0806
			
			//getting the source and destination mac adresses ready
				//ethHeader->h_dest = ethHeader->h_source; //does not work because incompatable types

			//making the source for the packet our own mac address
			for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
				ethHeader->h_source[i] = myMacAddr[i]; //setting the source in the ethernet header to our mac address
			}
			//making the destination for the packet the old source
			for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
				ethHeader->h_dest[i] = oldSource[i]; 
			}
			unsigned char * startofBuf = (unsigned char *)ethHeader;
			for (i = 0; i < sizeof(struct ethhdr); i++) {
				Arp_Buf[i] = startofBuf[i]; //adding the ethernet header to the start of the arp reply
			}

	/******************making the arp responce header*/
			unsigned char oldSourceIP[4];
			
			arpHead->opcode =htons(ARPOP_REPLY); //setting the op code to be a reply
			
			for(i=0; i<4; i++){
				oldSourceIP[i] = arphead->sender_ip[i]; //getting the old ip address out of the ARP Header
			}
			
			for (i = 0; i < 4; i++) {
				//IP Address for the target is the old source IP found above by getting the sender IP out of the arp header
				arpHead->target_ip[i] = oldSourceIP[i];
			}
						
			for (i = 0; i < 6; i++) {
				//setting the senders MAC as our own
				arpHead->sender_mac[i] = myMacAddr[i];
			}
			
			for (i = 0; i < 6; i++) { //have to loop
				//setting the target MAC as the old source address from the packet
				arpHead->target_mac[i] = oldSource[i];
			}
			
			for (i = 0; i < 4; i++) {
				//IP Address for the sender is our ip address
				arpHead->sender_ip[i] = myIPAddr[i];
			}

			arpHead->hardware_type = arphead->hardware_type;
			arpHead->protocol_type = arphead->protocol_type;
			arpHead->hardwareAddr_len = arphead->hardwareAddr_len; //could this be replaced with MAC_LENGTH?
			arpHead->protocolAddr_len = arphead->protocolAddr_len; //then this replaced with IPV4_LENGTH?
			
			unsigned char * secondBuf = (unsigned char *)arpHead; //making a pointer to the new arp header to get the contents
			for (i = 0; i < sizeof(struct arp_header); i++) {
				Arp_Buf[i + 14] = secondBuf[i]; //addind the arp header to the buffer
			}
    	/****Sending the reply!****/
		send(packetSocket, Arp_Buf, 42, 0); //send the reply back :-)
}

//TODO:notification of a failed packet if the IP address does not exist

/***********************************************************ICMP Reply***************************************************************/
void makeICMPreply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]){
		/*The reply needs 
    		-ethernet header
    		-IP header
    			-Don't forget Checksum
    		-ICMP Header
    		-data
    					*/
    	char ICMP_Buf[100]; //unsure of how big this buffer needs to be
    		
    	int i; //variable for for loops
    	
    	//pointers to the old buffer to use (in the begining tried to just edit this buffer but decided to make a new buffer)
    	unsigned char* ethO = (unsigned char*) buf; //pointer to the OLD buffer
		struct ethhdr *ethHeaderO = (struct ethhdr *) ethO;	//OLD pointer to the ethernet header of the old buffer as a stuct
		unsigned char* ipHeaderr = (unsigned char*) buf+14;
		struct iphdr *ipHeadd = (struct iphdr *) ipHeaderr;
		unsigned char* icmpHeaderr = (unsigned char*) buf; //@TODO Find out how long the IP header is 
		struct icmp *icmpHeadd = (struct icmp *) icmpHeaderr;
    	
    	//make the new buffer headers
 		unsigned char* eth = (unsigned char*) ICMP_Buf;           //pointer to the NEW buffer
		struct ethhdr *ethHeader = (struct ethhdr *) eth;	      //NEW pointer to the ethernet header of the old buffer as a stuct
		unsigned char* ipHeader = (unsigned char*) ICMP_Buf+14;   // New pointer to the IP Header in the buffer 
		struct iphdr *ipHead = (struct iphdr *) ipHeader;           // New pointer to the IP Header in the new buffer
		//ICMP Header is 64
		unsigned char* icmpHeader = (unsigned char*) ICMP_Buf+10; //@TODO Find out how long the IP header is 160+ or minus 
		struct icmp *icmpHead = (struct icmp *) icmpHeader;
    	
   //**************making the ethernet header****************************************************************************
	//destination is the source from the old packet
		unsigned char oldSource[6];//might need to pass this as a perameter
		
		for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
			//putting the old source into an array so that it can be used to set the destination
			oldSource[i] = ethHeaderO->h_source[i];
		}
		
		ethHeader->h_proto = htons(IP_ETHERTYPE); //setting the type to be arp 0806

		//making the source for the packet our own mac address
		for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
			ethHeader->h_source[i] = myMacAddr[i]; //setting the source in the ethernet header to our mac address
		}
		
		//making the destination for the packet the old source
		for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
			ethHeader->h_dest[i] = oldSource[i]; 
		}
    		
    //**************making the IP Header**********************************************************************************	
		
	   		
    //**************making ICMP Header************************************************************************************  
    	
		
    		
    	//send(packet_socket, ICMP_Buf, 100, 0); //send the reply back :-)
}


unsigned short checksum(void *b, int len){
	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

