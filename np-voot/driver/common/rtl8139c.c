/*  rtl8139c.c

    $Id: rtl8139c.c,v 1.14 2002/11/08 19:47:41 quad Exp $

DESCRIPTION

    Driver code for the RealTek 8139c network chipset.

    This module should only be accessed the ethernet layer. (ether)

    CREDIT: RealTek (http://www.realtek.com.tw/) has been great and released
    Engrish documentation about their card.

    CREDIT: Andrew has also contributed to this card with great reference
    code and even better personal support in #dcdev@EFNet. Give the guy a
    damn hand.

TODO

    Remove rtl_rx_all function and replace with a completely interrupt
    driven driver.

*/

#include "vars.h"
#include "util.h"
#include "malloc.h"
#include "asic.h"

#include "rtl8139c.h"

/*
    NOTE: An internalized rtl_t reference is required for interrupt
    handling. However, it's also explicitly accessed and released.
*/

rtl_t   *rtl_irq_info;

/* NOTE: TX descriptor addresses. */

static uint8*   rtl_tx_descs[]  = {
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

static bool pci_detect (void)
{
    const char  gapspci_ident_str[] = "GAPSPCI_BRIDGE_2";

    return !memcmp (PCI_IDENT_STR, gapspci_ident_str, sizeof (gapspci_ident_str));
}

/* NOTE: All of the following code is *very* magic - even more magical than the above code. */

static bool pci_bb_init (void)
{
    uint32  count;

    /* STAGE: Initialize the "GAPS" PCI glue controller. */

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
        STAGE: Initialize the PCI Bridge.
    
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
    NOTE: Miscellaneous control functions for the adapter. No real logic
    occurs in these functions.
*/

static void rtl_soft_reset (void)
{
    RTL_IO_BYTE(RTL_CHIPCMD) = RTL_CMD_RESET;

    while (RTL_IO_BYTE(RTL_CHIPCMD) & RTL_CMD_RESET);
}

static void rtl_send_command (uint8 command)
{
    RTL_IO_BYTE(RTL_CHIPCMD) = command;

    /* NOTE: Ignore all but known command bits. */

    while ((RTL_IO_BYTE(RTL_CHIPCMD) & ~0xe3) != command);
}

static void rtl_stop (void)
{
    /* STAGE: Disable frame reception. */

    RTL_IO_INT(RTL_RXCONFIG) &= ~(RTL_RX_TOALL | RTL_RX_TOUS);

    /* STAGE: Shutdown the chip. */

    rtl_send_command (NULL);
}

static void rtl_start (void)
{
    /* STAGE: Enable RX and TX abilities. */

    rtl_send_command (RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

    /* STAGE: Enable broadcast and physical match frames. */

    RTL_IO_INT(RTL_RXCONFIG) |= RTL_RX_TOALL | RTL_RX_TOUS;
}

static void rtl_negotiate_media (void)
{
    /* STAGE: Enable auto-negotiation of network media. */

    RTL_IO_SHORT(RTL_MII_BMCR) = RTL_BMCR_RESET | RTL_BMCR_ANE | RTL_BMCR_RAN;
}

/*
    NOTE: Packet reception logic!
*/

static uint8* rtl_copy_frame (const uint8 *src, uint32 size)
{
    uint8  *dest;
    uint8  *dma_buffer_end;

    dma_buffer_end = (uint8 *) RTL_DMA_BYTE + RX_BUFFER_LEN;

    /* STAGE: Make sure we have a buffer large enough to hold the frame. */

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
            remaining end of the frame.
        */
        memcpy (dest + (dma_buffer_end - src),
                (uint8 *) RTL_DMA_BYTE,
                (uint32) (size - (dma_buffer_end - src))
               );
    }

    return dest;
}

static void rtl_rx_all (rtl_t *rtl_info)
{
    /*
        STAGE: Keep receiving frames until they're all gone...
        
        or we run into a frame still being read by the hardware...
        
        or of the network layer tells us to stop receiving.
    */

    while (!(RTL_IO_BYTE(RTL_CHIPCMD) & RTL_CMD_RX_BUF_EMPTY))
    {
        uint8  *frame_in;
        uint32  rx_status;
        uint32  rx_size;
        uint32  frame_size;
        uint8  *frame_data;

        frame_in    = NULL;

        /* STAGE: Get pointers to frame size and status. */

        rx_status   = RTL_DMA_SHORT[rtl_info->cur_rx/2];
        rx_size     = RTL_DMA_SHORT[rtl_info->cur_rx/2 + 1];

        /*
            STAGE: Check if the RTL is still copying frames - if so, try
            again.
            
            NOTE: This should never happen because we're driven by
            interrupts.
         */

        if (rx_size == RTL_DMA_FRAME_COPYING)
            break;

        frame_size = rx_size - 4;

        /* STAGE: If the frame is done being transferred via DMA. */

        if (rx_status & 1)
        {
            /* NOTE: + 4 to skip the header. */

            frame_data = (uint8 *) ((((uint32) RTL_DMA_BYTE) + rtl_info->cur_rx) + 4);

            frame_in = rtl_copy_frame (frame_data, frame_size);
        }

        /*
            STAGE: Align the next frame on a 32-bit boundray...

            NOTE: + 4 (Rx header)
                  + 3 (so no collision)
        */

        rtl_info->cur_rx = (((rtl_info->cur_rx + rx_size + 4) + 3) & ~3) % RX_BUFFER_LEN;

        /* STAGE: Reset both buffers to within our data scope. */

        RTL_IO_SHORT(RTL_RXBUFTAIL) = (rtl_info->cur_rx - RX_BUFFER_THRESHOLD) % RX_BUFFER_LEN;

        /* STAGE: Don't release the frame if network layer instructs us too... */

        if (frame_in && ether_handle_frame (frame_in, frame_size))
            continue;

        /* STAGE: free () the frame, otherwise. */

        free (frame_in);
    }
}

/*
    NOTE: Chip interrupt handler.
*/

static void* rtl_irq_handler (void *passer, register_stack *stack, void *current_vector)
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
        
        rtl_irq_info->cur_rx = RTL_IO_SHORT(RTL_RXBUFHEAD) % RX_BUFFER_LEN;
        RTL_IO_SHORT(RTL_RXBUFTAIL) = rtl_irq_info->cur_rx - RX_BUFFER_THRESHOLD;

        rtl_start ();
    }

    /*
        STAGE: If we're performing a link change, finish up.

        NOTE: The direction of link_stable with respect to the chip is a bit
        magic.
    */

    else if (intr & RTL_INT_LINKCHG)
    {
        if (rtl_irq_info->link_stable)
        {
            rtl_negotiate_media ();

            rtl_irq_info->link_stable = FALSE;
        }
        else
        {
            rtl_irq_info->link_stable = TRUE;
        }
    }

    /* STAGE: Check if we received frames. */

    else if (intr & RTL_INT_RX_OK)
    {
        /* TODO: Notify the IP stack we have packets. Handling occurs semi-out of band. */
    }
    else if (intr & RTL_INT_TX_OK)
    {
        /* TODO: Notify the IP stack we can transmit. Handling occurs semi-out of band. */
    }


    if (rtl_irq_info->old_rtl_handler)
        return rtl_irq_info->old_rtl_handler (passer, stack, my_exception_finish);
    else
        return my_exception_finish;
}

static void rtl_chip_configure (void)
{
    /* STAGE: Allow us to write the chipside configuration data. */

    RTL_IO_BYTE(RTL_CFG9346) = RTL_9346_EEM1 | RTL_9346_EEM0;

    /* STAGE: Configure RX and TX chipside functions. */

    RTL_IO_INT(RTL_RXCONFIG) = RTL_RXCONFIG_ON;
    RTL_IO_INT(RTL_TXCONFIG) = RTL_TXCONFIG_ON;

    /* STAGE: Configure miscellaneous chipside options. */

    RTL_IO_BYTE(RTL_CONFIG1) &= ~0x30;              /* NOTE: Disable LAN-awake. */
    RTL_IO_BYTE(RTL_CONFIG1) |= 0x20;               /* NOTE: Set driver-loaded bit. */
    RTL_IO_BYTE(RTL_CONFIG4) |= RTL_C4_RX_AUTOCLR;  /* NOTE: Set Rx FIFO overflow to auto-flush. */

    /* STAGE: Finished configuration - switch to normal operation mode. */

    RTL_IO_BYTE(RTL_CFG9346) = 0x0;

    /* STAGE: Reset Rx FIFO overflow counter. */

    RTL_IO_INT(RTL_RXMISSED) = 0;
}

static bool rtl_int_configure (rtl_t *rtl_info)
{
    asic_lookup_table_entry entry;
    uint32                  intr;

    /* STAGE: Configure hook for interrupt table. */

    entry.irq       = EXP_CODE_INT13;       /* NOTE: No one uses IRQ 13 - at least, VOOT doesn't. */
    entry.mask1     = ASIC_MASK1_PCI;       /* NOTE: Latch on the G2 PCI interrupt. */
    entry.handler   = rtl_irq_handler;

    /* STAGE: Add hook to interrupt table. */

    if (!(asic_add_handler (&entry, &(rtl_info->old_rtl_handler))))
    {
        /* NOTE: Shutdown handling is in rtl_init_real (). */

        return FALSE;
    }

    /* STAGE: Clear the interrupt status. */

    intr = RTL_IO_SHORT(RTL_INTRSTATUS);
    RTL_IO_SHORT(RTL_INTRSTATUS) = intr;

    /* STAGE: Tell the RTL which interrupts to fire off. */

    RTL_IO_SHORT(RTL_INTRMASK) = RTL_INT_TX_OK | RTL_INT_RX_OK | RTL_INT_RXFIFO_OVERFLOW | RTL_INT_RXBUF_OVERFLOW | RTL_INT_LINKCHG;

    /* STAGE: Disable all multi-interrupts - don't tell us about weird frames. */

    RTL_IO_SHORT(RTL_MULTIINTR) = 0;

    /* STAGE: Configured successfully! */

    return TRUE;
}

static void rtl_cache_mac (rtl_t *rtl_info)
{
    unsigned tmp;

    /* STAGE: Copy over the first segment. */

    tmp = RTL_IO_INT(RTL_IDR0);

    rtl_info->mac[0] = tmp & 0xff;
    rtl_info->mac[1] = (tmp >> 8) & 0xff;
    rtl_info->mac[2] = (tmp >> 16) & 0xff;
    rtl_info->mac[3] = (tmp >> 24) & 0xff;

    /* STAGE: Copy over the second segment. */

    tmp = RTL_IO_INT(RTL_IDR1);

    rtl_info->mac[4] = tmp & 0xff;
    rtl_info->mac[5] = (tmp >> 8) & 0xff;
}

/*
    NOTE: This is the actual initialization function.

    The sequence is broken up between various functions for readability.
*/

static bool rtl_init_real (rtl_t *rtl_info)
{
    /*
        STAGE: [STEP 1] Soft reset the adapter and stop the adapter.
        
        This keeps us safe from any previous use.
    */

    rtl_soft_reset ();
    rtl_stop ();

    /*
        STAGE: [STEP 2] Clear out the adapter's current configuration.
    */

    RTL_IO_BYTE(RTL_CONFIG1) = 0;

    /*
        STAGE: [STEP 3] Auto-negotiate network media type.
    */

    rtl_negotiate_media ();

    /*
        STAGE: [STEP 4] Configure chip for operation.

        NOTE: RX and TX functionality must be enabled, but we don't want to
        receive frames.
    */

    rtl_send_command (RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

    rtl_chip_configure ();

    /*
        STAGE: [STEP 5] Configure the RX and TX DMA buffers.

        NOTE: These values are affected by the PCI initialization.
    */

    RTL_IO_INT(RTL_RXBUF)   = 0x01840000;
    RTL_IO_INT(RTL_TXADDR0) = 0x01846000;
    RTL_IO_INT(RTL_TXADDR1) = 0x01846800;
    RTL_IO_INT(RTL_TXADDR2) = 0x01847000;
    RTL_IO_INT(RTL_TXADDR3) = 0x01847800;

    /*
        STAGE: [STEP 6] Enable and latch on chip interrupts.
    */

    if (!(rtl_int_configure (rtl_info)))
    {
        rtl_stop ();
        rtl_soft_reset ();

        return FALSE;
    }

    /*
        STAGE: [STEP 7] Enable RX and TX functionality.
    */

    rtl_start ();

    /*
        STAGE: [STEP 8] Finish module initialization.
    */

    rtl_cache_mac (rtl_info);

    rtl_info->cur_rx = rtl_info->cur_tx = 0;

    return TRUE;
}

/* NOTE: Interface to the ETHERNET LAYER. */

bool rtl_init (rtl_t *rtl_info)
{ 
    if (pci_detect ())
    {
        if (pci_bb_init ())
        {
            rtl_info->inited = rtl_init_real (rtl_info);
        }
    }

    return rtl_info->inited;
}

bool rtl_tx_write (rtl_t *rtl_info, const uint8 *data, uint32 data_length)
{
    uint32  maybe_frame_length;

    /* STAGE: Ensure the driver is initialized. */

    if (!rtl_info->inited)
        return FALSE;

    /* STAGE: Limit the total frame to a certain size. */

    maybe_frame_length = rtl_info->cur_tx_index + data_length;

    if (maybe_frame_length > RTL_TX_SIZE_MASK)
        return FALSE;

    /* STAGE: Wait/force the descriptor to open up. */

    while (!(RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info->cur_tx * sizeof (uint32))) & RTL_TX_HOST_OWNS))
    {
        if (RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info->cur_tx * sizeof (uint32))) & RTL_TX_ABORTED)
            RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info->cur_tx * sizeof (uint32))) |= 1;
    }

    /* STAGE: Copy the incomplete frame into the descriptor. */

    memcpy (rtl_tx_descs[rtl_info->cur_tx] + rtl_info->cur_tx_index, data, data_length);

    /* STAGE: Update the index into the descriptor. */

    rtl_info->cur_tx_index = maybe_frame_length;
    
    return TRUE;
}

bool rtl_tx_final (rtl_t *rtl_info)
{
    uint32  frame_length;

    frame_length = rtl_info->cur_tx_index;

    /*
        STAGE: Since they can't begin a transmission without us being
        initialized, I'll only check if we have data in the TX descriptor.
    */

    if (!frame_length)
        return FALSE;

    /*
        STAGE: Inform the chip the frame has landed and the 'proper' size of it.

        NOTE: The 8139c doesn't pad its frames.
    */

    frame_length = (frame_length < 60) ? 60 : frame_length;

    RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info->cur_tx * sizeof (uint32))) = frame_length;

    /* STAGE: Use the next descriptor on our next TX. */

    rtl_info->cur_tx_index   = 0;
    rtl_info->cur_tx         = (rtl_info->cur_tx + 1) % 4;

    return TRUE;
}

bool rtl_tx_abort (rtl_t *rtl_info)
{
    /*
        STAGE: Since they can't begin a transmission without us being
        initialized, I'll only check if we have data in the TX descriptor.
    */

    if (!rtl_info->cur_tx_index)
        return FALSE;

    /*
        STAGE: Reset the TX descriptor index.
    */
    
    rtl_info->cur_tx_index = 0;

    return TRUE;
}
