#include "contiki.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-sr.h"
#include "net/mac/tsch/tsch.h"
#include "net/routing/routing.h"
#include "net/routing/rpl-classic/rpl.h"

#define DEBUG DEBUG_PRINT
#define LOG_MODULE "NodeMapper"
#define LOG_LEVEL LOG_LEVEL_INFO
#include "net/ipv6/uip-debug.h"

#define MAX_NODE_MAP_SIZE 10

typedef struct {
  uip_ipaddr_t ip;
  uint16_t node_id;
  uint8_t used;
} node_map_entry_t;

const uip_ipaddr_t *malicieux_ip = NULL;
static node_map_entry_t node_map[MAX_NODE_MAP_SIZE];

uint16_t get_node_id_from_ip(const uip_ipaddr_t *ipaddr) {
  return UIP_HTONS(ipaddr->u8[14] << 8 | ipaddr->u8[15]);
}

void add_to_node_map(const uip_ipaddr_t *ip, uint16_t node_id) {
  for(int i = 0; i < MAX_NODE_MAP_SIZE; i++) {
    if(node_map[i].used && uip_ipaddr_cmp(&node_map[i].ip, ip)) {
      node_map[i].node_id = node_id;
      return;
    }
  }

  for(int i = 0; i < MAX_NODE_MAP_SIZE; i++) {
    if(!node_map[i].used) {
      uip_ipaddr_copy(&node_map[i].ip, ip);
      node_map[i].node_id = node_id;
      node_map[i].used = 1;
      return;
    }
  }

  printf("Node map full! Cannot add new entry.\n");
}

void print_routing_table() {
  uip_ds6_route_t *r;
  printf("==== Routing Table ====\n");
  for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
    const uip_ipaddr_t *nexthop = uip_ds6_route_nexthop(r);
    printf("Route to: ");
    uip_debug_ipaddr_print(&r->ipaddr);
    printf(" via ");
    uip_debug_ipaddr_print(nexthop);
    printf("\n");
  }
  printf("Routing entries: %u\n", uip_ds6_route_num_routes());
}

PROCESS(node_process, "RPL Node");
AUTOSTART_PROCESSES(&node_process);

PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();

#if CONTIKI_TARGET_COOJA || CONTIKI_TARGET_Z1
  if(node_id == 1) {
    NETSTACK_ROUTING.root_start();
  }
#endif

  NETSTACK_MAC.on();

#if WITH_PERIODIC_ROUTES_PRINT
  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND * 2);

  while(1) {
    uip_ds6_nbr_t *nbr;
    for(nbr = uip_ds6_nbr_head(); nbr != NULL; nbr = uip_ds6_nbr_next(nbr)) {
      const uip_ipaddr_t *ipaddr = &nbr->ipaddr;
      uint16_t node_Id = get_node_id_from_ip(ipaddr) / 256;
      if(node_Id <= MAX_NODE_MAP_SIZE) {
        add_to_node_map(ipaddr, node_Id);
      } else {
        malicieux_ip = ipaddr;
        printf("\nInvalid node detected\nNode ID: %u\nIP address: ", node_Id);
        uip_debug_ipaddr_print(malicieux_ip);
        printf("\n");

        printf("\n[Before Removal]\n");
        print_routing_table();

        // Remove the route whose global address matches the link-local suffix
        uip_ds6_route_t *r;
        for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
          if(memcmp(&r->ipaddr.u8[8], &malicieux_ip->u8[8], 8) == 0) {
            printf("\nRemoving route to: ");
            uip_debug_ipaddr_print(&r->ipaddr);
            printf("\n");
            uip_ds6_route_rm(r);
            break;
          }
        }

        printf("\n[After Removal]\n");
        print_routing_table();
      }
    }

    // Print node map
    printf("==== Node Map ====\n");
    for(int i = 0; i < MAX_NODE_MAP_SIZE; i++) {
      if(node_map[i].used) {
        printf("IP: ");
        uip_debug_ipaddr_print(&node_map[i].ip);
        printf(" -> Node ID: %u\n", node_map[i].node_id);
      }
    }

    // Print routing table (final)
    print_routing_table();

    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }
#endif

  PROCESS_END();
}

