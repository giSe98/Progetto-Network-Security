#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/node-id.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (60 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static float var = 60;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer periodic_timer_DDOS;
  clock_time_t SEND_INTERVAL_DDOS = var * CLOCK_SECOND;
  static unsigned count;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  etimer_set(&periodic_timer_DDOS, random_rand() % SEND_INTERVAL_DDOS);
  while(1) {
    if(node_id == 3 || node_id == 16 || node_id == 9 || node_id == 6)
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer_DDOS));
    else
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      LOG_INFO("Sending request %u to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      if(node_id == 3 || node_id == 16 || node_id == 9 || node_id == 6) 
        snprintf(str, sizeof(str), "%d", count);
      else 
        snprintf(str, sizeof(str), "%d", count);
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      count++;
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    /* Add some jitter */
    if(node_id == 3 || node_id == 16 || node_id == 9 || node_id == 6)
      etimer_set(&periodic_timer_DDOS, SEND_INTERVAL_DDOS - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
    else
      etimer_set(&periodic_timer, 2*SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
    
    if(var > 1.10) {
      var = var - 0.03;
      SEND_INTERVAL_DDOS = var * CLOCK_SECOND;
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
