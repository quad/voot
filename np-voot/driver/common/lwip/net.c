/*  net.c

    $Id: net.c,v 1.11 2003/03/08 08:10:53 quad Exp $

DESCRIPTION

    Network interface layer between the common library and lwIP

TODO

    Implement TCP.

    Add support for LAN Adapter.

    Add support for modem via SLIP.

*/

#include <vars.h>
#include <anim.h>
#include <util.h>
#include <timer.h>
#include <init.h>

#include "bbaif.h"

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"

#include "net.h"

static anim_render_chain_f  old_anim_chain;
static np_reconf_handler_f  my_reconfigure_handler;

static void net_etharp_timer (void)
{
    static uint32   last_tick = 0;

    if (last_tick && ((time () - last_tick) >= (ARP_TMR_INTERVAL / 1000)))
    {
        etharp_tmr ();

        last_tick = time ();
    }
    else if (!last_tick)
        last_tick = time ();
}

static void net_dhcp_coarse_timer (void)
{
    static uint32   last_tick = 0;

    if (last_tick && ((time () - last_tick) >= DHCP_COARSE_TIMER_SECS))
    {
        dhcp_coarse_tmr ();

        last_tick = time ();
    }
    else if (!last_tick)
        last_tick = time ();
}

static void net_dhcp_fine_timer (void)
{
    static uint32   last_tick = 0;

    if (last_tick && (timer_gen_micro (timer_gen_count () - last_tick) >= (DHCP_FINE_TIMER_MSECS * 1000)))
    {
        dhcp_fine_tmr ();

        last_tick = timer_gen_count ();
    }
    else if (!last_tick)
        last_tick = timer_gen_count ();
}

static void my_anim_chain (uint16 anim_code_a, uint16 anim_code_b)
{
    if (netif_default)
    {
        net_etharp_timer ();

        net_dhcp_coarse_timer ();
        net_dhcp_fine_timer ();
    }

    if (old_anim_chain)
        return old_anim_chain (anim_code_a, anim_code_b);
}

static void net_handle_reconfigure ()
{
    /* STAGE: Release ownership of all interfaces. */

    netif_set_default (NULL);
    bbaif_set_netif (NULL);

    return my_reconfigure_handler ();
}

void net_handle_tx (void *owner)
{
    struct netif   *netif;

    netif = (struct netif *) owner;

    if (netif)
    {
        /* NOTE: Dummy function, for the moment. */
    }
}

void net_handle_rx (void *owner)
{
    struct netif   *netif;

    netif = (struct netif *) owner;

    if (netif)
    {
        /* NOTE: Dummy function, simply calls bbaif_input. */

        while (bbaif_input (netif));
    }
}

void net_init (void)
{
    struct ip_addr  ipaddr;
    struct ip_addr  netmask;
    struct ip_addr  gw;
    struct netif   *netif;

    /* STAGE: Initialize the various timers. */

    timer_init ();

    /* STAGE: Ensure we're latched on the animation vector. */

    if (!old_anim_chain)
    {
        anim_init ();

        anim_add_render_chain (my_anim_chain, &old_anim_chain);
    }

    /* STAGE: Ensure lwIP is initialized. */

    if (!netif_default)
    {
        mem_init ();
        memp_init ();
        pbuf_init (); 
        netif_init ();
        ip_init ();
        udp_init ();

        /* STAGE: Initial IP configuration. */

        IP4_ADDR(&gw, 0,0,0,0);
        IP4_ADDR(&ipaddr, 0,0,0,0);
        IP4_ADDR(&netmask, 0,0,0,0);

        /* STAGE: Enable the BBA network interface. */

        netif = netif_add (&ipaddr, &netmask, &gw, NULL, bbaif_init, ip_input);

        netif_set_default (netif);

        /* STAGE: Enable address resolution via DHCP. */

        dhcp_start (netif);
    }

    /* STAGE: Make sure we're soft-reset clean. */

    if (!my_reconfigure_handler)
        my_reconfigure_handler = np_add_reconfigure_chain (net_handle_reconfigure);
}
