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
#define ICMP_ETHERTYPE 0x800
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
        unsigned char sender_mac[MAC_LENGTH];  //Length can change
        unsigned char sender_ip[IPV4_LENGTH];  //Length can change
        unsigned char target_mac[MAC_LENGTH];  //Length can change
        unsigned char target_ip[IPV4_LENGTH];  //Length can change
};

//ideas from
//https://stackoverflow.com/questions/16710040/arp-request-and-reply-using-c-socket-programming

int main(){
  int packet_socket;
    //struct ethhdr *send_req = (struct ethhdr *)buf;
  
  //get list of interfaces (actually addresses)
  struct ifaddrs *ifaddr, *tmp;
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs"); //gets ALL addresses in a convient link list
    /*In python is is different but in doc gives a liberary*/
    return 1;
  }
  
  //have the list, loop over the list
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
  
    //Check if this is a packet address, there will be one per
    //interface.  There are IPv4 and IPv6 as well, but we don't care
    //about those for the purpose of enumerating interfaces. We can
    //use the AF_INET addresses in this list for example to get a list
    //of our own IP addresses
    /*AF_IP ip address???*/
    if(tmp->ifa_addr->sa_family==AF_PACKET){/*MAC ADDRESS*/
      printf("Interface: %s\n",tmp->ifa_name); /*If they have the same name then the mac and ip addr goes together*/
      
      //create a packet socket on interface r?-eth1
		if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){ /* want this for all sockets eht 2*/
		printf("Creating Socket on interface %s\n",tmp->ifa_name);
	
		//create a packet socket
		//AF_PACKET makes it a packet socket
		//SOCK_RAW makes it so we get the entire packet
		//could also use SOCK_DGRAM to cut off link layer header
		//ETH_P_ALL indicates we want all (upper layer) protocols
		//we could specify just a specific one			
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
    struct sockaddr_ll recvaddr; 
    /*"man packet" has the stucture for sockaddr_ll*/
    int recvaddrlen=sizeof(struct sockaddr_ll); //length of the structure 
    int n = recvfrom(packet_socket, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen); //returns the length of the message in bytes
    
    unsigned char* eth_start = buf; //pointer to the start poof the buffer
    struct ethhdr *eth = (struct ethhdr *)eth_start; //pointer to the ethernet header
    struct arp_header *arp_header;
    unsigned char my_mac[6];

    //we can use recv, since the addresses are in the packet, but we
    //use recvfrom because it gives us an easy way to determine if
    //this packet is incoming or outgoing (when using ETH_P_ALL, we
    //see packets in both directions. Only outgoing can be seen when
    //using a packet socket with some specific protocol)
    
    

    
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
    	/*The reply needs 
    		-ethernet header
    		-arp header
    			-like the struct made at the top
    		-data
    					*/
    }
    if(ntohs(eth->h_proto) == ICMP_ETHERTYPE){
    	//this is a ICMP message
		/*The reply needs 
    		-ethernet header
    		-IP header
    			-Don't forget Checksum
    		-ICMP Header
    		-data
    					*/
    	
    }
    
    
    //send() send back
    
  }
  //exit
  return 0;
}




/*
POINTERS
cd usr/include/net
file 
ethernet header
cast it as a:
struct ether_header
and use 

less ethernet.h

ip.h

					icmphdr   ---wouldnt use but could if wanted to
if_ppp.h
if_arp.h - minipulates arp headers*************************

can help with differnt types of packetssock

struct iphdr 
it starts 14 bytes in so cant cast it immidiatly but you can start at the 14th bit and then cast it


OVER ALL want to forward packets. R1 and R2 have the same code but used with differnt input forwarding table. 
prefix - used for matching desination header 16-first 2 dots, 24-first 2 dots
middle dash is for arp there is a dash if directly connected
physical port - the direction it sends in eth0 or eth1 or eth2
LAST LINE on the table doesn't have a dash because there is an ip address there and you arp for the MAC address 

PART ONE respond to incoming packets and look at the packet header
	ping R1 from H1 -the first packet sent is going to be an arp, it sends requests until it thinks 
						it has a reply
					-the second packer R1 is going to get is ICMP Echo request
					NEED to find your own MAC address and look at ethernet packet to get destination and send the arp reply.
					then host1 is going to send the echo request
					NEED to send the echo reply
					
					REMEMBER the networksourcery.com for the arp request header layout
					and decide what to put in the arp reply
					
					
					
	MININET
scp prj3.net.py mininet@111.11.111.111:

sudo python prj3-net.py

mininet> xterm h1 r1                    //makes the terminals - type rfconfig can see the connections
mininet> 



PART ONE 
test it with using H1 as ping
can ping from wireshark
CHANGE send sorce- send destination-reply-sorce MAc and IP- GIVE THEM THE ONE THEY PINGED FOR
*/
