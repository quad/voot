/*  main.c

    The code first ran by the first stage loader. We quickly abandon this
    shell and hook to various parts of the system.
*/

#include "exception-lowlevel.h"
#include "exception.h"
#include "trap.h"
#include "util.h"
#include "warez_load.h"

#define LOADED_POINT        0x8C300000
#define REAL_LOAD_POINT     0x8C010000

#include "biudp.h"
#ifdef HARDCODE_IP

void hard_biudp_init(void)
{
    biudp_control_t control;
    uint8 a_mac[ETHER_MAC_SIZE] = {0x00, 0xD0, 0xF1, 0x02, 0x97, 0xDC};
    uint8 b_mac[ETHER_MAC_SIZE] = {0x00, 0xD0, 0xF1, 0x02, 0x97, 0xCD};
    uint32 ip = 0x0A0000F0;

    ip = htonl(ip);

#define DO_A_MAC
#ifdef DO_A_MAC
    memcpy(control.dest_mac, a_mac, ETHER_MAC_SIZE);
#else
    memcpy(control.dest_mac, b_mac, ETHER_MAC_SIZE);
#endif

    IP_ADDR_COPY(control.source_ip, ip);
    IP_ADDR_COPY(control.dest_ip, ip);
    control.port = htons(VOOT_UDP_PORT);

    biudp_init(&control);
}
#endif

int dc_main(int do_descramble)
{
    unsigned long bin_size;

    /* STAGE: Initialize the UBC. */
    *UBC_R_BRCR = UBC_BRCR_UBDE;
    dbr_set(exception_handler_lowlevel);
    vbr_set(vbr_buffer);

    /* STAGE: Initialize both UBC channels. */
    init_ubc_a_exception();
    init_ubc_b_serial();

    /* STAGE: Wait enough cycles for the UBC to be working properly. */
    ubc_wait();

#ifdef HARDCODE_IP
    hard_biudp_init();
#endif

    /* STAGE: Handle the 1ST_READ.BIN */
    if (do_descramble)
    {
        disable_cache();
        warez_load(*((unsigned long *) REAL_LOAD_POINT));
    }
    else
    {
        /* STAGE: Relocate the 1st_read.bin */
        bin_size = *((unsigned long *) REAL_LOAD_POINT);
        memmove((unsigned char *) REAL_LOAD_POINT, (unsigned char *) LOADED_POINT, bin_size);

        /* STAGE: Execute the 1ST_READ.BIN */
        disable_cache();
        (*(void (*)()) REAL_LOAD_POINT) ();
    }

    /* Freeze the system. */
    while(1);
}
