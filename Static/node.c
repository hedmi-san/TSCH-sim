#include "contiki.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-sr.h"
#include "net/mac/tsch/tsch.h"
#include "net/routing/routing.h"
#include "net/routing/rpl-classic/rpl.h"

#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"

PROCESS(node_process, "RPL Node");
AUTOSTART_PROCESSES(&node_process);


PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();
#if CONTIKI_TARGET_COOJA || CONTIKI_TARGET_Z1
  if(node_id == 1) { /* Coordinator node. */
    NETSTACK_ROUTING.root_start();
  }
#endif
  NETSTACK_MAC.on();
  // used for knowing every node parent  

printf("Node STATRETD ++++++++++++++++++++++++++++++++++++++++");

#if WITH_PERIODIC_ROUTES_PRINT
  static struct etimer et;
  /* Print out routing tables every minute */
  etimer_set(&et, CLOCK_SECOND * 2);
  while(1) {
    /* Used for non-regression testing */
    printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    #if (UIP_MAX_ROUTES != 0)
      PRINTF("Routing entries: %u\n", uip_ds6_route_num_routes());
    #endif
    #if (UIP_SR_LINK_NUM != 0)
      PRINTF("Routing links: %u\n", uip_sr_num_nodes());
    #endif
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }
#endif /* WITH_PERIODIC_ROUTES_PRINT */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
