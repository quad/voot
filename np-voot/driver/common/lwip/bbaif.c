/*  bbaif.c

    $Id: bbaif.c,v 1.8 2003/01/20 21:11:53 quad Exp $

DESCRIPTION

    lwIP interface to the RealTek 8139c network chipset driver.

    This module should only be accessed by the lwIP and network layers.

    CREDIT: Original ethernetif module, Copyright (c) 2001, 2002 Swedish
    Institute of Computer Science. All rights reserved. Author: Adam Dunkels
    <adam@sics.se>

*/

#include <vars.h>
#include <rtl8139c.h>

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "netif/etharp.h"

#include "bbaif.h"

#define IFNAME0 'r'
#define IFNAME1 't'

static err_t low_level_init (struct netif *netif)
{
    if (rtl_init ())
        return ERR_OK;
    else
        return ERR_BUF;
}

static struct pbuf* low_level_input ()
{
    struct pbuf    *p;
    struct pbuf    *q;
    uint16          len;

    /*
        STAGE: Obtain the size of the packet and put it into the "len"
        variable.
     */

    len = rtl_rx_status ();

    if (len <= 0)
        return NULL;

    /* STAGE: We allocate a pbuf chain of pbufs from the pool. */

    p = pbuf_alloc (PBUF_RAW, len, PBUF_POOL);

    if (p)
    {
        /*
            STAGE: We iterate over the pbuf chain until we have read the
            entire packet into the pbuf.
        */

        for(q = p; q != NULL; q = q->next)
        {
            uint32  recv;

            recv  = rtl_rx (q->payload, q->len);

            /* STAGE: If the packet was corrupt, abort. */

            if (!recv)
            {
                pbuf_free (p);
                p = NULL;

#ifdef LINK_STATS
                lwip_stats.link.lenerr++;
                lwip_stats.link.drop++;
#endif /* LINK_STATS */      

                break;
            }
        }

#ifdef LINK_STATS
        lwip_stats.link.recv++;
#endif /* LINK_STATS */      
    }
    else
    {
        /* STAGE: Drop the frame, no memory to take it. */

        rtl_rx (NULL, 0);

        pbuf_free (p);
        p = NULL;

#ifdef LINK_STATS
        lwip_stats.link.memerr++;
        lwip_stats.link.drop++;
#endif /* LINK_STATS */      
    }

    return p;  
}

static err_t low_level_output (struct netif *netif, struct pbuf *p)
{
    struct pbuf    *q;

    /* STAGE: Queue up all the data by iterating through the pbuf. */

    for(q = p; q != NULL; q = q->next)
    {
        if (!rtl_tx_write (q->payload, q->len))
        {
            /* STAGE: Something went wrong queuing for TX. */

            rtl_tx_abort ();

            return ERR_BUF;
        }
    }

    /* STAGE: Now all the data is queued, perform the transmission. */

    if (!rtl_tx_final ())
    {
        rtl_tx_abort ();

        return ERR_BUF;
    }
  
#ifdef LINK_STATS
    lwip_stats.link.xmit++;
#endif /* LINK_STATS */      

    return ERR_OK;
}

err_t bbaif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
    err_t   retval;

    retval = ERR_OK;

    p = etharp_output (netif, ipaddr, p);

    if (p)
        retval = low_level_output (netif, p);
    else
#ifdef LINK_STATS
        lwip_stats.link.err++;
#endif /* LINK_STATS */      

    return retval;
}

bool bbaif_input (struct netif *netif)
{
    struct eth_hdr *ethhdr;
    struct pbuf    *p;
    struct pbuf    *q;

    q = NULL;

    /*
        STAGE: Attempt to receive an actual packet, if we get one process
        it.
    */

    p = low_level_input ();

    if (p)
    {
        ethhdr = p->payload;

        switch (htons (ethhdr->type))
        {
            case ETHTYPE_IP :
            {
                q = etharp_ip_input (netif, p);
                pbuf_header (p, -14);
                netif->input (p, netif);

                break;
            }

            case ETHTYPE_ARP :
                q = etharp_arp_input (netif, (struct eth_addr *) &(netif->hwaddr), p);
                break;

            default :
#ifdef LINK_STATS
                lwip_stats.link.proterr++;
#endif /* LINK_STATS */

                pbuf_free (p);
                break;
        }

        /*
            STAGE: If the etharp layer needs to transmit a resolution
            packet...

            TODO: This may be a bug waiting to happen! While we've just
            received a packet, there is no guarantee we can transmit. The
            current BBA TX logic will block until it can transmit - however,
            this won't always remain true.

            If possible, I would like to switch to a interrupt operated TX
            system with a queue for the packets.
        */

        if (q)
        {
            low_level_output (netif, q);
            pbuf_free (q);
        }
    }

    return !!p;
}

void bbaif_set_netif (struct netif *netif)
{
    rtl_set_owner (netif);

    if (netif)
        rtl_mac (netif->hwaddr);
}

void bbaif_init (struct netif *netif)
{
    netif->name[0]      = IFNAME0;
    netif->name[1]      = IFNAME1;
    netif->output       = bbaif_output;
    netif->linkoutput   = low_level_output;
    netif->mtu          = 1500;

    /* STAGE: Initialize the actual network harware. */
  
    if (!(low_level_init (netif)))
    {
        /* STAGE: Notify the callbacks of the proper owner. */

        bbaif_set_netif (netif);

        /* STAGE: Initialize the Ethernet/ARP layer. */

        etharp_init ();
    }
}
