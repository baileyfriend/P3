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

/*A struct that is for the arp header*/
struct arp_header
{
        unsigned short hardware_type;          // 16 bits
        unsigned short protocol_type;          // 16 bits
        unsigned char hardwareAddr_len;        // 8  bits
        unsigned char  protocolAddr_len;       // 8  bits
        unsigned short opcode;                 // 16 bits
        unsigned char sender_mac[MAC_LENGTH];  
        unsigned char sender_ip[IPV4_LENGTH];  
        unsigned char target_mac[MAC_LENGTH];  
        unsigned char target_ip[IPV4_LENGTH]; 
};

//ideas from
//https://stackoverflow.com/questions/16710040/arp-request-and-reply-using-c-socket-programming

//http://opensourceforu.com/2015/03/a-guide-to-using-raw-sockets/

//function headers
void makeArpReply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]);

void makeICMPreply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]);


int main(){
  int packet_socket;
    //struct ethhdr *send_req = (struct ethhdr *)buf;
  
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
    //read in first 14?
    
    if(ntohs(eth->h_proto) == ARP_ETHERTYPE){
    	//this is a ARP message

    	makeArpReply(buf,packet_socket,myMacAddr, myIPAddr);
    }
    if(ntohs(eth->h_proto) == IP_ETHERTYPE){ 
    	//this is a packet where the IP header will tell if it is a ICMP packet
    	//this is a ICMP message

    	makeICMPreply(buf,packet_socket,myMacAddr, myIPAddr);
    	
   }
    
    
    //send() send back
    
  }
  //exit
  return 0;
}

void makeArpReply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]){
		    	/*The arp reply needs 
    		-ethernet header
    		-arp header
    			-like the struct made at the top
    		-data
    					*/
    	char Arp_Buf[42]; //the buffer we are going to send back
    	
    	//make the buffer
 
		unsigned char* eth = (unsigned char*) buf; //pointer to the old buffer
		struct ethhdr *ethHeader = (struct ethhdr *) eth;	//pointer to the ethernet header of the old buffer as a stuct
		
		unsigned char* arp = buf + 14; //pointer to the spot in the old buffer where the arp header is
		struct arp_header *arpHead = (struct arp_header *)arp;  //pointer to the header as a struct that we defined at the top
		
		if (ntohs(arpHead->opcode) == ARPOP_REQUEST) { //if it is a arp request
			//the old buffer is an arp request so we should make a responce to it
			int i;
			//**************making the ethernet header
			//destination is the source from the old packet
			//might need to pass this as a perameter
			unsigned char oldSource[6];
			for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
				oldSource[i] = ethHeader->h_source[i];
			}
			
			ethHeader->h_proto = htons(ARP_ETHERTYPE); //setting the type to be arp 0806
			//getting the source and destination mac adresses ready
				//ethHeader->h_dest = ethHeader->h_source; //does not work because incompatable types
				// //does not work because incompatable types
			
			for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
				ethHeader->h_source[i] = myMacAddr[i]; //setting the source in the ethernet header to our mac address
			}

			for(i=0;i<6;i++){ //have to loop through because one is a char * and the other is char[6]
				ethHeader->h_dest[i] = oldSource[i]; 
			}
			
			//******************making the arp responce header
		}
    	
    	
		//send(packet_socket, Arp_Buf, 42, 0); //send the reply back :-)
}
void makeICMPreply(char buf[BUFFER_SIZE], int packetSocket, unsigned char myMacAddr[MAC_LENGTH], unsigned char myIPAddr[IPV4_LENGTH]){
		/*The reply needs 
    		-ethernet header
    		-IP header
    			-Don't forget Checksum
    		-ICMP Header
    		-data
    					*/
    		char ICMP_Buf[100]; //unsure of how big this buffer needs to be
    		
    		//TODO: make the buffer
    		
    		//send(packet_socket, ICMP_Buf, 100, 0); //send the reply back :-)
}

