/*  bbaif.h

    $Id: bbaif.h,v 1.2 2002/11/24 14:56:46 quad Exp $

*/

#ifndef __LWIP_BBAIF__
#define __LWIP_BBAIF__

#include <netif/etharp.h>

/* NOTE: Module definitions. */

err_t   bbaif_output    (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
bool    bbaif_input     (struct netif *netif);
void    bbaif_init      (struct netif *netif);

#endif
