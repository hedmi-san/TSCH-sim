#include "contiki.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-sr.h"
#include "net/mac/tsch/tsch.h"
#include "net/routing/routing.h"
#include "net/routing/rpl-classic/rpl.h"

#define DEBUG DEBUG_PRINT
// the node length should be set manuel every time          
#define NodeLength 10

#include "net/ipv6/uip-debug.h"

    uint16_t get_node_id_from_ip(const uip_ipaddr_t *ipaddr) {
  return UIP_HTONS(ipaddr->u8[14] << 8 | ipaddr->u8[15]);
}
uint16_t node_ids[NodeLength];
int node_count= 0;
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



#if WITH_PERIODIC_ROUTES_PRINT
  static struct etimer et;
  /* Print out routing tables every minute */
  etimer_set(&et, CLOCK_SECOND * 10);
  while(1) {
  
     
     uip_ds6_nbr_t *nbr;
     uint16_t NodeId;
     for(nbr = uip_ds6_nbr_head(); nbr != NULL;nbr = uip_ds6_nbr_next(nbr)) {

	  const uip_ipaddr_t *ipaddr = &nbr->ipaddr;
	  printf("Neighbor IP: ");
	  uip_debug_ipaddr_print(ipaddr);
	  printf("\n");
	  NodeId = get_node_id_from_ip(ipaddr)/256;
	  
	  printf("The Neighbor ID : %u\n", NodeId);

	  printf("\n");
	  // Check if NodeId is already in the list
        int already_exists = 0;
	for(int i = 0; i < node_count; i++) {
	  if(node_ids[i] == NodeId) {
	    already_exists = 1;
	    break;
	  }
	}
	if(!already_exists) {
	  if(node_count < NodeLength && NodeId <= NodeLength ) {
	     
	    node_ids[node_count] = NodeId;
	    node_count++;
	  } else if(NodeId > NodeLength) {
	    printf("The Node  Id Is Not Valide \n");
	  } else {printf(" List is full! Cannot add more IDs.\n");}
	  } else {
  printf("Node ID %u already exists in the list.\n", NodeId);}
	  
}







    

    /* Used for non-regression testing */
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
