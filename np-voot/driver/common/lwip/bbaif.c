/*  bbaif.c

    $Id: bbaif.c,v 1.2 2002/11/08 19:47:49 quad Exp $

DESCRIPTION

    lwIP interface to the RealTek 8139c network chipset driver.

    This module should only be accessed by the lwIP and network layers.

    CREDIT: Original ethernetif module, Copyright (c) 2001, 2002 Swedish
    Institute of Computer Science. All rights reserved. Author: Adam Dunkels
    <adam@sics.se>

TODO

    Move the rtl_info structure from statically defined inside rtl8139c.o
    into the device information structure.

*/

#include <vars.h>

#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/sys.h>

#include <netif/etharp.h>

#include <rtl8139c.h>

#define IFNAME0 'r'
#define IFNAME1 't'

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

static void low_level_init (struct netif *netif)
{
    rtl_init (netif->state);
}

static err_t low_level_output (rtl_t *bbaif, struct pbuf *p)
{
    struct pbuf *q;

    for(q = p; q != NULL; q = q->next)
        rtl_tx_write (bbaif, q->payload, q->len);

    rtl_tx_final (bbaif);
  
#ifdef LINK_STATS
    stats.link.xmit++;
#endif /* LINK_STATS */      

    return ERR_OK;
}

static struct pbuf* low_level_input (rtl_t *bbaif)
{
    struct pbuf    *p;
    struct pbuf    *q;
    uint16          len;

    /*
        TODO: Obtain the size of the packet and put it into the "len"
        variable.
     */

    len = NULL;

    /* We allocate a pbuf chain of pbufs from the pool. */

    p = pbuf_alloc (PBUF_LINK, len, PBUF_POOL);
  
    if(p != NULL)
    {
        /*
            STAGE: We iterate over the pbuf chain until we have read the
            entire packet into the pbuf.
        */

        for(q = p; q != NULL; q = q->next)
        {
            /*
                STAGE: Read enough bytes to fill this pbuf in the chain. The
                avaliable data in the pbuf is given by the q->len variable.
            */

            read data into (q->payload, q->len);
        }

        acknowledge that packet has been read();

#ifdef LINK_STATS
        stats.link.recv++;
#endif /* LINK_STATS */      
    }
    else
    {
        drop packet ();

#ifdef LINK_STATS
        stats.link.memerr++;
        stats.link.drop++;
#endif /* LINK_STATS */      
    }

    return p;  
}

static void arp_timer (void *arg)
{
    etharp_tmr ();

    sys_timeout (ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
}

err_t bbaif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
    p = etharp_output (netif, ipaddr, p);

    if (p)
        return low_level_output (netif->state, p);

    return ERR_OK;
}

void bbaif_input (struct netif *netif)
{
    rtl_t          *devif;
    struct eth_hdr *ethhdr;
    struct pbuf    *p;
    struct pbuf    *q;

    q = NULL;
    devif = netif->state;

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
            low_level_output (devif, q);
            pbuf_free (q);
        }
    }
}

void bbaif_init (struct netif *netif)
{
    rtl_t  *devif;

    /* STAGE: Allocate the device interface structure. */

    devif = mem_malloc (sizeof (rtl_t));

    if (!devif)
        return;

    /* STAGE: Initialize the network interface structure. */
    
    netif->state = devif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = bbaif_output;
    netif->linkoutput = low_level_ouput;

    /* STAGE: Allocate and initialize the device interface structure. */
  
    devif->mac = &(netif->hwaddr[0]);

    /* STAGE: Initialize the actual network harware. */
  
    low_level_init (netif);

    /* STAGE: Initialize the Ethernet/ARP layer. */

    etharp_init ();

    /* TODO: Determine how the ARP logic works, and get it working right. */

    sys_timeout (ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
}
