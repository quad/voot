/*  rtl8139c.h

    $Id: rtl8139c.h,v 1.6 2002/06/29 12:57:04 quad Exp $

*/

#ifndef __COMMON_RTL8139C_H__
#define __COMMON_RTL8139C_H__

#include "vars.h"
#include "ether.h"
#include "system.h"
#include "asic.h"

/* NOTE: PCI/G2 register definitions. */

#define PCI_IDENT_STR       (REGISTER(char)     0xa1001400)
#define G2_BYTE(idx)        (REGISTER(uint8)    0xa1000000)[(idx)]
#define G2_SHORT(idx)       (REGISTER(uint16)   0xa1000000)[(idx)/2]
#define G2_INT(idx)         (REGISTER(uint32)   0xa1000000)[(idx)/4]

/*
    NOTE: RTL8139c IO and DMA definitions.
*/

#define RTL_IO_BYTE(idx)    (REGISTER(uint8)    0xa1001700)[(idx)]
#define RTL_IO_SHORT(idx)   (REGISTER(uint16)   0xa1001700)[(idx)/2]
#define RTL_IO_INT(idx)     (REGISTER(uint32)   0xa1001700)[(idx)/4]

#define RTL_DMA_BYTE        (REGISTER(uint8)    0xa1840000)
#define RTL_DMA_SHORT       (REGISTER(uint16)   0xa1840000)
#define RTL_DMA_INT         (REGISTER(uint32)   0xa1840000)

/*
    NOTE: RTL8139c register definitions.

    CREDIT: Imported from dc-load-ip 1.0.1.
*/

#define RTL_IDR0                0x00            /* Mac address (32 + ) */
#define RTL_IDR1                0x04            /* Mac address (16 = 48 bits)*/
#define RTL_MAR0                0x08            /* Multicast filter */
#define RTL_TXSTATUS0           0x10            /* Transmit status (4x32 bit regs) */
#define RTL_TXADDR0             0x20            /* Tx descriptor 0 (32 bit) */
#define RTL_TXADDR1             0x24            /* Tx descriptor 1 (32 bit) */
#define RTL_TXADDR2             0x28            /* Tx descriptor 2 (32 bit) */
#define RTL_TXADDR3             0x2C            /* Tx descriptor 3 (32 bit) */
#define RTL_RXBUF               0x30            /* Receive buffer start address */
#define RTL_RXEARLYCNT          0x34            /* Early Rx byte count */
#define RTL_RXEARLYSTATUS       0x36            /* Early Rx status */
#define RTL_CHIPCMD             0x37            /* Command register */
#define RTL_RXBUFTAIL           0x38            /* Current address of packet read (queue tail) */
#define RTL_RXBUFHEAD           0x3A            /* Current buffer address (queue head) */
#define RTL_INTRMASK            0x3C            /* Interrupt mask */
#define RTL_INTRSTATUS          0x3E            /* Interrupt status */
#define RTL_TXCONFIG            0x40            /* Tx config */
#define RTL_RXCONFIG            0x44            /* Rx config */
#define RTL_TIMER               0x48            /* A general purpose counter */
#define RTL_RXMISSED            0x4C            /* 24 bits valid, write clears */
#define RTL_CFG9346             0x50            /* 93C46 command register */
#define RTL_CONFIG0             0x51            /* Configuration reg 0 */
#define RTL_CONFIG1             0x52            /* Configuration reg 1 */
#define RTL_TIMERINT            0x54            /* Timer interrupt register (32 bits) */
#define RTL_MEDIASTATUS         0x58            /* Media status register */
#define RTL_CONFIG3             0x59            /* Config register 3 */
#define RTL_CONFIG4             0x5A            /* Config register 4 */
#define RTL_MULTIINTR           0x5C            /* Multiple interrupt select */
#define RTL_MII_TSAD            0x60            /* Transmit status of all descriptors (16 bits) */
#define RTL_MII_BMCR            0x62            /* Basic Mode Control register (16 bits) */
#define RTL_MII_BMSR            0x64            /* Basic Mode Status register (16 bits) */
#define RTL_AS_ADVERT           0x66            /* Auto-negotiation advertisement reg (16 bits) */
#define RTL_AS_LPAR             0x68            /* Auto-negotiation link partner reg (16 bits) */
#define RTL_AS_EXPANSION        0x6A            /* Auto-negotiation expansion reg (16 bits) */

/*
    NOTE: RTL8193c command bits.

    OR these together and write the resulting value into CHIPCMD to execute
    it.
*/

#define RTL_CMD_RESET           0x10
#define RTL_CMD_RX_ENABLE       0x08
#define RTL_CMD_TX_ENABLE       0x04
#define RTL_CMD_RX_BUF_EMPTY    0x01

/* NOTE: RTL8193c MII BMCR bits; OR these together. */

#define RTL_BMCR_RESET          0x8000          /* Reset PHY registers */
#define RTL_BMCR_ANE            0x1000          /* Automatic Negotiation Enable */
#define RTL_BMCR_RAN            0x200           /* Reset Auto Negotiation */

/* NOTE: 93C456 command bits; OR these together. */

#define RTL_9346_EEM1           0x40            /* Enable EEM1 operating mode register */
#define RTL_9346_EEM0           0x80            /* Enable EEM0 operating mode register */

/* NOTE: RTL8193c config bits; OR these together. */

#define RTL_C4_RX_AUTOCLR       0x80            /* RTL8193c will clear Rx overflow automatically */
#define RTL_RX_TOALL            0x08            /* Accept broadcast packets */
#define RTL_RX_TOMULTI          0x04            /* Accept multicast match packets */
#define RTL_RX_TOUS             0x02            /* Accept physical match packets */

/*
    NOTE: RTL8139c interrupt status bits.

    To clear an interrupt write a true back on the status bit.
*/

#define RTL_INT_PCIERR          0x8000          /* PCI Bus error */
#define RTL_INT_TIMEOUT         0x4000          /* Set when TCTR reaches TimerInt value */
#define RTL_INT_RXFIFO_OVERFLOW 0x0040          /* Rx FIFO overflow */
#define RTL_INT_RXFIFO_UNDERRUN 0x0020          /* Packet underrun / link change */
#define RTL_INT_LINKCHG         0x0020
#define RTL_INT_RXBUF_OVERFLOW  0x0010          /* Rx BUFFER overflow */
#define RTL_INT_TX_ERR          0x0008
#define RTL_INT_TX_OK           0x0004
#define RTL_INT_RX_ERR          0x0002
#define RTL_INT_RX_OK           0x0001

/* NOTE: RTL8139c transmit status bits. */

#define RTL_TX_CARRIER_LOST     0x80000000      /* Carrier sense lost */
#define RTL_TX_ABORTED          0x40000000      /* Transmission aborted */
#define RTL_TX_OUT_OF_WINDOW    0x20000000      /* Out of window collision */
#define RTL_TX_STATUS_OK        0x00008000      /* Status ok: a good packet was transmitted */
#define RTL_TX_UNDERRUN         0x00004000      /* Transmit FIFO underrun */
#define RTL_TX_HOST_OWNS        0x00002000      /* Set to 1 when DMA operation is completed */
#define RTL_TX_SIZE_MASK        0x00001fff      /* Descriptor size mask */

/* NOTE: RTL8139C receive status bits. */

#define RTL_RX_MULTICAST        0x00008000      /* Multicast packet */
#define RTL_RX_PAM              0x00004000      /* Physical address matched */
#define RTL_RX_BROADCAST        0x00002000      /* Broadcast address matched */
#define RTL_RX_BAD_SYMBOL       0x00000020      /* Invalid symbol in 100TX packet */
#define RTL_RX_RUNT             0x00000010      /* Packet size is <64 bytes */
#define RTL_RX_TOO_LONG         0x00000008      /* Packet size is >4K bytes */
#define RTL_RX_CRC_ERR          0x00000004      /* CRC error */
#define RTL_RX_FRAME_ALIGN      0x00000002      /* Frame alignment error */
#define RTL_RX_STATUS_OK        0x00000001      /* Status ok: a good packet was received */

/*
    NOTE: RTL_TXCONFIG_ON in human language.

    Append CRC
    1024b Tx DMA bursts
    Retry Tx 16 times MAX
*/

#define RTL_TXCONFIG_ON         0x00000600

/*
    NOTE: RTL_RXCONFIG_ON in human language.

    1024 bytes Max Rx DMA burst
    16 byte Rx FIFO threshold
    16k + 16 bytes Rx DMA buffer length

    (16k is the largest we can have because we're restricted to 0x6000 bytes
    by the DMA size.)
*/

#define RTL_RXCONFIG_ON         0x00000e00
#define RX_BUFFER_LEN           16384
#define RX_BUFFER_THRESHOLD     16

#define RTL_DMA_FRAME_COPYING   0xfff0          /* in the frame status, means the DMA is still working */

/* NOTE: Main RTL module structure. */

typedef struct
{
    uint16          cur_rx;                 /* NOTE: Current Rx DMA buffer tail index. */
    uint16          cur_tx;                 /* NOTE: Current Tx descriptor. */
    uint8           mac[ETHER_MAC_SIZE];    /* NOTE: Cached MAC address. */

    asic_handler_f  old_rtl_handler;
} rtl_t;

/* NOTE: Module definitions. */

bool    rtl_init        (void);
bool    rtl_tx          (const uint8 *header, uint32 header_length, const uint8 *data, uint32 data_length);
uint8 * rtl_mac         (void);

#endif
