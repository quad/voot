/*  bbaif.h

    $Id: bbaif.h,v 1.1 2002/11/12 02:00:55 quad Exp $

*/

#ifndef __LWIP_BBAIF__
#define __LWIP_BBAIF__

#include <netif/etharp.h>

/* NOTE: Module definitions. */

err_t   bbaif_output    (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
void    bbaif_input     (struct netif *netif);
void    bbaif_init      (struct netif *netif);

#endif
