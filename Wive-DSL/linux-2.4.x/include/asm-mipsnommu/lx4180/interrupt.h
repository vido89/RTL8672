#ifndef _ASICREGS_H
#define _ASICREGS_H
#include <asm/mach-realtek/rtl8672/platform.h>


#undef SWTABLE_NO_HW

/* Basic features
*/
#define MAX_PORT_NUMBER                 6
#define PORT_NUM_MASK                   7
#define ALL_PORT_MASK                   0x3F



/* Memory mapping of tables 
*/
enum {
    TYPE_L2_SWITCH_TABLE = 0,
    TYPE_ARP_TABLE,
    TYPE_L3_ROUTING_TABLE,
    TYPE_MULTICAST_TABLE,
    TYPE_PROTOCOL_TRAP_TABLE,
    TYPE_VLAN_TABLE,
    TYPE_EXT_INT_IP_TABLE,
    TYPE_ALG_TABLE,
    TYPE_SERVER_PORT_TABLE,
    TYPE_L4_TCP_UDP_TABLE,
    TYPE_L4_ICMP_TABLE,
    TYPE_PPPOE_TABLE,
    TYPE_ACL_RULE_TABLE
};

#ifndef SWTABLE_NO_HW
#define SWCORE_BASE                 0xBC800000
#else
#define SWCORE_BASE                 0x80600000
#endif /*SWTABLE_NO_HW*/
/* Table access and CPU interface control registers
*/
#define TACI_BASE                   (SWCORE_BASE + 0x00000000)
#define SWTACR                      (0x000 + TACI_BASE)     /* Table Access Control */
#define SWTASR                      (0x004 + TACI_BASE)     /* Table Access Status */
#define SWTAA                       (0x008 + TACI_BASE)     /* Table Access Address */
#define TCR0                        (0x020 + TACI_BASE)     /* Table Access Control 0 */
#define TCR1                        (0x024 + TACI_BASE)     /* Table Access Control 1 */
#define TCR2                        (0x028 + TACI_BASE)     /* Table Access Control 2 */
#define TCR3                        (0x02C + TACI_BASE)     /* Table Access Control 3 */
#define TCR4                        (0x030 + TACI_BASE)     /* Table Access Control 4 */
#define TCR5                        (0x034 + TACI_BASE)     /* Table Access Control 5 */
#define TCR6                        (0x038 + TACI_BASE)     /* Table Access Control 6 */
#define TCR7                        (0x03C + TACI_BASE)     /* Table Access Control 7 */
/* Table access control register field definitions
*/
#define ACTION_MASK                 1
#define ACTION_DONE                 0
#define ACTION_START                1
#define CMD_MASK                    (7 << 1)
#define CMD_ADD                     (1 << 1)
#define CMD_MODIFY                  (2 << 1)
#define CMD_FORCE                   (4 << 1)
/* Table access status register field definitions 
*/
#define TABSTS_MASK                 1
#define TABSTS_SUCCESS              0
#define TABSTS_FAIL                 1
/* Vlan table access definitions 
*/
#define STP_DISABLE                 0
#define STP_BLOCK                   1
#define STP_LEARN                   2
#define STP_FORWARD                 3
/* Protocol trapping table access definitions
*/
#define TYPE_TRAP_ETHERNET          0x00
#define TYPE_TRAP_IP                0x02
#define TYPE_TRAP_TCP               0x05
#define TYPE_TRAP_UDP               0x06
/* L3 Routing table access definitions
*/
#define PROCESS_PPPOE               0x00
#define PROCESS_DIRECT              0x01
#define PROCESS_INDIRECT            0x02
#define PROCESS_S_CPU               0x04
#define PROCESS_N_CPU               0x05
#define PROCESS_S_DROP              0x06
#define PROCESS_N_DROP              0x07
/* ACL table access definitions
*/
#define RULE_ETHERNET               0x00
#define RULE_IP                     0x02
#define RULE_IFSEL                  0x03
#define RULE_ICMP                   0x04
#define RULE_IGMP                   0x05
#define RULE_TCP                    0x06
#define RULE_UDP                    0x07
#define ACTION_PERMIT               0x00
#define ACTION_REDIRECT             0x01
#define ACTION_S_CPU                0x02
#define ACTION_N_CPU                0x03
#define ACTION_S_DROP               0x04
#define ACTION_N_DROP               0x05
#define ACTION_MIRROR               0x06
#define ACTION_PPPOE_REDIRECT       0x07



/* MIB counter registers
*/
#define MIB_COUNTER_BASE                            (SWCORE_BASE + 0x00001000)
#define ETHER_STATS_OCTETS                          (0x000 + MIB_COUNTER_BASE)
#define ETHER_STATS_DROP_EVENTS                     (0x004 + MIB_COUNTER_BASE)
#define ETHER_STATS_CRC_ALIGN_ERRORS                (0x008 + MIB_COUNTER_BASE)
#define ETHER_STATS_FRAGMENTS                       (0x00C + MIB_COUNTER_BASE)
#define ETHER_STATS_JABBERS                         (0x010 + MIB_COUNTER_BASE)
#define IF_IN_UCAST_PKTS                            (0x014 + MIB_COUNTER_BASE)
#define ETHER_STATS_MULTICAST_PKTS                  (0x018 + MIB_COUNTER_BASE)
#define ETHER_STATS_BROADCAST_PKTS                  (0x01C + MIB_COUNTER_BASE)
#define ETHER_STATS_UNDERSIZE_PKTS                  (0x020 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_64_OCTETS                  (0x024 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_65_TO_127_OCTETS           (0x028 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_128_TO_255_OCTETS          (0x02C + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_256_TO_511_OCTETS          (0x030 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_512_TO_1023_OCTETS         (0x034 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_1024_TO_1518_OCTETS        (0x038 + MIB_COUNTER_BASE)
#define ETHER_STATS_OVERSIZE_PKTS                   (0x03C + MIB_COUNTER_BASE)
#define DOT3_CONTROL_IN_UNKNOWN_OPCODES             (0x040 + MIB_COUNTER_BASE)
#define DOT3_IN_PAUSE_FRAMES                        (0x044 + MIB_COUNTER_BASE)
#define IF_OUT_OCTETS                               (0x048 + MIB_COUNTER_BASE)
#define IF_OUT_UCAST_PKTS                           (0x04C + MIB_COUNTER_BASE)
#define IF_OUT_MULTICASTCAST_PKTS                   (0x050 + MIB_COUNTER_BASE)
#define IF_OUT_BROADCASTCAST_PKTS                   (0x054 + MIB_COUNTER_BASE)
#define DOT3_STATS_LATE_COLLISIONS                  (0x05C + MIB_COUNTER_BASE)
#define DOT3_STATS_DEFERRED_TRANSMISSIONS           (0x05C + MIB_COUNTER_BASE)
#define ETHER_STATS_COLLISIONS                      (0x060 + MIB_COUNTER_BASE)
#define DOT3_STATS_SINGLE_COLLISION_FRAMES          (0x064 + MIB_COUNTER_BASE)
#define DOT3_STATS_MULTIPLE_COLLISION_FRAMES        (0x068 + MIB_COUNTER_BASE)
#define DOT3_OUT_PAUSE_FRAMES                       (0x06C + MIB_COUNTER_BASE)
#define MIB_CONTROL                                 (0x070 + MIB_COUNTER_BASE)
/* MIB control register field definitions 
*/
#define IN_COUNTER_RESTART                          (1 << 31)
#define OUT_COUNTER_RESTART                         (1 << 30)
#define PORT_FOR_COUNTING_MASK                      0x3F000000
#define PORT_FOR_COUNTING_OFFSET                    24



/* PHY control registers 
*/
#define PHY_BASE                                    (SWCORE_BASE + 0x00002000)
#define PORT0_PHY_CONTROL                           (0x000 + PHY_BASE)
#define PORT0_PHY_STATUS                            (0x004 + PHY_BASE)
#define PORT0_PHY_IDENTIFIER_1                      (0x008 + PHY_BASE)
#define PORT0_PHY_IDENTIFIER_2                      (0x00C + PHY_BASE)
#define PORT0_PHY_AUTONEGO_ADVERTISEMENT            (0x010 + PHY_BASE)
#define PORT0_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x014 + PHY_BASE)
#define PORT1_PHY_CONTROL                           (0x020 + PHY_BASE)
#define PORT1_PHY_STATUS                            (0x024 + PHY_BASE)
#define PORT1_PHY_IDENTIFIER_1                      (0x028 + PHY_BASE)
#define PORT1_PHY_IDENTIFIER_2                      (0x02C + PHY_BASE)
#define PORT1_PHY_AUTONEGO_ADVERTISEMENT            (0x030 + PHY_BASE)
#define PORT1_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x034 + PHY_BASE)
#define PORT2_PHY_CONTROL                           (0x040 + PHY_BASE)
#define PORT2_PHY_STATUS                            (0x044 + PHY_BASE)
#define PORT2_PHY_IDENTIFIER_1                      (0x048 + PHY_BASE)
#define PORT2_PHY_IDENTIFIER_2                      (0x04C + PHY_BASE)
#define PORT2_PHY_AUTONEGO_ADVERTISEMENT            (0x050 + PHY_BASE)
#define PORT2_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x054 + PHY_BASE)
#define PORT3_PHY_CONTROL                           (0x060 + PHY_BASE)
#define PORT3_PHY_STATUS                            (0x064 + PHY_BASE)
#define PORT3_PHY_IDENTIFIER_1                      (0x068 + PHY_BASE)
#define PORT3_PHY_IDENTIFIER_2                      (0x06C + PHY_BASE)
#define PORT3_PHY_AUTONEGO_ADVERTISEMENT            (0x070 + PHY_BASE)
#define PORT3_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x074 + PHY_BASE)
#define PORT4_PHY_CONTROL                           (0x080 + PHY_BASE)
#define PORT4_PHY_STATUS                            (0x084 + PHY_BASE)
#define PORT4_PHY_IDENTIFIER_1                      (0x088 + PHY_BASE)
#define PORT4_PHY_IDENTIFIER_2                      (0x08C + PHY_BASE)
#define PORT4_PHY_AUTONEGO_ADVERTISEMENT            (0x090 + PHY_BASE)
#define PORT4_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x094 + PHY_BASE)
#define PORT5_PHY_CONTROL                           (0x0A0 + PHY_BASE)
#define PORT5_PHY_STATUS                            (0x0A4 + PHY_BASE)
#define PORT5_PHY_IDENTIFIER_1                      (0x0A8 + PHY_BASE)
#define PORT5_PHY_IDENTIFIER_2                      (0x0AC + PHY_BASE)
#define PORT5_PHY_AUTONEGO_ADVERTISEMENT            (0x0B0 + PHY_BASE)
#define PORT5_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x0B4 + PHY_BASE)
/* PHY control register field definitions 
*/
#define PHY_RESET                                   (1 << 15)
#define ENABLE_LOOPBACK                             (1 << 14)
#define SPEED_SELECT_100M                           (1 << 13)
#define SPEED_SELECT_10M                            0
#define ENABLE_AUTONEGO                             (1 << 12)
#define POWER_DOWN                                  (1 << 11)
#define ISOLATE_PHY                                 (1 << 10)
#define RESTART_AUTONEGO                            (1 << 9)
#define SELECT_FULL_DUPLEX                          (1 << 8)
#define SELECT_HALF_DUPLEX                          0
/* PHY status register field definitions 
*/
#define STS_CAPABLE_100BASE_T4                      (1 << 15)
#define STS_CAPABLE_100BASE_TX_FD                   (1 << 14)
#define STS_CAPABLE_100BASE_TX_HD                   (1 << 13)
#define STS_CAPABLE_100BASE_T_FD                    (1 << 12)
#define STS_CAPABLE_100BASE_T_HD                    (1 << 11)
#define STS_MF_PREAMBLE_SUPPRESSION                 (1 << 6)
#define STS_AUTONEGO_COMPLETE                       (1 << 5)
#define STS_REMOTE_FAULT                            (1 << 4)
#define STS_CAPABLE_NWAY_AUTONEGO                   (1 << 3)
#define STS_LINK_ESTABLISHED                        (1 << 2)
#define STS_JABBER_DETECTED                         (1 << 1)
#define STS_CAPABLE_EXTENDED                        (1 << 0)
/* PHY identifier 1 
*/
#define OUT_3_18_MASK                               (0xFFFF << 16)
#define OUT_3_18_OFFSET                             16
#define OUT_19_24_MASK                              (0x3F << 10)
#define OUT_19_24_OFFSET                            10
#define MODEL_NUMBER_MASK                           (0x3F << 4)
#define MODEL_NUMBER_OFFSET                         4
#define REVISION_NUMBER_MASK                        0x0F
#define REVISION_NUMBER_OFFSET                      0
/* PHY auto-negotiation advertisement and 
link partner ability registers field definitions
*/
#define NEXT_PAGE_ENABLED                           (1 << 15)
#define ACKNOWLEDGE                                 (1 << 14)
#define REMOTE_FAULT                                (1 << 13)
#define CAPABLE_PAUSE                               (1 << 10)
#define CAPABLE_100BASE_T4                          (1 << 9)
#define CAPABLE_100BASE_TX_FD                       (1 << 8)
#define CAPABLE_100BASE_TX_HD                       (1 << 7)
#define CAPABLE_100BASE_T_FD                        (1 << 6)
#define CAPABLE_100BASE_T_HD                        (1 << 5)
#define SELECTOR_MASK                               0x1F
#define SELECTOR_OFFSET                             0



/* CPU interface Tx/Rx packet registers 
*/
#define CPU_IFACE_BASE                      (SWCORE_BASE + 0x00004000)
#define CPUICR                              (0x000 + CPU_IFACE_BASE)    /* Interface control */
#define CPURPDCR                            (0x004 + CPU_IFACE_BASE)    /* Rx pkthdr descriptor control */
#define CPURMDCR                            (0x008 + CPU_IFACE_BASE)    /* Rx mbuf descriptor control */
#define CPUTPDCR                            (0x00C + CPU_IFACE_BASE)    /* Tx pkthdr descriptor control */
#define CPUIIMR                             (0x010 + CPU_IFACE_BASE)    /* Interrupt mask control */
#define CPUIISR                             (0x014 + CPU_IFACE_BASE)    /* Interrupt status control */
/* CPU interface control register field definitions 
*/
#if 0
#define TXCMD                               (1 << 31)       /* Enable Tx */
#define RXCMD                               (1 << 30)       /* Enable Rx */
#define TXFD                                (1 << 29)       /* Notify Tx descriptor fetch */
#define SOFTRST                             (1 << 28)       /* Re-initialize all descriptors */
#define STOPTX                              (1 << 27)       /* Stop Tx */
#define SWINTSET                            (1 << 26)       /* Set software interrupt */
#define BUSBURST_32WORDS                    0
#define BUSBURST_64WORDS                    (1 << 24)
#define BUSBURST_128WORDS                   (2 << 24)
#define BUSBURST_256WORDS                   (3 << 24)
#define MBUF_128BYTES                       0
#define MBUF_256BYTES                       (1 << 21)
#define MBUF_512BYTES                       (2 << 21)
#define MBUF_1024BYTES                      (3 << 21)
#define MBUF_2048BYTES                      (4 << 21)
#else
#define TXCMD                               (1 << 31)       /* Enable Tx */
#define RXCMD                               (1 << 30)       /* Enable Rx */
#define BUSBURST_32WORDS                    0
#define BUSBURST_64WORDS                    (1 << 28)
#define BUSBURST_128WORDS                   (2 << 28)
#define BUSBURST_256WORDS                   (3 << 28)
#define MBUF_128BYTES                       0
#define MBUF_256BYTES                       (1 << 24)
#define MBUF_512BYTES                       (2 << 24)
#define MBUF_1024BYTES                      (3 << 24)
#define MBUF_2048BYTES                      (4 << 24)
#define TXFD                                (1 << 23)       /* Notify Tx descriptor fetch */
#define SOFTRST                             (1 << 22)       /* Re-initialize all descriptors */
#define STOPTX                              (1 << 21)       /* Stop Tx */
#define SWINTSET                            (1 << 20)       /* Set software interrupt */
#define LBMODE                              (1 << 19)       /* Loopback mode */
#define LB10MHZ                             (1 << 18)       /* LB 10MHz */
#define LB100MHZ                            (1 << 18)       /* LB 100MHz */
#endif
/* CPU interface descriptor field defintions 
*/
#define DESC_OWNED_BIT                      1
#define DESC_RISC_OWNED                     0
#define DESC_SWCORE_OWNED                   1
#define DESC_WRAP                           (1 << 1)
/* CPU interface interrupt mask register field definitions 
*/
#define LINK_CHANG_IE                       (1 << 31)    /* Link change interrupt enable */
#define RX_ERR_IE                           (1 << 30)    /* Rx error interrupt enable */
#define TX_ERR_IE                           (1 << 29)    /* Tx error interrupt enable */
#define SW_INT_IE                           (1 << 28)    /* Software interrupt enable */
#define L4_COL_REMOVAL_IE                   (1 << 27)    /* L4 collision removal interrupt enable */
#define PKTHDR_DESC_RUNOUT_IE               (1 << 23)    /* Run out pkthdr descriptor interrupt enable */
#define MBUF_DESC_RUNOUT_IE                 (1 << 22)    /* Run out mbuf descriptor interrupt enable */
#define TX_DONE_IE                          (1 << 21)    /* Tx one packet done interrupt enable */
#define RX_DONE_IE                          (1 << 20)    /* Rx one packet done interrupt enable */
#define TX_ALL_DONE_IE                      (1 << 19)    /* Tx all packets done interrupt enable */
/* CPU interface interrupt status register field definitions 
*/
#define LINK_CHANG_IP                       (1 << 31)   /* Link change interrupt pending */
#define RX_ERR_IP                           (1 << 30)   /* Rx error interrupt pending */
#define TX_ERR_IP                           (1 << 29)   /* Tx error interrupt pending */
#define SW_INT_IP                           (1 << 28)   /* Software interrupt pending */
#define L4_COL_REMOVAL_IP                   (1 << 27)   /* L4 collision removal interrupt pending */
#define PKTHDR_DESC_RUNOUT_IP               (1 << 23)   /* Run out pkthdr descriptor interrupt pending */
#define MBUF_DESC_RUNOUT_IP                 (1 << 22)   /* Run out mbuf descriptor interrupt pending */
#define TX_DONE_IP                          (1 << 21)   /* Tx one packet done interrupt pending */
#define RX_DONE_IP                          (1 << 20)   /* Rx one packet done interrupt pending */
#define TX_ALL_DONE_IP                      (1 << 19)   /* Tx all packets done interrupt pending */
#define INTPENDING_NIC_MASK     (RX_ERR_IP | TX_ERR_IP | PKTHDR_DESC_RUNOUT_IP | \
                                    MBUF_DESC_RUNOUT_IP | TX_DONE_IP | RX_DONE_IP)



/* System control registers 
*/
#define SYSTEM_BASE                         (SWCORE_BASE + 0x00003000)
#define MACCR                               (0x000 + SYSTEM_BASE)   /* MAC control */
#define MACMR                               (0x004 + SYSTEM_BASE)   /* MAC monitor */
#define VLANTCR                             (0x008 + SYSTEM_BASE)   /* Vlan tag control */
#define DSCR0                               (0x00C + SYSTEM_BASE)   /* Qos by DS control */
#define DSCR1                               (0x010 + SYSTEM_BASE)   /* Qos by DS control */
#define QOSCR                               (0x014 + SYSTEM_BASE)   /* Qos control */
#define MISCCR                              (0x018 + SYSTEM_BASE)   /* Switch core misc control */
#define SWTMCR                              (0x01C + SYSTEM_BASE)   /* Switch table misc control */
#define TMRMR                               (0x020 + SYSTEM_BASE)   /* Test mode Rx mii-like */
#define TMTMR                               (0x024 + SYSTEM_BASE)   /* Test mode Tx mii-like */
#define TMCR                                (0x028 + SYSTEM_BASE)   /* Test mode control */
#define GMHR                                (0x02C + SYSTEM_BASE)   /* Gateway MAC high */
#define GMLR                                (0x030 + SYSTEM_BASE)   /* Gateway MAC low */
/* MAC control register field definitions 
*/
#define DIS_IPG                             (1 << 31)   /* Set IFG */
#define EN_INT_CAM                          (1 << 30)   /* Enable internal CAM */
#define NORMAL_BACKOFF                      (1 << 29)   /* Normal back off slot timer */
#define ACPT_MAXLEN_1536                    0           /* Accepted max length of packets */
#define ACPT_MAXLEN_1552                    (1 << 26)
#define FULL_RST                            (1 << 25)   /* Reset all tables & queues */
#define SEMI_RST                            (1 << 24)   /* Reset queues */
//#define BCAST_TO_CPU                        (1 << 23)   /* Broadcast to CPU */
//#define DIS_ACPT_BCAST                      (1 << 22)   /* Disable accept broadcast packets */
/* MAC monitor register field definitions 
*/
#define EN_MON_ID_BUS                       (1 << 31)   /* Enable monitor ID bus for ASIC debug */
#define EN_MON_PKT_BUS                      (1 << 30)   /* Enable monitor packet bus for ASIC debug */
/* VLAN tag control register field definitions 
*/
//#define NO_INS_REM_TAG                      3           /* Do not insert/remove tag */
//#define INS_TAG                             2           /* Insert tag */
//#define INS_TAG_HQ_ONLY                     1           /* Insert tag from high queue only */
//#define REM_TAG                             0           /* Remove tag */
//#define VLAN_TAG_P0_OFFSET                  30          /* Port 0 offset */
//#define VLAN_TAG_P1_OFFSET                  28          /* Port 1 offset */
//#define VLAN_TAG_P2_OFFSET                  26          /* Port 2 offset */
//#define VLAN_TAG_P3_OFFSET                  24          /* Port 3 offset */
//#define VLAN_TAG_P4_OFFSET                  22          /* Port 4 offset */
//#define VLAN_TAG_P5_OFFSET                  20          /* Port 5 offset */
#define VLAN_TAG_ONLY                       (1 << 19)   /* Only accept tagged packets */
/* Qos by DS control register 
*/
#define DS0_7                               0xFF000000
#define DS8_15                              0x00FF0000
#define DS16_23                             0x0000FF00
#define DS24_31                             0x000000FF
#define DS32_39                             0xFF000000
#define DS40_47                             0x00FF0000
#define DS48_55                             0x0000FF00
#define DS56_63                             0x000000FF
/* Qos control register 
*/
#define QWEIGHT_MASK                        (3 << 30)
#define QWEIGHT_ALWAYS_H                    (3 << 30)   /* Weighted round robin of priority always high first */
#define QWEIGHT_16TO1                       (2 << 30)   /* Weighted round robin of priority queue 16:1 */
#define QWEIGHT_8O1                         (1 << 30)   /* Weighted round robin of priority queue 8:1 */
#define QWEIGHT_4TO1                        0           /* Weighted round robin of priority queue 4:1 */
#define EN_FCA_AUTOOFF                      (1 << 29)   /* Enable flow control auto off */
#define DIS_DS_PRI                          (1 << 28)   /* Disable DS priority */
#define DIS_VLAN_PRI                        (1 << 27)   /* Disable 802.1p priority */
#define PORT5_H_PRI                         (1 << 26)   /* Port 5 high priority */
#define PORT4_H_PRI                         (1 << 25)   /* Port 4 high priority */
#define PORT3_H_PRI                         (1 << 24)   /* Port 3 high priority */
#define PORT2_H_PRI                         (1 << 23)   /* Port 2 high priority */
#define PORT1_H_PRI                         (1 << 22)   /* Port 1 high priority */
#define PORT0_H_PRI                         (1 << 21)   /* Port 0 high priority */
#define QOS_PORT_OFFSET                     21
/* Switch core misc control register field definitions 
*/
//#define SOFT_RST_CORE                       (1 << 31)   /* Soft reset */
#define DIS_P5_LOOPBACK                     (1 << 30)   /* Disable port 5 loopback */
#define P5_LINK_MII_MAC                     0           /* Port 5 MII MAC type */
#define P5_LINK_MII_PHY                     1           /* Port 5 MII PHY type */
#define P5_LINK_SNI_MAC                     2           /* Port 5 SNI MAC type */
#define P5_LINK_OFFSET                      28          /* Port 5 link type offset */
//#define P4_USB_SEL                          (1 << 25)   /* Select port USB interface */
#define EN_P5_LINK_PHY                      (1 << 26)   /* Enable port 5 PHY provides link status to MAC */
#define EN_P4_LINK_PHY                      (1 << 25)   /* Enable port 4 PHY provides link status to MAC */
#define EN_P3_LINK_PHY                      (1 << 24)   /* Enable port 3 PHY provides link status to MAC */
#define EN_P2_LINK_PHY                      (1 << 23)   /* Enable port 2 PHY provides link status to MAC */
#define EN_P1_LINK_PHY                      (1 << 22)   /* Enable port 1 PHY provides link status to MAC */
#define EN_P0_LINK_PHY                      (1 << 21)   /* Enable port 0 PHY provides link status to MAC */
/* Switch table misc control register field definitions 
*/
#define NAPTR_NOT_FOUND_TO_CPU              0           /* Reverse NAPT not found to CPU */
#define NAPTR_NOT_FOUND_DROP                (1 << 0)    /* Reverse NAPT not found to S_DROP */
#define EN_NAPT_AUTO_LEARN                  (1 << 1)    /* Enable NAPT auto learn */
#define EN_NAPT_AUTO_DELETE                 (1 << 2)    /* Enable NAPT auto delete */
#define EN_VLAN_INGRESS_FILTER              (1 << 3)    /* Enable Vlan ingress filtering */
#define EN_VLAN_EGRESS_FILTER               (1 << 4)    /* Enable Vlan egress filtering */
#define WAN_ROUTE_MASK                      (3 << 5)
#define WAN_ROUTE_FORWARD                   0           /* Route WAN packets */
#define WAN_ROUTE_TO_CPU                    (1 << 5)    /* Forward WAN packets to CPU */
#define WAN_ROUTE_DROP                      (2 << 5)    /* Drop WAN packets */
#define MCAST_STP_STS_MASK                  (0x3ff << 7)
#define MCAST_STP_STS_P0_OFFSET             7           /* Multicast spanning tree status port 0 */
#define MCAST_STP_STS_P1_OFFSET             9           /* Multicast spanning tree status port 1 */
#define MCAST_STP_STS_P2_OFFSET             11          /* Multicast spanning tree status port 2 */
#define MCAST_STP_STS_P3_OFFSET             13          /* Multicast spanning tree status port 3 */
#define MCAST_STP_STS_P4_OFFSET             15          /* Multicast spanning tree status port 4 */
#define MCAST_STP_STS_P5_OFFSET             17          /* Multicast spanning tree status port 5 */
#define MCAST_PORT_EXT_MODE_OFFSET          19          /* Multicast port mode offset */
#define MCAST_PORT_EXT_MODE_MASK            (0x3f << 19)    /* Multicast port mode mask */
#define NAPTF2CPU                           (1 << 25)   /* Trap packets not in TCP/UDP/ICMP format and 
                                                        destined to the interface required to do NAPT */
#define EN_MCAST                            (1 << 26)   /* Enable Multicast Table */
#define BCAST_TO_CPU                        (1 << 28)   /* If EN_BCAST is not set, trap broadcast packets 
                                                        to CPU */
#define MCAST_TO_CPU                        (1 << 29)   /* If EN_MCAST is not set, trap multicast packets 
                                                        to CPU */
#define EN_BCAST                            (1 << 30)   /* Enable Broadcast Handling */
/* Test mode Rx MII-like register field definitions 
*/
#define P5_RXDV                             (1 << 30)   /* Enable port 0 MII RXDV signal */
#define P4_RXDV                             (1 << 29)   /* Enable port 1 MII RXDV signal */
#define P3_RXDV                             (1 << 28)   /* Enable port 2 MII RXDV signal */
#define P2_RXDV                             (1 << 27)   /* Enable port 3 MII RXDV signal */
#define P1_RXDV                             (1 << 26)   /* Enable port 4 MII RXDV signal */
#define P0_RXDV                             (1 << 25)   /* Enable port 5 MII RXDV signal */
#define NIBBLE_MASK                         0x0F        /* Mask for a nibble */
#define P5_RXD_OFFSET                       20          /* Port 0 RXD MII signal */
#define P4_RXD_OFFSET                       16          /* Port 1 RXD MII signal */
#define P3_RXD_OFFSET                       12          /* Port 2 RXD MII signal */
#define P2_RXD_OFFSET                       8           /* Port 3 RXD MII signal */
#define P1_RXD_OFFSET                       4           /* Port 4 RXD MII signal */
#define P0_RXD_OFFSET                       0           /* Port 5 RXD MII signal */
/* Test mode Tx MII-like register field definitions 
*/
#define P5_TXDV                             (1 << 30)   /* Enable port 0 MII TXDV signal */
#define P4_TXDV                             (1 << 29)   /* Enable port 1 MII TXDV signal */
#define P3_TXDV                             (1 << 28)   /* Enable port 2 MII TXDV signal */
#define P2_TXDV                             (1 << 27)   /* Enable port 3 MII TXDV signal */
#define P1_TXDV                             (1 << 26)   /* Enable port 4 MII TXDV signal */
#define P0_TXDV                             (1 << 25)   /* Enable port 5 MII TXDV signal */
#define P5_TXD_OFFSET                       20          /* Port 0 TXD MII signal */
#define P4_TXD_OFFSET                       16          /* Port 1 TXD MII signal */
#define P3_TXD_OFFSET                       12          /* Port 2 TXD MII signal */
#define P2_TXD_OFFSET                       8           /* Port 3 TXD MII signal */
#define P1_TXD_OFFSET                       4           /* Port 4 TXD MII signal */
#define P0_TXD_OFFSET                       0           /* Port 5 TXD MII signal */
/* Test mode enable register 
*/
#define TX_TEST_PORT_OFFSET                 26          /* Tx test mode enable port offset */
#define RX_TEST_PORT_OFFSET                 18          /* Rx test mode enable port offset */
/* Gateway MAC low register 
*/
#define GMACL_OFFSET                        20          /* Gateway MAC[15:4] offset */
#define GMACL_MASK                          (0xFFFFFFFF << 20)  /* Gateway MAC[15:4] mask */



/* Miscellaneous control registers 
*/
#define MISC_BASE                           (SWCORE_BASE + 0x00005000)
#define LEDCR                               (0x000 + MISC_BASE)     /* LED control */
#define PSCR                                (0x004 + MISC_BASE)     /* Power saving control */
#define BISTCR                              (0x008 + MISC_BASE)     /* BIST control */
#define BWCR                                (0x00C + MISC_BASE)     /* Bandwidth control */
#define CSCR                                (0x010 + MISC_BASE)     /* Checksum control */
#define FCREN                               (0x014 + MISC_BASE)     /* Flow control enable control */
#define FCRTH                               (0x018 + MISC_BASE)     /* Flow control threshold */
#define PTCR                                (0x01C + MISC_BASE)     /* Port trunk control */
#define PPPCR                               (0x020 + MISC_BASE)     /* PPP control */
#define PTRAPCR                             (0x024 + MISC_BASE)     /* Protocol trapping control */
#define STCR                                (0x028 + MISC_BASE)     /* Spanning tree control */
#define TTLCR                               (0x02C + MISC_BASE)     /* TTL control */
#define MSCR                                (0x030 + MISC_BASE)     /* Module switch control */
#define BSCR                                (0x038 + MISC_BASE)     /* Broadcast storm control */
#define TEATCR                              (0x03C + MISC_BASE)     /* Table entry aging time control */
#define PMCR                                (0x040 + MISC_BASE)     /* Port mirror control */
#define PPMAR                               (0x044 + MISC_BASE)     /* Per port matching action */
#define PATP0                               (0x048 + MISC_BASE)     /* Pattern for port 0 */
#define PATP1                               (0x04C + MISC_BASE)     /* Pattern for port 1 */
#define PATP2                               (0x050 + MISC_BASE)     /* Pattern for port 2 */
#define PATP3                               (0x054 + MISC_BASE)     /* Pattern for port 3 */
#define PATP4                               (0x058 + MISC_BASE)     /* Pattern for port 4 */
#define PATP5                               (0x05C + MISC_BASE)     /* Pattern for port 5 */
#define MASKP0                              (0x060 + MISC_BASE)     /* Mask for port 0 */
#define MASKP1                              (0x064 + MISC_BASE)     /* Mask for port 1 */
#define MASKP2                              (0x068 + MISC_BASE)     /* Mask for port 2 */
#define MASKP3                              (0x06C + MISC_BASE)     /* Mask for port 3 */
#define MASKP4                              (0x070 + MISC_BASE)     /* Mask for port 4 */
#define MASKP5                              (0x074 + MISC_BASE)     /* Mask for port 5 */
#define PVCR                                (0x078 + MISC_BASE)     /* Port based vlan config */
#define GIDXMCR                             (0x07C + MISC_BASE)     /* GIDX mapping control */
#define OCR                                 (0x080 + MISC_BASE)     /* Offset control */
/* LED control register field definitions 
*/
#define LED_P0_COL                          (1 << 0)    /* LED port 0 collision */
#define LED_P0_ACT                          (1 << 1)    /* LED port 0 active */
#define LED_P0_100M                         (1 << 2)    /* LED port 0 speed 100M */
#define LED_P1_COL                          (1 << 3)    /* LED port 1 collision */
#define LED_P1_ACT                          (1 << 4)    /* LED port 1 active */
#define LED_P1_100M                         (1 << 5)    /* LED port 1 speed 100M */
#define LED_P2_COL                          (1 << 6)    /* LED port 2 collision */
#define LED_P2_ACT                          (1 << 7)    /* LED port 2 active */
#define LED_P2_100M                         (1 << 8)    /* LED port 2 speed 100M */
#define LED_P3_COL                          (1 << 9)    /* LED port 3 collision */
#define LED_P3_ACT                          (1 << 10)   /* LED port 3 active */
#define LED_P3_100M                         (1 << 11)   /* LED port 3 speed 100M */
#define LED_P4_COL                          (1 << 12)   /* LED port 4 collision */
#define LED_P4_ACT                          (1 << 13)   /* LED port 4 active */
#define LED_P4_100M                         (1 << 14)   /* LED port 4 speed 100M */
#define LED_P5_COL                          (1 << 15)   /* LED port 5 collision */
#define LED_P5_ACT                          (1 << 16)   /* LED port 5 active */
#define LED_P5_100M                         (1 << 17)   /* LED port 5 speed 100M */
#define LED_CPU_CTRL                        (1 << 18)   /* CPU control LED */
/* Power saving control register field definitions 
*/
#define EN_POWER_SAVE                       (1 << 0)    /* Enable power saving mode */
/* BIST control register field definitions 
*/
#define PAGE_BIST_ERR_MASK                  0x1F        /* Page error bit map mask */
#define PAGE_BIST_ERR_OFFSET                0           /* Page error bit map offset */
#define TXQ_BIST_ERR_MASK                   (0x1F << 5) /* Tx queue error bit map mask */
#define TXQ_BIST_ERR_OFFSET                 5           /* Tx queue error bit map offset */
#define L2_BIST_ERR_MASK                    (1 << 10)   /* L2 error bit map mask */
#define L2_BIST_ERR_OFFSET                  10          /* L2 error bit map offset */
#define L4_BIST_ERR_MASK                    (1 << 11)  /* L4 error bit map mask */
#define L4_BIST_ERR_OFFSET                  11          /* L4 error bit map offset */
/* Bandwidth control register field definitions 
*/
#define EN_BC_PORT_MASK                     0x3F        /* Bandwidth control port mask */
#define BC_P0_OFFSET                        6           /* Bandwidth control port 0 offset */
#define BC_P1_OFFSET                        9           /* Bandwidth control port 1 offset */
#define BC_P2_OFFSET                        12          /* Bandwidth control port 2 offset */
#define BC_P3_OFFSET                        15          /* Bandwidth control port 3 offset */
#define BC_P4_OFFSET                        18          /* Bandwidth control port 4 offset */
#define BC_P5_OFFSET                        21          /* Bandwidth control port 5 offset */
#define BW_FULL_RATE                        0
#define BW_128K                             1
#define BW_256K                             2
#define BW_512K                             3
#define BW_1M                               4
#define BW_2M                               5
#define BW_4M                               6
#define BW_8M                               7
/* Checksum control register field definitions 
*/
#define ALLOW_L3_CHKSUM_ERR                 (1 << 0)    /* Allow L3 checksum error */
#define ALLOW_L4_CHKSUM_ERR                 (1 << 1)    /* Allow L4 checksum error */
#define EN_ETHER_L3_CHKSUM_REC              (1 << 2)    /* Enable L3 checksum recalculation for ethernet port */
#define EN_ETHER_L4_CHKSUM_REC              (1 << 3)    /* Enable L4 checksum recalculation for ethernet port */
#define EN_CPU_L3_CHKSUM_REC                (1 << 4)    /* Enable L3 checksum recalculation for CPU port */
#define EN_CPU_L4_CHKSUM_REC                (1 << 5)    /* Enable L4 checksum recalculation for CPU port */
/* Flow control enable register field defintions 
*/
#define EN_P0_IN_Q_FC                       (1 << 31)   /* Enable port 0 input flow control */
#define EN_P1_IN_Q_FC                       (1 << 30)   /* Enable port 1 input flow control */
#define EN_P2_IN_Q_FC                       (1 << 29)   /* Enable port 2 input flow control */
#define EN_P3_IN_Q_FC                       (1 << 28)   /* Enable port 3 input flow control */
#define EN_P4_IN_Q_FC                       (1 << 27)   /* Enable port 4 input flow control */
#define EN_P5_IN_Q_FC                       (1 << 26)   /* Enable port 5 input flow control */
#define EN_IN_Q_FC_PORT_OFFSET              26
#define EN_P0_OUT_Q_FC                      (1 << 25)   /* Enable port 0 output flow control */
#define EN_P1_OUT_Q_FC                      (1 << 24)   /* Enable port 1 output flow control */
#define EN_P2_OUT_Q_FC                      (1 << 23)   /* Enable port 2 output flow control */
#define EN_P3_OUT_Q_FC                      (1 << 22)   /* Enable port 3 output flow control */
#define EN_P4_OUT_Q_FC                      (1 << 21)   /* Enable port 4 output flow control */
#define EN_P5_OUT_Q_FC                      (1 << 20)   /* Enable port 5 output flow control */
#define EN_OUT_Q_FC_PORT_OFFSET             20
#define EN_CPU_OUT_Q_FC                     (1 << 19)   /* Enable CPU output flow control */
#define FC_LAUNCH_PORT_OFFSET               13          /* CPU launches flow control on port offset */
/* Flow control enable register field defintions 
*/
#define PER_PORT_BUF_FC_TH_MASK             0xFF        /* Per port buffer page flow control threshold mask */
#define IN_Q_PER_PORT_BUF_FC_THH_OFFSET     24          /* InQ per port buffer page flow control high threshold offset */
#define IN_Q_PER_PORT_BUF_FC_THL_OFFSET     16          /* InQ per port buffer page flow control low threshold offset */
#define OUT_Q_PER_PORT_BUF_FC_THH_OFFSET    8           /* OutQ per port buffer page flow control high threshold offset */
#define OUT_Q_PER_PORT_BUF_FC_THL_OFFSET    0           /* OutQ per port buffer page flow control low threshold offset */
/* Port trunking control register field definitions 
*/
#define LMPR7_OFFSET                        27          /* Physical port index for logical port 7 */
#define LMPR6_OFFSET                        24          /* Physical port index for logical port 6 */
#define LMPR5_OFFSET                        21          /* Physical port index for logical port 5 */
#define LMPR4_OFFSET                        18          /* Physical port index for logical port 4 */
#define LMPR3_OFFSET                        15          /* Physical port index for logical port 3 */
#define LMPR2_OFFSET                        12          /* Physical port index for logical port 2 */
#define LMPR1_OFFSET                        9           /* Physical port index for logical port 1 */
#define LMPR0_OFFSET                        6           /* Physical port index for logical port 0 */
#define TRUNK1_PORT_MASK_OFFSET             0           /* Physical port mask of trunk 1 */
/* Port trunking control register field definitions 
*/
#define EN_PPP_OP                           (1 << 31)   /* Enable PPPoE auto insert and remove */
/* Protocol trapping control register field definitions 
*/
#define EN_ARP_TRAP                         (1 << 31)   /* Enable trapping ARP packets */
#define EN_RARP_TRAP                        (1 << 30)   /* Enable trapping RARP packets */
#define EN_PPPOE_TRAP                       (1 << 29)   /* Enable trapping PPPoE packets */
#define EN_IGMP_TRAP                        (1 << 28)   /* Enable trapping IGMP packets */
#define EN_DHCP_TRAP1                       (1 << 27)   /* Enable trapping DHCP 67 packets */
#define EN_DHCP_TRAP2                       (1 << 26)   /* Enable trapping DHCP 68 packets */
#define EN_OSPF_TRAP                        (1 << 25)   /* Enable trapping OSPF packets */
#define EN_RIP_TRAP                         (1 << 24)   /* Enable trapping RIP packets */
/* Spanning tree control register field definitions 
*/
#define EN_ESTP_S_DROP                      (1 << 31)   /* Enable egress spanning tree forward S_Drop */
/* TTL control register field definitions 
*/
#define DIS_TTL1                            (1 << 31)   /* Disable TTL-1 operation */
/* Module switch control register field definitions 
*/
#define MOD_MASK                            7
#define EN_L2                               (1 << 0)    /* Enable L2 module */
#define EN_L3                               (1 << 1)    /* Enable L3 module */
#define EN_L4                               (1 << 2)    /* Enable L4 module */
#define EN_OUT_ACL                          (1 << 3)    /* Enable egress ACL */
#define EN_IN_ACL                           (1 << 4)    /* Enable ingress ACL */
#define EN_STP                              (1 << 7)    /* Enable spanning tree */
#define FORCE_TO_CPU                        (1 << 8)
#define L2_COL_BCAST                        (1 << 9)    /* L2 collision broadcast */
#define I8021D_TO_CPU                       (1 << 10)   /* 802.1D trap to CPU */
#define GARP_TO_CPU                         (1 << 11)   /* GARP trap to CPU */
/* Broadcast storm control register field definitions 
*/
#define EN_BCAST_STORM                      (1 << 0)    /* Enable broadcast storm control */
#define BCAST_TH_MASK                       (0xFF << 1) /* Threshold within broadcast interval mask */
#define BCAST_TH_OFFSET                     1           /* Threshold within broadcast interval offset */
#define TI_100M_MASK                        (0x3F << 9) /* Time interval for 100M mask */
#define TI_100M_OFFSET                      9           /* Time interval for 100M offset */
#define TI_10M_MASK                         (0x3F << 15)/* Time interval for 10M mask */
#define TI_10M_OFFSET                       15          /* Time interval for 10M offset */
/* Table entry aging time control register field definitions 
*/
#define ICMP_TH_OFFSET                      26          /* ICMP timeout threshold offset */
#define ICMP_TH_MASK                        (0x3f << ICMP_TH_OFFSET)
#define UDP_TH_OFFSET                       20          /* UDP timeout threshold offset */
#define UDP_TH_MASK                         (0x3f << UDP_TH_OFFSET)
#define TCP_LONG_TH_OFFSET                  14          /* TCP long timeout threshold offset */
#define TCP_LONG_TH_MASK                    (0x3f << TCP_LONG_TH_OFFSET)
#define TCP_MED_TH_OFFSET                   8           /* TCP medium timeout threshold offset */
#define TCP_MED_TH_MASK                     (0x3f << TCP_MED_TH_OFFSET)
#define TCP_FAST_TH_OFFSET                  2           /* TCP fast timeout threshold offset */
#define TCP_FAST_TH_MASK                    (0x3f << TCP_FAST_TH_OFFSET)
/* Port mirror control register field definitions 
*/
#define MIRROR_TO_PORT_OFFSET               29          /* Port receiving the mirrored traffic offset */
#define SRC_MIRROR                          (1 << 28)   /* Source mirror */
#define DEST_MIRROR                         0           /* Destination mirror */
#define MIRROR_FROM_PORT_LIST_OFFSET        22          /* Port list to be mirrored offset */
#define EN_PPORT_PMATCH_MIRROR              (1 << 21)   /* Enable per port pattern match mirror */
/* Per port matching action register field definitions 
*/
#define EN_PMATCH_PORT_LIST_OFFSET          0           /* Enable pattern match port list offset */
#define MATCH_S_DROP                        0           /* S_DROP if matched */
#define MATCH_MIRROR_TO_CPU                 (1 << 6)    /* Mirror to CPU if matched */
#define MATCH_FORWARD_TO_CPU                (2 << 6)    /* Forward to CPU if matched */
#define MATCH_TO_MIRROR_PORT                (3 << 6)    /* To mirror port if matched */
#define MATCH_OP_MASK                       (3 << 6)    /* Operation if matched mask */
#define MATCH_OP_OFFSET                     6           /* Operation if matched offset */
/* Port based vlan config register field definitions 
*/
#define PVID_MASK                           7           /* MASK for PVID */
#define VIDP0_OFFSET                        0           /* Vlan table index for port 0 */
#define VIDP1_OFFSET                        3           /* Vlan table index for port 1 */
#define VIDP2_OFFSET                        6           /* Vlan table index for port 2 */
#define VIDP3_OFFSET                        9           /* Vlan table index for port 3 */
#define VIDP4_OFFSET                        12          /* Vlan table index for port 4 */
#define VIDP5_OFFSET                        15          /* Vlan table index for port 5 */
/* GIDX mapping control register field definitions 
*/
#define GIDX_MASK                           7           /* MASK for GIDX */
#define GIDX0_OFFSET                        0           /* Index to IP table */
#define GIDX1_OFFSET                        3           /* Index to IP table */
#define GIDX2_OFFSET                        6           /* Index to IP table */
#define GIDX3_OFFSET                        9           /* Index to IP table */
#define GIDX4_OFFSET                        12          /* Index to IP table */
#define GIDX5_OFFSET                        15          /* Index to IP table */
#define GIDX6_OFFSET                        18          /* Index to IP table */
#define GIDX7_OFFSET                        21          /* Index to IP table */
/* Offset control register field definitions 
*/
#define OCR_START_MASK                      (0x1f << 27)    /* Starting value of offset mask */
#define OCR_START_OFFSET                    27              /* Starting value of offset offset */
#define OCR_END_MASK                        (0x1f << 22)    /* End value of offset mask */
#define OCR_END_OFFSET                      27              /* End value of offset offset */



/* PPT registers 
*/
#define PPT_BASE                            0xBD010000
#define PDTR                                (0x000 + PPT_BASE)      /* Data register */
#define CTR                                 (0x002 + PPT_BASE)      /* Control register */
#define EPPAR                               (0x003 + PPT_BASE)      /* EPP address register */
#define EPPDR                               (0x004 + PPT_BASE)      /* EPP data register */
#define PPCR                                (0x008 + PPT_BASE)      /* Printer port control register */
#define DFIFO                               (0x400 + PPT_BASE)      /* Data FIFO */
#define CFGRA                               (0x400 + PPT_BASE)      /* Configuration A */
#define CFGRB                               (0x401 + PPT_BASE)      /* Configuration B */
#define ECR                                 (0x402 + PPT_BASE)      /* Extended control register */
/* Status register field definitions 
*/
#define TIME_OUT                            (1 << 0)
#define IRQ_MASK                            (1 << 2)    /* Interrupt (low active) mask */
#define ERR_MASK                            (1 << 3)    /* Error (low active) mask */
#define SLCT                                (1 << 4)    /* Printer selected and online */
#define PE                                  (1 << 5)    /* End of paper */
#define ACK_MASK                            (1 << 6)    /* Char reception complete (low active) mask */
#define BUSY_MASK                           (1 << 7)    /* Busy (low active) mask */
/* Control register field definitions 
*/
#define DATA_STROBE                         (1 << 0)    /* Data strobe signal */
#define AFD                                 (1 << 1)    /* Automatic line feed */
#define INIT_MASK                           (1 << 2)    /* Initialize (low active) mask */
#define SLIN                                (1 << 3)    /* Printer selected and online */
#define IE                                  (1 << 4)    /* Interrupt enable */
#define DIRC_INPUT                          (1 << 5)    /* Direction control input */
#define DIRC_OUTPUT                         0           /* Direction control output */
/* Extended control register field definitions 
*/
#define STANDARD_MODE                       0           /* Standard mode */
#define PS2_MODE                            (1 << 5)    /* PS/2 mode */
#define PARALLEL_FIFO_MODE                  (2 << 5)    /* Parallel port FIFO mode */
#define ECP_FIFO_MODE                       (3 << 5)    /* ECP FIFO mode */
#define EN_ECP_INTR                         (1 << 4)    /* Enable ECP interrupt */
#define ECP_SERVICE                         (1 << 2)    /* ECP service bit */
#define FIFO_FULL                           (1 << 1)    /* FIFO full */
#define FIFO_EMPTY                          (1 << 0)    /* FIFO empty */
/* Printer port control register 
*/
#define ENPPT                               (1 << 0)    /* Enable PPT */
#define ENEPP                               (1 << 1)    /* Enable EPP */
#define ENECP                               (1 << 2)    /* Enable ECP */
#define EXTENDIF                            (1 << 3)    /* Extended mode */
#define INTR_TRIG_1BYTE                     0           /* Interrupt trigger level 1 byte */
#define INTR_TRIG_4BYTE                     (1 << 4)    /* Interrupt trigger level 4 byte */
#define INTR_TRIG_8BYTE                     (2 << 4)    /* Interrupt trigger level 8 byte */
#define INTR_TRIG_14BYTE                    (3 << 4)    /* Interrupt trigger level 14 byte */



/* UART registers 
*/
#define UART1_BASE                          0xB9C00000
//#define UART2_BASE                          0xBD011010
#define RBR                                 0x000       /* Rx buffer */
#define THR                                 0x000       /* Tx holding */
#define DLL                                 0x000       /* Divisor latch LSB */
#if 0
#define IER                                 0x001       /* Interrupt enable */
#define DLM                                 0x001       /* Divisor latch MSB */
#define IIR                                 0x002       /* Interrupt identification */
#define FCR                                 0x002       /* FIFO control */
#define LCR                                 0x003       /* Line control */
#define MCR                                 0x004       /* Modem control */
#define LSR                                 0x005       /* Line status */
#define MSR                                 0x006       /* Modem status */
#define SCR                                 0x007       /* Scratchpad */
#else
#define IER                                 0x004       /* Interrupt enable */
#define DLM                                 0x004       /* Divisor latch MSB */
#define IIR                                 0x008       /* Interrupt identification */
#define FCR                                 0x008       /* FIFO control */
#define LCR                                 0x00C       /* Line control */
#define MCR                                 0x010       /* Modem control */
#define LSR                                 0x014       /* Line status */
#define MSR                                 0x018       /* Modem status */
#define SCR                                 0x01C       /* Scratchpad */
#endif
/* Line Control Register 
*/
#define LCR_WLN         0x03
#define CHAR_LEN_5      0x00
#define CHAR_LEN_6      0x01
#define CHAR_LEN_7      0x02
#define CHAR_LEN_8      0x03
#define LCR_STB         0x04
#define ONE_STOP        0x00
#define TWO_STOP        0x04
#define LCR_PEN         0x08
#define LCR_EPS         0x30
#define PARITY_ODD      0x00
#define PARITY_EVEN     0x10
#define PARITY_MARK     0x20
#define PARITY_SPACE    0x30
#define PARITY_NONE     0x80
#define LCR_SBRK        0x40
#define LCR_DLAB        0x80
#define DLAB            LCR_DLAB
/* Line Status Register 
*/
#define LSR_DR          0x01
#define RxCHAR_AVAIL    LSR_DR
#define LSR_OE          0x02
#define LSR_PE          0x04
#define LSR_FE          0x08
#define LSR_BI          0x10
#define LSR_THRE        0x20
#define LSR_TEMT        0x40
#define LSR_FERR        0x80
/* Interrupt Identification Register 
*/
#define IIR_IP          0x01
#define IIR_ID          0x0e
#define IIR_RLS         0x06
#define Rx_INT          IIR_RLS
#define IIR_RDA         0x04
#define RxFIFO_INT      IIR_RDA
#define IIR_THRE        0x02
#define TxFIFO_INT      IIR_THRE
#define IIR_MSTAT       0x00
#define IIR_TIMEOUT     0x0c
/* Interrupt Enable Register 
*/
#define IER_ERBI        0x01
#define IER_ETBEI       0x02
#define IER_ELSI        0x04
#define IER_EDSSI       0x08
#define IER_ESLP        0x10
#define IER_ELP         0x20
/* Modem Control Register 
*/
#define MCR_DTR         0x01
#define DTR             MCR_DTR
#define MCR_RTS         0x02
#define MCR_OUT1        0x04
#define MCR_OUT2        0x08
#define MCR_LOOP        0x10
/* Modem Status Register 
*/
#define MSR_DCTS        0x01
#define MSR_DDSR        0x02
#define MSR_TERI        0x04
#define MSR_DDCD        0x08
#define MSR_CTS         0x10
#define MSR_DSR         0x20
#define MSR_RI          0x40
#define MSR_DCD         0x80
/* FIFO Control Register 
*/
#define FCR_EN          0x01
#define FIFO_ENABLE     FCR_EN
#define FCR_RXCLR       0x02
#define RxCLEAR         FCR_RXCLR
#define FCR_TXCLR       0x04
#define TxCLEAR         FCR_TXCLR
#define FCR_DMA         0x08
#define FCR_RXTRIG_L    0x40
#define FCR_RXTRIG_H    0x80



/* Global interrupt control registers 
*/
#define GICR_BASE                           0xB9C03000
#define GIMR                                (0x010 + GICR_BASE)       /* Global interrupt mask */
#define GISR                                (0x012 + GICR_BASE)       /* Global interrupt status */
#define IRR1                                (0x014 + GICR_BASE)       /* Interrupt routing 1 */
#define IRR2                                (0x018 + GICR_BASE)       /* Interrupt routing 2 */
#define ILR                                 (0x019 + GICR_BASE)       /* Interrupt level register */
#define IMR                                 (0x01A + GICR_BASE)       /* Interrupt module */
/* Global interrupt mask register field definitions 
*/
#define UART_IM                             (1 << 15)       /* UART interrupt enable */
#define Timer_IM                            (1 << 14)       /* Timer interrupt enable */
#define Time_Out_IM                         (1 << 13)       /* Time out interrupt enable */
#define SAR_IM                              (1 << 12)       /* SAR interrupt enable */
#define Ethernet_IM                         (1 << 11)       /* Ethernet interrupt enable */
#define DMT_IM                              (1 << 10)       /* DMT interrupt enable */
#define USB_IM                              (1 <<  9)       /* USB client interrupt enable */
#define PCI_IM                              (1 <<  8)       /* PCI interrupt enable */
#define GPIO_IM                             (1 <<  7)       /* GPIO port interrupt enable */
/* Global interrupt status register field definitions 
*/
#define UART_IS                             (1 << 15)       /* UART interrupt pending */
#define Timer_IS                            (1 << 14)       /* Timer/Counter interrupt pending */
#define Time_Out_IS                         (1 << 13)       /* time out interrupt pending */
#define SAR_IS                              (1 << 12)       /* SAR interrupt pending */
#define Ethernet_IS                         (1 << 11)       /* Ethernet interrupt pending */
#define DMT_IS                              (1 << 10)       /* DMT interrupt pending */
#define USB_IS                              (1 <<  9)       /* USB client interrupt pending */
#define PCI_IS                              (1 <<  8)       /* PCI interrupt pending */
#define GPIO_IS                             (1 <<  7)       /* GPIO interrupt pending */
/* Interrupt routing register 1 field definitions 
*/
#define UART_IPS                            28              /* UART interrupt routing select offset */
#define Timer_IPS                           24              /* Timer interrupt routing select offset */
#define Time_Out_IPS                        20              /* Time out interrupt routing select offset */
#define SAR_IPS                             16              /* SAR interrupt routing select offset */
#define Ethernet_IPS                        12              /* Ethernet interrupt routing select offset */
#define DMT_IPS                              8              /* DMT interrupt routing select offset */
#define USB_IPS                              4              /* USB interrupt routing select offset */
#define PCI_IPS                              0              /* PCI interrupt routing select offset */
/* Interrupt routing register 2 field definitions 
*/
#define GPIO_IPS                             7              /* GPIO interrupt routing select offset */


/* Timer control registers 
*/
#define GPIOCR_BASE 0xB9C01000
#define TC0DATA                             (0x020 + GPIOCR_BASE)       /* Timer/Counter 0 data */
#define TC1DATA                             (0x024 + GPIOCR_BASE)       /* Timer/Counter 1 data */
#define TC0CNT                              (0x028 + GPIOCR_BASE)       /* Timer/Counter 0 count */
#define TC1CNT                              (0x02C + GPIOCR_BASE)       /* Timer/Counter 1 count */
#define TCCNR                               (0x030 + GPIOCR_BASE)       /* Timer/Counter control */
#define TCIR                                (0x034 + GPIOCR_BASE)       /* Timer/Counter intertupt */
#define CDBR                                (0x038 + GPIOCR_BASE)       /* Clock division base */
#define WDTCNR                              (0x03C + GPIOCR_BASE)       /* Watchdog timer control */
#define BSTMOUT                             (0x040 + GPIOCR_BASE)       /* bus time out interval */
/* Timer/Counter data register field definitions 
*/
#define TCD_OFFSET                          8
/* Timer/Counter control register field defintions 
*/
#define TC0EN                               (1 << 31)       /* Timer/Counter 0 enable */
#define TC0MODE_COUNTER                     0               /* Timer/Counter 0 counter mode */
#define TC0MODE_TIMER                       (1 << 30)       /* Timer/Counter 0 timer mode */
#define TC1EN                               (1 << 29)       /* Timer/Counter 1 enable */
#define TC1MODE_COUNTER                     0               /* Timer/Counter 1 counter mode */
#define TC1MODE_TIMER                       (1 << 28)       /* Timer/Counter 1 timer mode */
/* Timer/Counter interrupt register field definitions 
*/
#define TCIR_TC0IE                          (1 << 31)       /* Timer/Counter 0 interrupt enable */
#define TCIR_TC1IE                          (1 << 30)       /* Timer/Counter 1 interrupt enable */
#define TCIR_TC0IP                          (1 << 29)       /* Timer/Counter 0 interrupt pending */
#define TCIR_TC1IP                          (1 << 28)       /* Timer/Counter 1 interrupt pending */
/* Clock division base register field definitions 
*/
#define DIVF_OFFSET                         16
/* Watchdog control register field definitions 
*/
#define WDTE_OFFSET                         24              /* Watchdog enable */
#define WDSTOP_PATTERN                      0xA5            /* Watchdog stop pattern */
#define WDTCLR                              (1 << 23)       /* Watchdog timer clear */
#define OVSEL_13                            0               /* Overflow select count 2^13 */
#define OVSEL_14                            (1 << 21)       /* Overflow select count 2^14 */
#define OVSEL_15                            (2 << 21)       /* Overflow select count 2^15 */
#define OVSEL_16                            (3 << 21)       /* Overflow select count 2^16 */

/*Bus time out register
*/
#define BSTMOUT_OFFSET                       29              
#define BSTMOUT_20us                         0               /* bus time out interval 20 us*/
#define BSTMOUT_30us                         (1 << BSTMOUT_OFFSET)       /* bus time out interval 30 us*/
#define BSTMOUT_40us                         (2 << BSTMOUT_OFFSET)       /* bus time out interval 40 us*/
#define BSTMOUT_50us                         (3 << BSTMOUT_OFFSET)       /* bus time out interval 50 us*/


/* GPIO control registers 
*/
#define PBCNR                               (0x00C + GPIOCR_BASE)     /* Port B control */
#define PBDIR                               (0x010 + GPIOCR_BASE)     /* Port B direction */
#define PBDAT                               (0x014 + GPIOCR_BASE)     /* Port B data */
#define PBISR                               (0x018 + GPIOCR_BASE)     /* Port B interrupt status */
#define PBIMR                               (0x01C + GPIOCR_BASE)     /* Port B interrupt mask */


/* Memory config registers 
*/
#define MEMCFG_BASE                         0xBD013000
#define MCFGR                               (0x000 + MEMCFG_BASE)       /* Memory config */
#define MTCR                                (0x004 + MEMCFG_BASE)       /* Memory timing config */
/* Memory config register field definitions 
*/
#define ROM_256K                            0               /* ROM size 256K */
#define ROM_512K                            (1 << 30)       /* ROM size 512K */
#define ROM_1M                              (2 << 30)       /* ROM size 1M */
#define ROM_2M                              (3 << 30)       /* ROM size 2M */
#define SDRAM_16M                           0               /* SDRAM size 512Kx16x2 */
#define SDRAM_64M                           (1 << 28)       /* SDRAM size 1Mx16x4 */
#define SDRAM_128M                          (2 << 28)       /* SDRAM size 2Mx16x4 */
#define CAS_LAT_2                           0               /* CAS latency = 2 */
#define CAS_LAT_3                           (1 << 27)       /* CAS latency = 3 */
#define B0_BUSWIDTH_8BIT                    0               /* Bank 0 bus width 8 bits */
#define B0_BUSWIDTH_16BIT                   (1 << 25)       /* Bank 0 bus width 16 bits */
#define B0_BUSWIDTH_32BIT                   (2 << 25)       /* Bank 0 bus width 32 bits */
#define B1_BUSWIDTH_8BIT                    0               /* Bank 1 bus width 8 bits */
#define B1_BUSWIDTH_16BIT                   (1 << 23)       /* Bank 1 bus width 16 bits */
#define B1_BUSWIDTH_32BIT                   (2 << 23)       /* Bank 1 bus width 32 bits */
#define SDBUSWID_16BIT                      0               /* SDRAM bus width 16 bits */
#define SDBUSWID_32BIT                      (1 << 24)       /* SDRAM bus width 32 bits */
#define MEMCLK_2_LXCLK_1_1                  0               /* Memory clock to lexra bus clock 1:1 */
#define MEMCLK_2_LXCLK_1_2                  (1 << 23)       /* Memory clock to lexra bus clock 1:2 */
#define CLKRATE_200                         0               /* Bus clock 200 */
#define CLKRATE_100                         (1 << 20)       /* Bus clock 100 */
#define CLKRATE_50                          (2 << 20)       /* Bus clock 50 */
#define CLKRATE_25                          (3 << 20)       /* Bus clock 25 */
#define CLKRATE_12_5                        (4 << 20)       /* Bus clock 12.5 */
#define CLKRATE_6_25                        (5 << 20)       /* Bus clock 6.25 */
/* Memory timing config register field definitions 
*/
#define CE0T_CS_OFFSET                      28
#define CE0T_WP_OFFSET                      24
#define CE1T_CS_OFFSET                      20
#define CE1T_WP_OFFSET                      16
#define CE23T_RAS_OFFSET                    10
#define CE23T_RFC_OFFSET                    6
#define CE23T_RP_OFFSET                     4



#endif   /* _ASICREGS_H */

