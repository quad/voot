/*  bbaif.c

    $Id: bbaif.c,v 1.1 2002/11/07 02:22:48 quad Exp $

DESCRIPTION

    lwIP interface to the RealTek 8139c network chipset driver.

    This module should only be accessed by the lwIP and network layers.

    CREDIT: Original ethernetif module, Copyright (c) 2001, 2002 Swedish
    Institute of Computer Science. All rights reserved. Author: Adam Dunkels
    <adam@sics.se>

*/

#include "lwip/debug.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "netif/arp.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'r'
#define IFNAME1 't'

struct bbaif
{
    struct eth_addr    *ethaddr;

    /* TODO: Move rtl_t structure in here! */
};

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

static void low_level_init (struct netif *netif)
{
    struct ethernetif  *ethernetif;

    ethernetif = netif->state;
  
    /* STAGE: Cache the MAC address. */

    ethernetif->ethaddr->addr[0] = ;
    ethernetif->ethaddr->addr[1] = ;
    ethernetif->ethaddr->addr[2] = ;

    /* TODO: Finish the initialization process! */
}

static err_t low_level_output (struct ethernetif *ethernetif, struct pbuf *p)
{
    struct pbuf *q;

    initiate transfer ();
  
    for(q = p; q != NULL; q = q->next)
    {
        /*
            TODO: Send the data from the pbuf to the interface, one pbuf at
            a time. The size of the data in each pbuf is kept in the ->len
            variable.
        */

        send data from (q->payload, q->len);
    }

    signal that packet should be sent ();
  
#ifdef LINK_STATS
    stats.link.xmit++;
#endif /* LINK_STATS */      

    return ERR_OK;
}

static struct pbuf* low_level_input (struct ethernetif *ethernetif)
{
    struct pbuf    *p;
    struct pbuf    *q;
    uint16          len;

    /*
        TODO: Obtain the size of the packet and put it into the "len"
        variable.
     */

    len = ;

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

static err_t ethernetif_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
    struct ethernetif  *ethernetif;
    struct pbuf        *q;
    struct eth_hdr     *ethhdr;
    struct eth_addr    *dest;
    struct eth_addr     mcastaddr;
    struct ip_addr     *queryaddr;
    err_t               err;
    uint8               i;
  
    ethernetif = netif->state;

    /* STAGE: Make room for Ethernet header. */

    if (pbuf_header(p, 14) != 0)
    {
        /*
            STAGE: The pbuf_header() call shouldn't fail, but we allocate an
            extra pbuf just in case.
        */

        q = pbuf_alloc (PBUF_LINK, 14, PBUF_RAM);

        if(q == NULL)
        {
#ifdef LINK_STATS
          stats.link.drop++;
          stats.link.memerr++;
#endif /* LINK_STATS */      
          return ERR_MEM;
        }

        pbuf_chain (q, p);
        p = q;
    }

    /*
        STAGE: Construct Ethernet header. Start with looking up deciding
        which MAC address to use as a destination address. Broadcasts and
        multicasts are special, all other addresses are looked up in the ARP
        table.
    */

    queryaddr = ipaddr;
    if (ip_addr_isany (ipaddr) || ip_addr_isbroadcast (ipaddr, &(netif->netmask)))
    {
        dest = (struct eth_addr *) &ethbroadcast;
    }
    else if (ip_addr_ismulticast (ipaddr))
    {
        /* STAGE: Hash IP multicast address to MAC address. */

        mcastaddr.addr[0] = 0x01;
        mcastaddr.addr[1] = 0x0;
        mcastaddr.addr[2] = 0x5e;
        mcastaddr.addr[3] = ip4_addr2 (ipaddr) & 0x7f;
        mcastaddr.addr[4] = ip4_addr3 (ipaddr);
        mcastaddr.addr[5] = ip4_addr4 (ipaddr);

        dest = &mcastaddr;
    }
    else
    {
        if (ip_addr_maskcmp (ipaddr, &(netif->ip_addr), &(netif->netmask)))
        {
            /*
                STAGE: Use destination IP address if the destination is on
                the same subnet as we are.
            */

            queryaddr = ipaddr;
        }
        else
        {
            /*
                STAGE: Otherwise we use the default router as the address to
                send the Ethernet frame to.
            */

            queryaddr = &(netif->gw);
        }

        dest = arp_lookup (queryaddr);
    }


    /*
        STAGE: If the arp_lookup() didn't find an address, we send out an
        ARP query for the IP address.
    */
    if (dest == NULL)
    {
        q = arp_query (netif, ethernetif->ethaddr, queryaddr);

        if (q != NULL)
        {
            err = low_level_output (ethernetif, q);
            pbuf_free (q);

            return err;
        }

#ifdef LINK_STATS
        stats.link.drop++;
        stats.link.memerr++;
#endif /* LINK_STATS */          

        return ERR_MEM;
    }

    ethhdr = p->payload;

    for (i = 0; i < 6; i++)
    {
        ethhdr->dest.addr[i] = dest->addr[i];
        ethhdr->src.addr[i] = ethernetif->ethaddr->addr[i];
    }
  
    ethhdr->type = htons (ETHTYPE_IP);
  
    return low_level_output (ethernetif, p);
}

static void ethernetif_input (struct netif *netif)
{
    struct ethernetif  *ethernetif;
    struct eth_hdr     *ethhdr;
    struct pbuf        *p;

    ethernetif = netif->state;
  
    p = low_level_input (ethernetif);

    if(p != NULL)
    {
#ifdef LINK_STATS
        stats.link.recv++;
#endif /* LINK_STATS */

        ethhdr = p->payload;
    
        switch (htons (ethhdr->type))
        {
            case ETHTYPE_IP :
            {
                arp_ip_input (netif, p);
                pbuf_header (p, -14);
                netif->input (p, netif);

                break;
            }

           case ETHTYPE_ARP :
           {
                p = arp_arp_input (netif, ethernetif->ethaddr, p);

                if (p != NULL)
                {
                    low_level_output (ethernetif, p);
                    pbuf_free (p);
                }

                break;
            }

            default :
              pbuf_free (p);
              break;
        }
    }
}

static void arp_timer (void *arg)
{
    arp_tmr ();
    sys_timeout (ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
}

void ethernetif_init (struct netif *netif)
{
    struct ethernetif  *ethernetif;
    
    ethernetif = mem_malloc (sizeof (struct ethernetif));
    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = ethernetif_output;
    netif->linkoutput = low_level_output;
  
    ethernetif->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);
  
    low_level_init (netif);
    arp_init ();

    sys_timeout (ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
}
