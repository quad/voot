/*  rtl8139c.c

    $Id: rtl8139c.c,v 1.5 2002/06/23 03:22:52 quad Exp $

DESCRIPTION

    Driver code for the RealTek 8139c.

    This card is a very confusing card but I hope to write my driver
    "cleaner" than the ones before it and thus make it a much better
    reference doc for people out there.

    CREDIT: RealTek (http://www.realtek.com.tw/) has been great and released
    Engrish documentation about their card.

    CREDIT: Andrew has also contributed to this card with great reference
    code and even better personal support in #dcdev@EFNet. Give the guy a
    damn hand.

*/

#include "vars.h"
#include "system.h"
#include "asic.h"
#include "exception-lowlevel.h"
#include "util.h"
#include "malloc.h"
#include "video.h"

#include "rtl8139c.h"

rtl_t   rtl_info;

/* RTL8139c Tx descriptor indexes */
static uint8* rtl_tx_descs[] =  {
                                    (uint8 *) 0xa1846000,
                                    (uint8 *) 0xa1846800,
                                    (uint8 *) 0xa1847000,
                                    (uint8 *) 0xa1847800
                                };

/*
    NOTE: GAPS PCI Controller and BBA PCI Configuration

    pc_* is all code that is completely magic.
    
    My main gripe with it is not it being magic but rather it assumes that
    because there is a GAPS PCI controller - there is a BBA. In all honesty,
    there will probably never be another PCI device on the Dreamcast... but
    I just don't feel comfortable not checking the PCI identification codes.

    However, you can fit what I know about writing code for PCI in a thimble
    and still have room for the air, so I think I'll shut up until someone
    much more talented than I comes along and fixes up this code.
*/

bool pci_detect (void)
{
    const char  gapspci_ident_str[] = "GAPSPCI_BRIDGE_2";

    /* NOTE: Must be re-casted to char * so we can const-ify it cleanly. Grr. */

    return !memcmp ((char *) PCI_IDENT_STR, gapspci_ident_str, sizeof (gapspci_ident_str));
}

/* All of the following code is *very* magic - even more magical than the above code. */
bool pci_bb_init (void)
{
    uint32  count;

    /* STAGE: Initialize the "GAPS" PCI glue controller */

    G2_INT(0x1418) = 0x5a14a501;

    count = 10000;
    while (!(G2_INT(0x1418) & 1) && count > 0)
        count--;

    if (!(G2_INT(0x1418) & 1))
        return FALSE;

    G2_INT(0x1420) = 0x01000000;
    G2_INT(0x1424) = 0x01000000;
    G2_INT(0x1428) = 0x01840000;                /* DMA Base */
    G2_INT(0x142c) = 0x01840000 + 32 * 1024;    /* DMA End - 32k buffer includes Rx Ring and Tx descriptors */
    G2_INT(0x1414) = 0x00000001;                /* Interrupt enable. */
    G2_INT(0x1434) = 0x00000001;

    /*
        STAGE: Initialize the PCI Bridge
    
        This is the part which *should* be more intelligent, but isn't.
    */

    G2_SHORT(0x1606) = 0xf900;
    G2_INT(0x1630)   = 0x00000000;
    G2_BYTE(0x163c)  = 0x00;
    G2_BYTE(0x160d)  = 0xf0;

    (void) G2_SHORT(0x04);

    G2_SHORT(0x1604) = 0x0006;
    G2_INT(0x1614)   = 0x01000000;

    (void) G2_BYTE(0x1650);

    return TRUE;
}

/*
    NOTE: Real-Tek 8139C Ethernet Network Interface Card

    This is where the real fun begins. Documentation will come.
*/

static void rtl_get_mac (void)
{
    unsigned tmp;

    /* STAGE: Copy over the first segment */

    tmp = RTL_IO_INT(RTL_IDR0);

    rtl_info.mac[0] = tmp & 0xff;
    rtl_info.mac[1] = (tmp >> 8) & 0xff;
    rtl_info.mac[2] = (tmp >> 16) & 0xff;
    rtl_info.mac[3] = (tmp >> 24) & 0xff;

    /* STAGE: Copy over the second segment. */

    tmp = RTL_IO_INT(RTL_IDR1);
    rtl_info.mac[4] = tmp & 0xff;
    rtl_info.mac[5] = (tmp >> 8) & 0xff;
}

uint8* rtl_mac (void)
{
    return rtl_info.mac;
}

static void rtl_soft_reset (void)
{
    RTL_IO_BYTE(RTL_CHIPCMD) = RTL_CMD_RESET;

    while (RTL_IO_BYTE(RTL_CHIPCMD) & RTL_CMD_RESET);
}

static void rtl_send_command (uint8 command)
{
    RTL_IO_BYTE(RTL_CHIPCMD) = command;

    /* NOTE: Ignore all but known command registers */

    while ((RTL_IO_BYTE(RTL_CHIPCMD) & ~0xe3) != command);
}

static void rtl_stop (void)
{
    /* STAGE: Disable packet reception. */

    RTL_IO_INT(RTL_RXCONFIG) &= ~(RTL_RX_TOALL | RTL_RX_TOUS);

    /* STAGE: Shutdown the chip. */

    rtl_send_command (NULL);
}

static void rtl_start (void)
{
    /* STAGE: Reenable RX and TX abilities. */

    rtl_send_command (RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

    /* STAGE: Enable broadcast and physical match packets. */

    RTL_IO_INT(RTL_RXCONFIG) |= RTL_RX_TOALL | RTL_RX_TOUS;
}

bool rtl_init(void)
{
    /*
        STAGE: Soft reset the adapter.
        
        This keeps us safe from any previous use.
    */

    rtl_soft_reset ();

/*** AUTO-NEGOTIATE MEDIA TYPE ***/

    /*
        STAGE: Reset and clear CONFIG 1 so we can add our own configuration.
    */

    RTL_IO_BYTE(RTL_CONFIG1) = 0;

    /* STAGE: Enable auto-negotiation of network media. */

    RTL_IO_SHORT(RTL_MII_BMCR) = RTL_BMCR_RESET | RTL_BMCR_ANE | RTL_BMCR_RAN;

    rtl_soft_reset ();

/*** CONFIGURE CARD FOR OPERATION ***/

    /* STAGE: Enable writing configuration. */

    RTL_IO_BYTE(RTL_CFG9346) = RTL_9346_EEM1 | RTL_9346_EEM0;

    /*
        STAGE: Enable receive and transmit functions.
        
        Required to set DMA information.

        CREDIT: According to Linux driver.
    */

    rtl_send_command (RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

    RTL_IO_INT(RTL_RXCONFIG) = RTL_RXCONFIG_ON;
    
    /*
        STAGE: Append CRC
              1024b Tx DMA bursts
              Retry Tx 16 times MAX
    */

    RTL_IO_INT(RTL_TXCONFIG) = RTL_TXCONFIG_ON;

    /* STAGE: Turn off lan-awake. */

    RTL_IO_BYTE(RTL_CONFIG1) &= ~0x30;

    /* STAGE: Set driver-loaded bit. */

    RTL_IO_BYTE(RTL_CONFIG1) |= 0x20;

    /* STAGE: Set Rx FIFO overflow to auto-clear. */

    RTL_IO_BYTE(RTL_CONFIG4) |= RTL_C4_RX_AUTOCLR;

    /* STAGE: Finish configuration - switch to normal operation mode. */

    RTL_IO_BYTE(RTL_CFG9346) = 0x0;

/*** CONFIGURE RING BUFFERS ***/

    /*
        STAGE: Setup Rx and Tx buffers.
        
        NOTE: These values are affected by the PCI initialization.
    */

    RTL_IO_INT(RTL_RXBUF)   = 0x01840000;
    RTL_IO_INT(RTL_TXADDR0) = 0x01846000;
    RTL_IO_INT(RTL_TXADDR1) = 0x01846800;
    RTL_IO_INT(RTL_TXADDR2) = 0x01847000;
    RTL_IO_INT(RTL_TXADDR3) = 0x01847800;

    /* STAGE: Reset Rx FIFO overflow counter. */

    RTL_IO_INT(RTL_RXMISSED) = 0;

/*** CONFIGURE PACKET RECEPTION ***/

    rtl_start ();

/*** CONFIGURE INTERRUPTS ***/
    {
        asic_lookup_table_entry entry;
        uint32                  intr;

        /* STAGE: Configure hook for interrupt table */

        entry.irq       = EXP_CODE_INT13;       /* No one uses IRQ 13 - at least, VOOT doesn't. */
        entry.mask1     = ASIC_MASK1_PCI;       /* Link on the ASIC/G2 PCI interrupt. */
        entry.handler   = rtl_handler;

        /* STAGE: Add hook to interrupt table */

        rtl_info.hdl_tbl_index = asic_add_handler (&entry);

        if (!rtl_info.hdl_tbl_index)    /* We couldn't attach an exception handler - weird, huh? */
        {
            rtl_stop ();
            return FALSE;
        }

        /* STAGE: Clear the interrupt status. */

        intr = RTL_IO_SHORT(RTL_INTRSTATUS);
        RTL_IO_SHORT(RTL_INTRSTATUS) = intr;

        /* STAGE: Tell the RTL which interrupts to fire off. */

        RTL_IO_SHORT(RTL_INTRMASK) = RTL_INT_RX_OK | RTL_INT_RXFIFO_OVERFLOW | RTL_INT_RXBUF_OVERFLOW | RTL_INT_LINKCHG;
    }

    /* STAGE: Disable all multi-interrupts - don't tell us about weird packets. */

    RTL_IO_SHORT(RTL_MULTIINTR) = 0;

    /*
        STAGE: Enable receive and transmit functions.
        
        Required to set DMA information.

        CREDIT: According to Linux driver.
    */

    rtl_send_command(RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

/*** INITIALIZE INTERNAL DATA STRUCTURE ***/

    /* STAGE: Cache the MAC address. */

    rtl_get_mac ();

    /* STAGE: Zero the Rx and Tx counters. */

    rtl_info.cur_rx = rtl_info.cur_tx = 0;

    /* STAGE: Initialize the rate limiter. */

    rtl_info.last_action = time ();

    return TRUE;
}

static uint8* rtl_copy_packet (const uint8 *src, uint32 size)
{
    uint8  *dest;
    uint8  *dma_buffer_end;

    dma_buffer_end = (uint8 *) RTL_DMA_BYTE + RX_BUFFER_LEN;

    /* STAGE: Make sure we have a buffer large enough to hold the packet. */

    dest = malloc (size);

    if (!dest)
        return NULL;

    /*
        STAGE: Copy straight from the DMA if possible, otherwise wrap around
        the end of the ring.
    */

    if ((uint32) (src + size) < (uint32) dma_buffer_end)
    {
        memcpy (dest, src, size);
    }
    else
    {
        /* STAGE: First copy from the src to the end of the DMA buffer. */

        memcpy (dest, src, (uint32) (dma_buffer_end - src));

        /*
            STAGE: Then copy from the beginning of the DMA buffer to the
            remaining end of the packet.
        */
        memcpy (dest + (dma_buffer_end - src),
                (uint8 *) RTL_DMA_BYTE,
                (uint32) (size - (dma_buffer_end - src))
               );
    }

    return dest;
}

/*
    NOTE: This entire implementation is stinky to me because you can't cycle
    through the descriptors to whichever happens to be open.

    I want to be able to parallelize like that.

    Why can't I, huh?
*/

bool rtl_tx (const uint8* frame, uint32 length)
{
    /* STAGE: Perform a rate limit check. */

    if ((time () - rtl_info.last_action) < 5)
    {
        video_waitvbl ();

        rtl_info.last_action = time ();
    }

    /* STAGE: Limit us to a certain size. */

    length &= RTL_TX_SIZE_MASK;

    /* STAGE: Wait/force the descriptor to open up. */

    while (!(RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof (uint32))) & RTL_TX_HOST_OWNS))
    {
        if (RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof (uint32))) & RTL_TX_ABORTED)
            RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof (uint32))) |= 1;
    }

    /* STAGE: Copy the packet into the descriptor. */

    memcpy (rtl_tx_descs[rtl_info.cur_tx], frame, length);

    /* STAGE: Inform the chip the packet has landed and the 'proper' size of it. */

    length = (length < 60) ? 60 : length;   /* NOTE: The 8139c doesn't pad the frames. */

    RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof (uint32))) = length;

    /* STAGE: And use the next descriptor on the next TX. */

    rtl_info.cur_tx = (rtl_info.cur_tx + 1) % 4;

    return TRUE;
}

static void rtl_rx_all (void)
{
    /*
        STAGE: Keep receiving packets until they're all gone...
        
        or we run into a packet still being read by the hardware...
        
        or of the network layer tells us to stop receiving.
    */

    while (!(RTL_IO_BYTE(RTL_CHIPCMD) & RTL_CMD_RX_BUF_EMPTY))
    {
        uint8  *frame_in;
        uint32  rx_status;
        uint32  rx_size;
        uint32  packet_size;
        uint8  *packet_data;

        frame_in = NULL;

        /* STAGE: Get pointers to frame size and status. */

        rx_status = RTL_DMA_SHORT[rtl_info.cur_rx/2];
        rx_size = RTL_DMA_SHORT[rtl_info.cur_rx/2 + 1];

        /*
            STAGE: Check if the RTL is still copying packets - if so, try
            again.
            
            NOTE: This should never happen because we're driven by
            interrupts.
         */

        if (rx_size == RTL_DMA_FRAME_COPYING)
            break;

        packet_size = rx_size - 4;

        /* STAGE: Handle that GOOD packet, baby! */

        if (rx_status & 1)  /* NOTE: .. if it's done? */
        {
            /* NOTE: + 4 to skip the header. */

            packet_data = (uint8 *) ((((uint32) RTL_DMA_BYTE) + rtl_info.cur_rx) + 4);

            frame_in = rtl_copy_packet (packet_data, packet_size);
        }

        /*
            STAGE: Align the next packet on a 32-bit boundray...

            NOTE: + 4 (Rx header)
                  + 3 (so no collision)
        */

        rtl_info.cur_rx = (((rtl_info.cur_rx + rx_size + 4) + 3) & ~3) % RX_BUFFER_LEN;

        /* STAGE: Reset both buffers to within our data scope. */

        RTL_IO_SHORT(RTL_RXBUFTAIL) = (rtl_info.cur_rx - RX_BUFFER_THRESHOLD) % RX_BUFFER_LEN;

        /* STAGE: Stop processing if network layer instructs us too... */

        if (frame_in && net_handle_frame (frame_in, packet_size))
            break;

        /* STAGE: free() the packet, no matter what. */

        free (frame_in);
    }
}

void* rtl_handler (void *passer, register_stack *stack, void *current_vector)
{
    uint32 intr;

    /*
        STAGE: First, identify and clear out the interrupts.
        
        We don't want it to *keep* yelling at us.
    */

    intr = RTL_IO_SHORT(RTL_INTRSTATUS);
    RTL_IO_SHORT(RTL_INTRSTATUS) = intr;

    /*
        STAGE: Handle overflows relatively harshly.
        
        CREDIT: I'm taking this solution from the BSD driver.
    */

    if (intr & RTL_INT_RXBUF_OVERFLOW)
    {
        rtl_stop ();
        
        rtl_info.cur_rx = RTL_IO_SHORT(RTL_RXBUFHEAD) % RX_BUFFER_LEN;
        RTL_IO_SHORT(RTL_RXBUFTAIL) = rtl_info.cur_rx - RX_BUFFER_THRESHOLD;

        rtl_start ();
    }
    /* STAGE: If we're performing a link change, finish up. */
    else if (intr & RTL_INT_LINKCHG)
    {
        static bool link_stable = FALSE;

        if (link_stable)
        {
            RTL_IO_SHORT(RTL_MII_BMCR) = RTL_BMCR_RESET | RTL_BMCR_ANE | RTL_BMCR_RAN;
            link_stable = FALSE;
        }
        else
        {
            link_stable = TRUE;
        }
    }
    /* STAGE: Check if we received packets. */
    else if (intr & RTL_INT_RX_OK)
    {
        rtl_rx_all ();
    }

    return my_exception_finish;
}
