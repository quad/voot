/*  bbaif.c

    $Id: bbaif.c,v 1.3 2002/11/12 02:00:55 quad Exp $

DESCRIPTION

    lwIP interface to the RealTek 8139c network chipset driver.

    This module should only be accessed by the lwIP and network layers.

    CREDIT: Original ethernetif module, Copyright (c) 2001, 2002 Swedish
    Institute of Computer Science. All rights reserved. Author: Adam Dunkels
    <adam@sics.se>

TODO

    Implement an etharp timer.

    Add in the complete statistics package.

    Determine why the netstack goes into constant reset mode after sustained
    ping-flooding.

*/

#include <vars.h>
#include <rtl8139c.h>

#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/sys.h>

#include <netif/etharp.h>

#include "bbaif.h"

#define IFNAME0 'r'
#define IFNAME1 't'

static err_t low_level_init (struct netif *netif)
{
    rtl_t  *devif;

    /* STAGE: Allocate and initialize the device interface structure. */

    devif = mem_malloc (sizeof (rtl_t));

    if (!devif)
        return ERR_MEM;
  
    devif->mac      = &(netif->hwaddr[0]);
    devif->owner    = netif;

    /* STAGE: Update the network interface structure. */
    
    netif->state    = devif;

    if (rtl_init (netif->state))
        return ERR_OK;
    else
        return ERR_BUF;
}

static struct pbuf* low_level_input (rtl_t *devif)
{
    struct pbuf    *p;
    struct pbuf    *q;
    uint16          len;

    /*
        STAGE: Obtain the size of the packet and put it into the "len"
        variable.
     */

    len = rtl_rx_status (devif);

    if (len <= 0)
        return NULL;

    /* STAGE: We allocate a pbuf chain of pbufs from the pool. */

    p = pbuf_alloc (PBUF_LINK, len, PBUF_POOL);

    if (p)
    {
        /*
            STAGE: We iterate over the pbuf chain until we have read the
            entire packet into the pbuf.
        */

        for(q = p; q != NULL; q = q->next)
        {
            uint32  recv;

            recv  = rtl_rx (devif, q->payload, q->len);

            /* STAGE: If the packet was corrupt, abort. */

            if (!recv)
            {
                pbuf_free (p);
                p = NULL;

                break;
            }
        }

#ifdef LINK_STATS
        stats.link.recv++;
#endif /* LINK_STATS */      
    }
    else
    {
        /* STAGE: Drop the frame, no memory to take it. */

        rtl_rx (devif, NULL, 0);

        pbuf_free (p);
        p = NULL;

#ifdef LINK_STATS
        stats.link.memerr++;
        stats.link.drop++;
#endif /* LINK_STATS */      
    }

    return p;  
}

static err_t low_level_output (struct netif *netif, struct pbuf *p)
{
    rtl_t          *devif;
    struct pbuf    *q;

    devif = netif->state;

    /* STAGE: Queue up all the data by iterating through the pbuf. */

    for(q = p; q != NULL; q = q->next)
    {
        if (!rtl_tx_write (devif, q->payload, q->len))
        {
            /* STAGE: Something went wrong queuing for TX. */

            rtl_tx_abort (devif);

            return ERR_BUF;
        }
    }

    /* STAGE: Now all the data is queued, perform the transmission. */

    if (!rtl_tx_final (devif))
    {
        rtl_tx_abort (devif);

        return ERR_BUF;
    }
  
#ifdef LINK_STATS
    stats.link.xmit++;
#endif /* LINK_STATS */      

    return ERR_OK;
}

err_t bbaif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
    p = etharp_output (netif, ipaddr, p);

    if (p)
    {
        low_level_output (netif, p);

        /* STAGE: Clean any ARP left-overs. Needed in this lwIP CVS. */

        etharp_output_sent (p);
        p = NULL;
    }

    return ERR_OK;
}

void bbaif_input (struct netif *netif)
{
    rtl_t          *devif;
    struct eth_hdr *ethhdr;
    struct pbuf    *p;
    struct pbuf    *q;

    devif = netif->state;
    q = NULL;

    /*
        STAGE: Attempt to receive an actual packet, if we get one process
        it.
    */

    p = low_level_input (devif);

    if (p)
    {
#ifdef LINK_STATS
        stats.link.recv++;
#endif /* LINK_STATS */

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
                q = etharp_arp_input (netif, (struct eth_addr *) devif->mac, p);
                break;

            default :
                pbuf_free (p);
                break;
        }

        /*
            STAGE: If the etharp layer needs to transmit a resolution
            packet...

            TODO: This may be a bug waiting to happen! While we've just
            received a packet, there is no guarantee we can transmit. The
            current BBA TX logic will block until it can transmit - however, this won't always remain true.

            If possible, I would like to switch to a interrupt operated TX
            system with a queue for the packets.
        */

        if (q)
        {
            low_level_output (netif, q);
            pbuf_free (q);
        }
    }
}

void bbaif_init (struct netif *netif)
{
    netif->name[0]      = IFNAME0;
    netif->name[1]      = IFNAME1;
    netif->output       = bbaif_output;
    netif->linkoutput   = low_level_output;

    /* STAGE: Initialize the actual network harware. */
  
    low_level_init (netif);

    /* STAGE: Initialize the Ethernet/ARP layer. */

    etharp_init ();
}
