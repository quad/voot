/*  bbaif.h

    $Id: bbaif.h,v 1.4 2003/03/06 07:37:56 quad Exp $

*/

#ifndef __LWIP_BBAIF__
#define __LWIP_BBAIF__

#include <netif/etharp.h>

/* NOTE: Module definitions. */

err_t   bbaif_output    (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
bool    bbaif_input     (struct netif *netif);
void    bbaif_set_netif (struct netif *netif);
err_t   bbaif_init      (struct netif *netif);

#endif
