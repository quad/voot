/*  net.h

    $Id: net.h,v 1.1 2002/11/12 02:00:55 quad Exp $

*/

#ifndef __LWIP_NET__
#define __LWIP_NET__

/* NOTE: Module definitions. */

void    net_handle_tx   (void *netif);
void    net_handle_rx   (void *netif);
void    net_init        (void);

#endif
