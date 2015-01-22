/*
 *	LiMon Monitor (LiMon) - Network.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *
 *
 * History
 *	9/16/00	  bor  adapted to TQM823L/STK8xxL board, RARP/TFTP boot added
 */

#ifndef __NET_H__
#define __NET_H__
#include <string.h>
#include <common.h>

#define DEBUG_LL_STATE 0	/* Link local state machine changes */
#define DEBUG_DEV_PKT 0		/* Packets or info directed to the device */
#define DEBUG_NET_PKT 0		/* Packets on info on the network at large */
#define DEBUG_INT_STATE 0	/* Internal network state changes */

/*
 *	The number of receive packet buffers, and the required packet buffer
 *	alignment in memory.
 *
 */

#ifdef CONFIG_SYS_RX_ETH_BUFFER
# define PKTBUFSRX	CONFIG_SYS_RX_ETH_BUFFER
#else
# define PKTBUFSRX	4
#endif

#define PKTALIGN	4

/* IPv4 addresses are always 32 bits in size */
typedef u32		IPaddr_t;


/**
 * An incoming packet handler.
 * @param pkt    pointer to the application packet
 * @param dport  destination UDP port
 * @param sip    source IP address
 * @param sport  source UDP port
 * @param len    packet length
 */
typedef void rxhand_f(unchar *pkt, unsigned dport,
		      IPaddr_t sip, unsigned sport,
		      unsigned len);

/**
 * An incoming ICMP packet handler.
 * @param type	ICMP type
 * @param code	ICMP code
 * @param dport	destination UDP port
 * @param sip	source IP address
 * @param sport	source UDP port
 * @param pkt	pointer to the ICMP packet data
 * @param len	packet length
 */
typedef void rxhand_icmp_f(unsigned type, unsigned code, unsigned dport,
		IPaddr_t sip, unsigned sport, unchar *pkt, unsigned len);

/*
 *	A timeout handler.  Called after time interval has expired.
 */
typedef void	thand_f(void);

enum eth_state_t {
	ETH_STATE_INIT,
	ETH_STATE_PASSIVE,
	ETH_STATE_ACTIVE
};

struct eth_device {
	char name[16];
	unsigned char enetaddr[6];
	int iobase;
	int state;

	int  (*send) (struct eth_device *, void *packet, int length);
	int  (*recv) (struct eth_device *);
	int  (*write_hwaddr) (struct eth_device *);
	void *priv;
};

extern int eth_initialize(char *filename);	/* Initialize network subsystem */
extern int eth_register(struct eth_device* dev);/* Register network device */
extern int eth_unregister(struct eth_device *dev);/* Remove network device */

/* get the current device MAC */
extern struct eth_device *eth_current;

static inline __attribute__((always_inline))
struct eth_device *eth_get_dev(void)
{
	return eth_current;
}

extern int eth_init(void);			/* Initialize the device */
extern int eth_send(void *packet, int length);	   /* Send a packet */
extern int eth_rx(void);			/* Check for received packets */
extern void eth_halt(void);			/* stop SCC */
extern char *eth_get_name(void);		/* get name of current device */

/* Set active state */
static inline __attribute__((always_inline)) int eth_init_state_only(void)
{
	eth_get_dev()->state = ETH_STATE_ACTIVE;

	return 0;
}
/* Set passive state */
static inline __attribute__((always_inline)) void eth_halt_state_only(void)
{
	eth_get_dev()->state = ETH_STATE_PASSIVE;
}

/*
 * Set the hardware address for an ethernet interface based on 'eth%daddr'
 * environment variable (or just 'ethaddr' if eth_number is 0).
 * Args:
 *	base_name - base name for device (normally "eth")
 *	eth_number - value of %d (0 for first device of this type)
 * Returns:
 *	0 is success, non-zero is error status from driver.
 */
int eth_write_hwaddr(struct eth_device *dev, const char *base_name,
		     int eth_number);


/**********************************************************************/
/*
 *	Protocol headers.
 */

/*
 *	Ethernet header
 */

struct ethernet_hdr {
	unchar		et_dest[6];	/* Destination node		*/
	unchar		et_src[6];	/* Source node			*/
	ushort		et_protlen;	/* Protocol or length		*/
};

/* Ethernet header size */
#define ETHER_HDR_SIZE	(sizeof(struct ethernet_hdr))

struct e802_hdr {
	unchar		et_dest[6];	/* Destination node		*/
	unchar		et_src[6];	/* Source node			*/
	ushort		et_protlen;	/* Protocol or length		*/
	unchar		et_dsap;	/* 802 DSAP			*/
	unchar		et_ssap;	/* 802 SSAP			*/
	unchar		et_ctl;		/* 802 control			*/
	unchar		et_snap1;	/* SNAP				*/
	unchar		et_snap2;
	unchar		et_snap3;
	ushort		et_prot;	/* 802 protocol			*/
};

/* 802 + SNAP + ethernet header size */
#define E802_HDR_SIZE	(sizeof(struct e802_hdr))

/*
 *	Virtual LAN Ethernet header
 */
struct vlan_ethernet_hdr {
	unchar		vet_dest[6];	/* Destination node		*/
	unchar		vet_src[6];	/* Source node			*/
	ushort		vet_vlan_type;	/* PROT_VLAN			*/
	ushort		vet_tag;	/* TAG of VLAN			*/
	ushort		vet_type;	/* protocol type		*/
};

/* VLAN Ethernet header size */
#define VLAN_ETHER_HDR_SIZE	(sizeof(struct vlan_ethernet_hdr))

#define PROT_IP		0x0800		/* IP protocol			*/
#define PROT_ARP	0x0806		/* IP ARP protocol		*/
#define PROT_RARP	0x8035		/* IP ARP protocol		*/
#define PROT_VLAN	0x8100		/* IEEE 802.1q protocol		*/

#define IPPROTO_ICMP	 1	/* Internet Control Message Protocol	*/
#define IPPROTO_UDP	17	/* User Datagram Protocol		*/

/*
 *	Internet Protocol (IP) header.
 */
struct ip_hdr {
	unchar		ip_hl_v;	/* header length and version	*/
	unchar		ip_tos;		/* type of service		*/
	ushort		ip_len;		/* total length			*/
	ushort		ip_id;		/* identification		*/
	ushort		ip_off;		/* fragment offset field	*/
	unchar		ip_ttl;		/* time to live			*/
	unchar		ip_p;		/* protocol			*/
	ushort		ip_sum;		/* checksum			*/
	IPaddr_t	ip_src;		/* Source IP address		*/
	IPaddr_t	ip_dst;		/* Destination IP address	*/
};

#define IP_OFFS		0x1fff /* ip offset *= 8 */
#define IP_FLAGS	0xe000 /* first 3 bits */
#define IP_FLAGS_RES	0x8000 /* reserved */
#define IP_FLAGS_DFRAG	0x4000 /* don't fragments */
#define IP_FLAGS_MFRAG	0x2000 /* more fragments */

#define IP_HDR_SIZE		(sizeof(struct ip_hdr))

/*
 *	Internet Protocol (IP) + UDP header.
 */
struct ip_udp_hdr {
	unchar		ip_hl_v;	/* header length and version	*/
	unchar		ip_tos;		/* type of service		*/
	ushort		ip_len;		/* total length			*/
	ushort		ip_id;		/* identification		*/
	ushort		ip_off;		/* fragment offset field	*/
	unchar		ip_ttl;		/* time to live			*/
	unchar		ip_p;		/* protocol			*/
	ushort		ip_sum;		/* checksum			*/
	IPaddr_t	ip_src;		/* Source IP address		*/
	IPaddr_t	ip_dst;		/* Destination IP address	*/
	ushort		udp_src;	/* UDP source port		*/
	ushort		udp_dst;	/* UDP destination port		*/
	ushort		udp_len;	/* Length of UDP packet		*/
	ushort		udp_xsum;	/* Checksum			*/
};

#define IP_UDP_HDR_SIZE		(sizeof(struct ip_udp_hdr))
#define UDP_HDR_SIZE		(IP_UDP_HDR_SIZE - IP_HDR_SIZE)

/*
 *	Address Resolution Protocol (ARP) header.
 */
struct arp_hdr {
	ushort		ar_hrd;		/* Format of hardware address	*/
#   define ARP_ETHER	    1		/* Ethernet  hardware address	*/
	ushort		ar_pro;		/* Format of protocol address	*/
	unchar		ar_hln;		/* Length of hardware address	*/
#   define ARP_HLEN	6
	unchar		ar_pln;		/* Length of protocol address	*/
#   define ARP_PLEN	4
	ushort		ar_op;		/* Operation			*/
#   define ARPOP_REQUEST    1		/* Request  to resolve  address	*/
#   define ARPOP_REPLY	    2		/* Response to previous request	*/

#   define RARPOP_REQUEST   3		/* Request  to resolve  address	*/
#   define RARPOP_REPLY	    4		/* Response to previous request */

	/*
	 * The remaining fields are variable in size, according to
	 * the sizes above, and are defined as appropriate for
	 * specific hardware/protocol combinations.
	 */
	unchar		ar_data[0];
#define ar_sha		ar_data[0]
#define ar_spa		ar_data[ARP_HLEN]
#define ar_tha		ar_data[ARP_HLEN + ARP_PLEN]
#define ar_tpa		ar_data[ARP_HLEN + ARP_PLEN + ARP_HLEN]
#if 0
	unchar		ar_sha[];	/* Sender hardware address	*/
	unchar		ar_spa[];	/* Sender protocol address	*/
	unchar		ar_tha[];	/* Target hardware address	*/
	unchar		ar_tpa[];	/* Target protocol address	*/
#endif /* 0 */
};

#define ARP_HDR_SIZE	(8+20)		/* Size assuming ethernet	*/

/*
 * ICMP stuff (just enough to handle (host) redirect messages)
 */
#define ICMP_ECHO_REPLY		0	/* Echo reply			*/
#define ICMP_NOT_REACH		3	/* Detination unreachable	*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/
#define ICMP_ECHO_REQUEST	8	/* Echo request			*/

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET		0	/* Redirect Net			*/
#define ICMP_REDIR_HOST		1	/* Redirect Host		*/

/* Codes for NOT_REACH */
#define ICMP_NOT_REACH_PORT	3	/* Port unreachable		*/

struct icmp_hdr {
	unchar		type;
	unchar		code;
	ushort		checksum;
	union {
		struct {
			ushort	id;
			ushort	sequence;
		} echo;
		ulong	gateway;
		struct {
			ushort	__unused;
			ushort	mtu;
		} frag;
		unchar data[0];
	} un;
};

#define ICMP_HDR_SIZE		(sizeof(struct icmp_hdr))
#define IP_ICMP_HDR_SIZE	(IP_HDR_SIZE + ICMP_HDR_SIZE)

/*
 * Maximum packet size; used to allocate packet storage.
 * TFTP packets can be 524 bytes + IP header + ethernet header.
 * Lets be conservative, and go for 38 * 16.  (Must also be
 * a multiple of 32 bytes).
 */
/*
 * AS.HARNOIS : Better to set PKTSIZE to maximum size because
 * traffic type is not always controlled
 * maximum packet size =  1518
 * maximum packet size and multiple of 32 bytes =  1536
 */
#define PKTSIZE			1518
#define PKTSIZE_ALIGN		1536
/*#define PKTSIZE		608*/

/*
 * Maximum receive ring size; that is, the number of packets
 * we can buffer before overflow happens. Basically, this just
 * needs to be enough to prevent a packet being discarded while
 * we are processing the previous one.
 */
#define RINGSZ		4
#define RINGSZ_LOG2	2

/**********************************************************************/
/*
 *	Globals.
 *
 * Note:
 *
 * All variables of type IPaddr_t are stored in NETWORK byte order
 * (big endian).
 */

/* net.c */
/** BOOTP EXTENTIONS **/
extern IPaddr_t NetOurGatewayIP;	/* Our gateway IP address */
extern IPaddr_t NetOurSubnetMask;	/* Our subnet mask (0 = unknown) */
extern IPaddr_t NetOurDNSIP;	/* Our Domain Name Server (0 = unknown) */
#if defined(CONFIG_BOOTP_DNS2)
extern IPaddr_t NetOurDNS2IP;	/* Our 2nd Domain Name Server (0 = unknown) */
#endif
extern char	NetOurNISDomain[32];	/* Our NIS domain */
extern char	NetOurHostName[32];	/* Our hostname */
extern char	NetOurRootPath[64];	/* Our root path */
extern ushort	NetBootFileSize;	/* Our boot file size in blocks */
/** END OF BOOTP EXTENTIONS **/
extern ulong		NetBootFileXferSize;	/* size of bootfile in bytes */
extern unchar		NetOurEther[6];		/* Our ethernet address */
extern unchar		NetServerEther[6];	/* Boot server enet address */
extern IPaddr_t		NetOurIP;	/* Our    IP addr (0 = unknown) */
extern IPaddr_t		NetServerIP;	/* Server IP addr (0 = unknown) */
extern unchar		*NetTxPacket;		/* THE transmit packet */
extern unchar		*NetRxPackets[PKTBUFSRX]; /* Receive packets */
extern unchar		*NetRxPacket;		/* Current receive packet */
extern int		NetRxPacketLen;		/* Current rx packet length */
extern unsigned		NetIPID;		/* IP ID (counting) */
extern unchar		NetBcastAddr[6];	/* Ethernet boardcast address */
extern unchar		NetEtherNullAddr[6];

#define VLAN_NONE	4095			/* untagged */
#define VLAN_IDMASK	0x0fff			/* mask of valid vlan id */
extern ushort		NetOurVLAN;		/* Our VLAN */
extern ushort		NetOurNativeVLAN;	/* Our Native VLAN */

extern int		NetRestartWrap;		/* Tried all network devices */

enum proto_t {
	BOOTP, RARP, ARP, TFTPGET, DHCP, PING, DNS, NFS, CDP, NETCONS, SNTP,
	TFTPSRV, TFTPPUT, LINKLOCAL
};

/* from net/net.c */
extern char	BootFile[128];			/* Boot File name */

extern IPaddr_t	NetPingIP;			/* the ip address to ping */

/* Initialize the network adapter */
extern void net_init(void);
extern int NetLoop(enum proto_t);

/* Get size of the ethernet header when we send */
extern int	NetEthHdrSize(void);

/* Set ethernet header; returns the size of the header */
extern int NetSetEther(unchar *, unchar *, uint);
extern int net_update_ether(struct ethernet_hdr *et, unchar *addr, uint prot);

/* Set IP header */
extern void net_set_ip_header(unchar *pkt, IPaddr_t dest, IPaddr_t source);
extern void net_set_udp_header(unchar *pkt, IPaddr_t dest, int dport,
				int sport, int len);

/* Checksum */
extern int	NetCksumOk(unchar *, int);	/* Return true if cksum OK */
extern uint	NetCksum(unchar *, int);		/* Calculate the checksum */

/* Callbacks */
extern rxhand_f *net_get_udp_handler(void);	/* Get UDP RX packet handler */
extern void net_set_udp_handler(rxhand_f *);	/* Set UDP RX packet handler */
extern rxhand_f *net_get_arp_handler(void);	/* Get ARP RX packet handler */
extern void net_set_arp_handler(rxhand_f *);	/* Set ARP RX packet handler */
extern void net_set_icmp_handler(rxhand_icmp_f *f); /* Set ICMP RX handler */
extern void	NetSetTimeout(ulong, thand_f *);/* Set timeout handler */

/* Network loop state */
enum net_loop_state {
	NETLOOP_CONTINUE,
	NETLOOP_RESTART,
	NETLOOP_SUCCESS,
	NETLOOP_FAIL
};
extern enum net_loop_state net_state;

static inline void net_set_state(enum net_loop_state state)
{
	debug_cond(DEBUG_INT_STATE, "--- NetState set to %d\n", state);
	net_state = state;
}

/* Transmit a packet */
static inline void NetSendPacket(unchar *pkt, int len)
{
	if (len < 64)
		len = 64;

	(void) eth_send(pkt, len);
}

/*
 * Transmit "NetTxPacket" as UDP packet, performing ARP request if needed
 *  (ether will be populated)
 *
 * @param ether Raw packet buffer
 * @param dest IP address to send the datagram to
 * @param dport Destination UDP port
 * @param sport Source UDP port
 * @param payload_len Length of data after the UDP header
 */
extern int NetSendUDPPacket(unchar *ether, IPaddr_t dest, int dport,
			int sport, int payload_len);

/* Processes a received packet */
extern void NetReceive(unchar *, int);

/*
 * Check if autoload is enabled. If so, use either NFS or TFTP to download
 * the boot file.
 */
void net_auto_load(void);

/*
 * The following functions are a bit ugly, but necessary to deal with
 * alignment restrictions on ARM.
 *
 * We're using inline functions, which had the smallest memory
 * footprint in our tests.
 */
/* return IP *in network byteorder* */
static inline IPaddr_t NetReadIP(void *from)
{
	IPaddr_t ip;

	memcpy((void *)&ip, (void *)from, sizeof(ip));
	return ip;
}

/* return ulong *in network byteorder* */
static inline ulong NetReadLong(ulong *from)
{
	ulong l;

	memcpy((void *)&l, (void *)from, sizeof(l));
	return l;
}

/* write IP *in network byteorder* */
static inline void NetWriteIP(void *to, IPaddr_t ip)
{
	memcpy(to, (void *)&ip, sizeof(ip));
}

/* copy IP */
static inline void NetCopyIP(void *to, void *from)
{
	memcpy((void *)to, from, sizeof(IPaddr_t));
}

/* copy ulong */
static inline void NetCopyLong(ulong *to, ulong *from)
{
	memcpy((void *)to, (void *)from, sizeof(ulong));
}

/* Convert an IP address to a string */
extern void ip_to_string(IPaddr_t x, char *s);

/* Convert a string to ip address */
extern IPaddr_t string_to_ip(const char *s);

/* Convert a VLAN id to a string */
extern void VLAN_to_string(ushort x, char *s);

/* Convert a string to a vlan id */
extern ushort string_to_VLAN(const char *s);

/* read a VLAN id from an environment variable */
extern ushort getenv_VLAN(char *);

/* copy a filename (allow for "..." notation, limit length) */
extern void copy_filename(char *dst, const char *src, int size);

/* get a random source port */
extern unsigned int random_port(void);

/**********************************************************************/

#endif /* __NET_H__ */
