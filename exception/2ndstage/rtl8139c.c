/*  rtl8139c.c

    Driver code for the RealTek 8139c.

    This card is a very confusing card but I hope to write my driver
    "cleaner" than the ones before it and thus make it a much better
    reference doc for people out there.

    RealTek (http://www.realtek.com.tw/) has been great and released Engrish
    documentation about their card.

    Andrew has also contributed to this card with great reference code and
    even better personal support in #dcdev@EFNet. Give the guy a damn hand.
*/

#define RTL_DCLOAD_VERSION

#include "vars.h"
#include "rtl8139c.h"
#include "system.h"
#include "asic.h"
#include "serial.h"
#include "exception-lowlevel.h"
#include "net.h"

/* *
   * GAPS PCI Controller and BBA PCI Configuration
   *
   * pc_* is all code that is completely magic. My main gripe with it is not
   * it being magic but rather it assumes that because there is a GAPS PCI
   * controller there is a BBA. In all honesty there will probably never be
   * another PCI device on the Dreamcast, but I just don't feel comfortable
   * not checking the PCI identification codes.
   *
   * However, you can fit what I know about writing code for PCI in a
   * thimble and still have room for the air, so I think I'll shut up until
   * someone much more talented than I comes along and fixes up this code.
   *
*/

bool pci_detect(void)
{
    const char gapspci_ident_str[] = "GAPSPCI_BRIDGE_2";

    /* Must be re-casted to char * so we can const-ify it cleanly. Grr. */
    return !memcmp((char *) PCI_IDENT_STR, gapspci_ident_str, sizeof(gapspci_ident_str));
}

/* All of the following code is *very* magic - even more magical than the above code. */
bool pci_bb_init(void)
{
    uint32  count;

    /* Initialize the "GAPS" PCI glue controller */
    G2_INT(0x1418) = 0x5a14a50;

    count = 10000;
    while (!(G2_INT(0x1418) & 1) && count > 0)
        count--;

    if (!(G2_INT(0x1418) & 1))
        return FALSE;

    G2_INT(0x1420) = 0x01000000;
    G2_INT(0x1424) = 0x01000000;
    G2_INT(0x1428) = 0x01840000;                /* DMA Base */
    G2_INT(0x142c) = 0x01840000 + 32 * 1024;    /* DMA End - 32k buffer includes Rx Ring and Tx descriptors */
    G2_INT(0x1414) = 0x00000001;
    G2_INT(0x1434) = 0x00000001;

    /* Initialize the PCI Bridge - this is the part which *should* be more intelligent, but isn't. */
    G2_SHORT(0x1606) = 0xf900;
    G2_INT(0x1630)   = 0x00000000;
    G2_BYTE(0x163c)  = 0x00;
    G2_BYTE(0x160d)  = 0xf0;

    (void)G2_SHORT(0x04);

    G2_SHORT(0x1604) = 0x0006;
    G2_INT(0x1614)   = 0x01000000;

    (void)G2_BYTE(0x1650);

    return TRUE;
}

/* *
   * Real-Tek 8139C Ethernet Network Interface Card
   *
   * This is where the real fun begins. Documentation will come.
   *
*/

rtl_t   rtl_info;

/* RTL8139c Tx descriptor indexes */
static uint8* rtl_tx_descs[] = { (uint8 *) 0xa1846000,
                                 (uint8 *) 0xa1846800,
                                 (uint8 *) 0xa1847000,
                                 (uint8 *) 0xa1847800
                               };

static void rtl_mac(void)
{
    unsigned tmp;

    /* Copy over the first segment */
    tmp = RTL_IO_INT(RTL_IDR0);

    rtl_info.mac[0] = tmp & 0xff;
    rtl_info.mac[1] = (tmp >> 8) & 0xff;
    rtl_info.mac[2] = (tmp >> 16) & 0xff;
    rtl_info.mac[3] = (tmp >> 24) & 0xff;

    tmp = RTL_IO_INT(RTL_IDR1);
    rtl_info.mac[4] = tmp & 0xff;
    rtl_info.mac[5] = (tmp >> 8) & 0xff;
}

static void rtl_soft_reset(void)
{
    RTL_IO_BYTE(RTL_CHIPCMD) = RTL_CMD_RESET;

    while (RTL_IO_BYTE(RTL_CHIPCMD) & RTL_CMD_RESET);
}

static void rtl_send_command(uint8 command)
{
    RTL_IO_BYTE(RTL_CHIPCMD) = command;

    /* Ignore all but known command registers */
    while ((RTL_IO_BYTE(RTL_CHIPCMD) & ~0xe3) != command);
}

void rtl_stop(void)
{
    RTL_IO_INT(RTL_RXCONFIG) &= ~(RTL_RX_TOALL | RTL_RX_TOUS);
    rtl_send_command(0);
}

void rtl_start(void)
{
    /* STAGE: Enable broadcast and physical match packets */
    rtl_send_command(RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);
    RTL_IO_INT(RTL_RXCONFIG) |= RTL_RX_TOALL | RTL_RX_TOUS;
}

bool rtl_init(void)
{
    /* STAGE: Soft reset the adapter, this keeps us safe from any previous
               use. */
    rtl_soft_reset();

/*** AUTO-NEGOTIATE MEDIA TYPE ***/

    /* STAGE: Reset and clear CONFIG 1 so we can add our own
               configuration. */
    RTL_IO_BYTE(RTL_CONFIG1) = 0;

    /* STAGE: Enable auto-negotiation of network media */
    RTL_IO_SHORT(RTL_MII_BMCR) = RTL_BMCR_ANE | RTL_BMCR_RAN;

    rtl_soft_reset();

/*** CONFIGURE CARD FOR OPERATION ***/

    /* STAGE: Enable writing configuration */
    RTL_IO_BYTE(RTL_CFG9346) = RTL_9346_EEM1 | RTL_9346_EEM0;

    /* STAGE: Enable receive and transmit functions - required to set DMA
               information (according to Linux driver) */
    rtl_send_command(RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

    RTL_IO_INT(RTL_RXCONFIG) = RTL_RXCONFIG_ON;
    
    /* STAGE: Append CRC
              1024b Tx DMA bursts
              Retry Tx 16 times MAX
    */
    RTL_IO_INT(RTL_TXCONFIG) = 0x00000600;

    /* STAGE: Turn off lan-awake. */
    RTL_IO_BYTE(RTL_CONFIG1) &= ~0x30;

    /* STAGE: Set driver-loaded bit. */
    RTL_IO_BYTE(RTL_CONFIG1) |= 0x20;

    /* STAGE: Set Rx FIFO overflow to auto-clear */
    RTL_IO_BYTE(RTL_CONFIG4) |= RTL_C4_RX_AUTOCLR;

    /* STAGE: Finish configuration - switch to normal operation mode */
    RTL_IO_BYTE(RTL_CFG9346) = 0x0;

/*** CONFIGURE RING BUFFERS ***/

    /* STAGE: Setup Rx and Tx buffers */
    RTL_IO_INT(RTL_RXBUF) = 0x01840000;
    RTL_IO_INT(RTL_TXADDR0) = 0x01846000;
    RTL_IO_INT(RTL_TXADDR1) = 0x01846800;
    RTL_IO_INT(RTL_TXADDR2) = 0x01847000;
    RTL_IO_INT(RTL_TXADDR3) = 0x01847800;

    /* STAGE: Reset Rx FIFO overflow counter */
    RTL_IO_INT(RTL_RXMISSED) = 0;

/*** CONFIGURE PACKET RECEPTION ***/

    rtl_start();

/*** CONFIGURE INTERRUPTS ***/
#define ENABLE_RTL_INTS
#ifdef ENABLE_RTL_INTS
    {
        asic_lookup_table_entry entry;

        /* STAGE: Configure hook for interrupt table */
        entry.irq = EXP_CODE_INT13;     /* No one uses IRQ 13 - at least, VOOT doesn't. */
        entry.mask1 = ASIC_MASK1_PCI;   /* Link on the ASIC/G2 PCI interrupt. */
        entry.handler = rtl_handler;

        /* STAGE: Add hook to interrupt table */
        rtl_info.hdl_tbl_index = add_asic_handler(entry);
        if (!rtl_info.hdl_tbl_index)    /* We couldn't attach an exception handler - weird, huh? */
        {
            rtl_stop();
            return FALSE;
        }

        /* STAGE: Tell the RTL which interrupts to fire off.
            It seems the RTL requires RTL_IMR_LINKCHK to work properly. Oh
            well.
        */
        RTL_IO_SHORT(RTL_INTRMASK) = RTL_IMR_LINKCHG | RTL_IMR_RX_OK | RTL_IMR_FIFO_OVER | RTL_IMR_DMA_OVER;
        //RTL_IO_SHORT(RTL_INTRMASK) = RTL_IMR_RX_OK | RTL_IMR_FIFO_OVER | RTL_IMR_DMA_OVER;
    }
#endif

    /* STAGE: Disable all multi-interrupts - don't tell us about weird packets. */
    RTL_IO_SHORT(RTL_MULTIINTR) = 0;

    /* STAGE: Enable receive and transmit functions - required to set DMA
               information (according to Linux driver) */
    rtl_send_command(RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

/*** INITIALIZE INTERNAL DATA STRUCTURE ***/

    /* STAGE: Read MAC address */
    rtl_mac();

    /* STAGE: Zero the Rx and Tx counters */
    rtl_info.cur_rx = rtl_info.cur_tx = 0;

    return TRUE;
}

/* Copy straight from the DMA if possible, otherwise wrap around the end */
static void rtl_copy_packet(uint8 *dest, uint8 *src, uint32 size)
{
    uint8 *dma_buffer_end;

    /* Precompute a bunch of values - the compiler should be smart about
        this, but we've turned off optimization for the moment. */
    dma_buffer_end = (uint8 *) RTL_DMA_BYTE + RX_BUFFER_LEN;

    if (size > sizeof(rtl_info.frame_in_buffer))   /* Final paranoia. */ 
        return;

    if ((uint32) (src + size) < (uint32) dma_buffer_end) /* maybe this wants to be <= */
        memcpy(dest, src, size);
    else
    {
        /* First copy from the src to the end of the DMA buffer. */
        memcpy(dest, src, (uint32) (dma_buffer_end - src));

        /* Then copy from the beginning of the DMA buffer to the remaining
            end of the packet. */
        memcpy(dest + (dma_buffer_end - src),
               (uint8 *) RTL_DMA_BYTE,
               (uint32) (size - (dma_buffer_end - src)));
    }
}

#ifdef TX_SCOTT_IMPL

static int16 rtl_find_free_descriptor(void)
{
    int16 index;

    for (index = 0; index < 4; index++)
    {
        /* STAGE: We can conveniently handle re-transmission of packets
            here. Of course, if there is something fundamentally wrong we'll
            run out of slots really fast. */
        if (RTL_IO_INT((index * sizeof(int)) + RTL_TXSTATUS0) & RTL_TX_ABORTED)
        {
#ifdef DEBUG_RTL8139C
            ubc_serial_write_str("[UBC] Retry on Tx 0x");
            ubc_serial_write_hex(index);
            ubc_serial_write_str("\r\n");
#endif

            RTL_IO_INT((index * sizeof(int)) + RTL_TXSTATUS0) &= ~RTL_TX_HOST_OWNS;
        }
        else if (RTL_IO_INT((index * sizeof(int)) + RTL_TXSTATUS0) & RTL_TX_HOST_OWNS)
            return index;
    }

    return -1;
}

bool rtl_tx(const uint8* frame, uint32 length)
{
    int16  descriptor;

    /* STAGE: Basic paranoia checking. */
    if (length >= NET_MAX_PACKET)
        return FALSE;

    /* STAGE: Find an open Tx descriptor for use. */
    descriptor = rtl_find_free_descriptor();
    if (descriptor < 0)
        return FALSE;

    /* STAGE: Copy frame into the TX descriptor. */
    memcpy(rtl_tx_descs[descriptor], frame, length);
    length = (length >= 60) ? length : 60;   /* Apparently you need to pad Ethernet frames. */

    /* STAGE: Copy the length into the status register - it also resets the
        register, but that's cool. */
    RTL_IO_INT((descriptor * sizeof(int)) + RTL_TXSTATUS0) = length;

    return TRUE;
}

#else

/* !!! This entire implementation is stinky to me because you can't cycle
        through the descriptors to whichever happens to be open. I want to
        be able to parallelize like that. Why can't I, huh? */

bool rtl_tx(const uint8* frame, uint32 length)
{
    /* STAGE: Limit us to the size of the  */
    length &= RTL_TX_SIZE_MASK;

    while (!(RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof(uint32))) & RTL_TX_HOST_OWNS))
    {
        if (RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof(uint32))) & RTL_TX_ABORTED)
        {
            ubc_serial_write_str("[UBC] Tx magic!\r\n");

            RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof(uint32))) |= 1;
        }
    }

    memcpy(rtl_tx_descs[rtl_info.cur_tx], frame, length);

    length = (length < 60) ? 60 : length;     /* the 8139c doesn't pad the frames. */

    RTL_IO_INT(RTL_TXSTATUS0 + (rtl_info.cur_tx * sizeof(uint32))) = length;

    rtl_info.cur_tx = (rtl_info.cur_tx + 1) % 4;

    return TRUE;
}

#endif


void rtl_rx_all(void)
{
    /* STAGE: Keep receiving packets until they're all gone - I hope we
        don't get flooded.
    */
    while(!(RTL_IO_BYTE(RTL_CHIPCMD) & RTL_CMD_RX_BUF_EMPTY))
    {
        uint32 rx_status, rx_size;
        uint8 *packet_data;
        uint32 packet_size;

        /* STAGE: Get pointers to frame size and status */
        rx_status = RTL_DMA_SHORT[rtl_info.cur_rx/2];
        rx_size = RTL_DMA_SHORT[rtl_info.cur_rx/2 + 1];

        /* STAGE: Check if the RTL is still copying packets - if so, try
            again. This should never happen because we're driven by
            interrupts.
         */
        if (rx_size == RTL_DMA_FRAME_COPYING)
            break;

        packet_size = rx_size - 4;

        /* Handle that GOOD packet, baby! */
        if (rx_status & 1)  /* if it's done? */
        {
            /* + 4 to skip the header. */
            packet_data = (uint8 *) ((((uint32) RTL_DMA_BYTE) + rtl_info.cur_rx) + 4);

            rtl_copy_packet(rtl_info.frame_in_buffer, packet_data, packet_size);
        }

        /* Align the next packet on a 32-bit boundray.
            + 4 (Rx header)
            + 3 (so no collision) */
        rtl_info.cur_rx = (((rtl_info.cur_rx + rx_size + 4) + 3) & ~3) % RX_BUFFER_LEN;

        /* Reset both buffers to within our scope. */
        RTL_IO_SHORT(RTL_RXBUFTAIL) = (rtl_info.cur_rx - RX_BUFFER_THRESHOLD) % RX_BUFFER_LEN;

        if (rx_status & 1)
            net_handle_frame(rtl_info.frame_in_buffer, packet_size);
    }
}

/* For the moment, this will be a debugging stub. Later, we'll implement
    whatever interface lwIP requires */
void* rtl_handler(void *passed, register_stack *stack, void *current_vector)
{
    uint32 intr;

    /* STAGE: First, identify and clear out the interrupts. We don't want it
        to *keep* yelling at us. */
    intr = RTL_IO_SHORT(RTL_INTRSTATUS);
    RTL_IO_SHORT(RTL_INTRSTATUS) = intr;

    /* STAGE: Check if we received packets. */
    if (intr & RTL_INT_RX_OK)
        rtl_rx_all();

    /* STAGE: These are handled by the chip because I told it too. I'll warn
        myself anyway, though. */
    if (intr & RTL_INT_RXFIFO_OVERFLOW)
        ubc_serial_write_str("[UBC] RTL8193c FIFO overflow!\r\n");

    /* STAGE: Handle overflows relatively harshly. I'm taking this solution
        from the BSD guys, though. I'm willing to bet they have a little
        experience in this. */
    if (intr & RTL_INT_RXBUF_OVERFLOW)
    {
        ubc_serial_write_str("[UBC] RTL8139c DMA overflow ...");

#ifdef RTL_OVERFLOW_BSD
        rtl_init();     /* I'm taking a page out of the BSD book... */
#else   /* handle it Linux style. */
        rtl_stop();
        
        rtl_info.cur_rx = RTL_IO_SHORT(RTL_RXBUFHEAD) % RX_BUFFER_LEN;
        RTL_IO_SHORT(RTL_RXBUFTAIL) = rtl_info.cur_rx - RX_BUFFER_THRESHOLD;

        rtl_start();
#endif
    }

    /* STAGE: Return from the handler */
    return my_exception_finish;
}
