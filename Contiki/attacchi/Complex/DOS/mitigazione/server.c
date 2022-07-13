/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "os/net/ipv6/uip.h" //aggiunta
#include "pt.h"
#include "sys/log.h"
#include "subprocess.h"
#include "os/net/mac/mac.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define INTERVALLO (60* CLOCK_SECOND)


static struct simple_udp_connection udp_conn;

PROCESS(udp_server_process, "UDP server");
PROCESS(calcola, "calcola RATE");
AUTOSTART_PROCESSES(&udp_server_process, &calcola);

/*---------------------------------------------------------------------------*/
uip_ipaddr_t ip[16];
int conta[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int i = 0;
float rate[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  LOG_INFO("Sending response.\n");
  simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
#endif /* WITH_SERVER_REPLY */
}


static void check_packet(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen){
           int trovato=0;
          for (int j = 0; j < i; j++) {
            if (uip_ip6addr_cmp(sender_addr, &ip[j])) {
              conta[j]++;
              trovato = 1;
              if (rate[j] < 0.0005) {
                udp_rx_callback(c, sender_addr, sender_port, receiver_addr, receiver_port, data, datalen);
              }
            }
          }
          if (!trovato) {
            ip[i]=*sender_addr;
            conta[i]++;
            i++;
          }
         }
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();
  //PT_INIT(&esempio_pt);
  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();
  //esempio(&esempio_pt);
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, check_packet);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(calcola, ev, data){
  static struct etimer tempo;

  PROCESS_BEGIN();
  etimer_set(&tempo, INTERVALLO);

  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&tempo));
    for (int j = 0; j < i; j++) {
      rate[j]=(float)conta[j]/INTERVALLO;
      conta[j]=0;
    }
    
    etimer_set(&tempo, INTERVALLO);
  }
  PROCESS_END();
}