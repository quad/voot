/*  net.c

    $Id: net.c,v 1.1 2002/11/12 02:00:55 quad Exp $

DESCRIPTION

    Network interface layer between the common library and lwIP

TODO

    Implement TCP.

    Add support for LAN Adapter.

    Add support for modem via SLIP.

*/

#include <vars.h>

#include "lwip/debug.h"

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"

#include "lwip/stats.h"

#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "bbaif.h"

#include "net.h"

void net_handle_tx (void *owner)
{
    struct netif   *netif;

    netif = (struct netif *) owner;

    /* NOTE: Dummy function, for the moment. */
}

void net_handle_rx (void *owner)
{
    struct netif   *netif;

    netif = (struct netif *) owner;

    /* NOTE: Dummy function, simply calls bbaif_input. */

    bbaif_input (netif);
}

void net_init (void)
{
    struct ip_addr  ipaddr;
    struct ip_addr  netmask;
    struct ip_addr  gw;
    struct netif   *netif;

    /* STAGE: Initialize lwIP. */

    mem_init ();
    memp_init ();
    pbuf_init (); 
    netif_init ();
    ip_init ();
    udp_init ();
    //tcp_init();

    /*
        STAGE: Static IP configuration.
    
        TODO: Replace with DHCP!
    */

    IP4_ADDR(&gw, 10,1,1,254);
    IP4_ADDR(&ipaddr, 10,1,1,71);
    IP4_ADDR(&netmask, 255,0,0,0);

    /* STAGE: Enable the BBA network interface. */

    netif = netif_add (&ipaddr, &netmask, &gw, bbaif_init, ip_input);
  
    netif_set_default (netif);
}
