/****************************************************************************
 * apps/examples/dogfeed/server.c
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name Gregory Nutt nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/*************************************************************************** 
 * Dogfeeder Server Protocol
 ***************************************************************************
 AXX (65 XX XX) -> Set Food Percentage XX = 00-99
 BXX (66 XX XX) -> Set Food Period XX = 00-24
 CXX (67 XX XX)-> Set Water Period XX = 00-24
 D ?? -> Confirm Definition ?? Don't Care
 E ?? -> Read Food Percentage, return 00-99
 F ?? -> Read Food Period, return 00-24
 G ?? -> Read Water Period, return 00-24
****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sched.h>
#include <errno.h>
#include <debug.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <apps/netutils/netlib.h>

#include "dogfeed.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

#define RXBUFFER_SIZE 128
uint8_t mybuffer[RXBUFFER_SIZE] __attribute__ ((aligned (4)));

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: create_server
 *
 * Description:
 *   This function is the server itself.  It does not return (unless an
 *   error occurs).
 *
 * Parameters:
 *   None
 *
 * Return:
 *   Does not return unless an error occurs.
 *
 ****************************************************************************/

int create_server(void)
{
  struct sockaddr_in myaddr;
  socklen_t addrlen;
  int ret, optval;
  int listensd;
  int acceptsd;
  int connected;

  /* Create a new TCP socket to use to listen for connections */

  listensd = socket(PF_INET, SOCK_STREAM, 0);
  if (listensd < 0)
    {
      int errval = errno;
      dbg("socket failure: %d\n", errval);
      return -errval;
    }

  /* Set socket to reuse address */

  optval = 1;
  if (setsockopt(listensd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(int)) < 0)
    {
      ndbg("setsockopt SO_REUSEADDR failure: %d\n", errno);
      goto errout_with_socket;
    }

  /* Bind the socket to a local address */

  myaddr.sin_family      = AF_INET;
  myaddr.sin_port        = htons(3333);
  myaddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(listensd, (struct sockaddr*)&myaddr, sizeof(struct sockaddr_in)) < 0)
    {
      dbg("bind failure: %d\n", errno);
      goto errout_with_socket;
    }

  /* Listen for connections on the bound TCP socket */

  if (listen(listensd, 1) < 0)
    {
      dbg("listen failure %d\n", errno);
      goto errout_with_socket;
    }

  /* Begin accepting connections */

  for (;;)
    {
      nllvdbg("Accepting connections on port 3333\n");

      addrlen = sizeof(struct sockaddr_in);
      acceptsd = accept(listensd, (struct sockaddr*)&myaddr, &addrlen);
      if (acceptsd < 0)
        {
          nllvdbg("accept failed: %d\n", errno);
          goto errout_with_socket;
        }

      nllvdbg("Client connected!\n");
      connected = 1;

      while(connected) 
        {
          /* Receive data from client */

          ret = recv(acceptsd, mybuffer, RXBUFFER_SIZE, 0);

         /* Did we receive anything? */

          if (ret > 0)
          { 
             /* Yes.. Process the newly received data */
             
             int i, val, first_digit = 1;
             char dog_cmd = 0, value[5];

             for(i = 0; i < ret; i++)
             {
                printf("Received data[%i] = 0x%02x\n", i, mybuffer[i]);
                if (mybuffer[i] >= 'A' && mybuffer[i] <= 'I')
                  {
                    dog_cmd = mybuffer[i];
                  }
                  else
                    if (mybuffer[i] >= '0' && mybuffer[i] <= '9' || mybuffer[i] == '?')
                      {
                        // A valid command start with a letter
                        if (dog_cmd == 0)
                            continue;

                        if (first_digit)
                          {
                            value[0] = mybuffer[i];
                            first_digit = 0;
                          }
                          else 
                          {
                            value[1] = mybuffer[i];
                            value[2] = '\0';
                            val = atoi(value);

                            /* Set Food Percentage */
                            if ( dog_cmd == 'A' )
                              {
                                set_running(0);
                                set_mode(PERCENT_FOOD);
                                set_percent_food(val);
                              }

                            /* Set Food Percentage */
                            if ( dog_cmd == 'B' )
                              {
                                if ( val >= 0 && val <= 24 )
                                  {
                                    set_running(0);
                                    set_mode(PERIOD_FOOD);
                                    set_period_food(val);
                                  }
                              }
                            /* Set Water Period */
                            if ( dog_cmd == 'C' )
                              {
                                if ( val >= 0 && val <= 24 )
                                  {
                                     set_running(0);
                                     set_mode(PERIOD_WATER);
                                     set_period_water(val);
                                  }
                              }
                            /* Confirm Select Options */
                            if ( dog_cmd == 'D' )
                              {
                                set_confirm_mode();
                              }
                            /* Return Percentage of Food */
                            if ( dog_cmd == 'E' )
                              {
                                val = get_percent_food();
                                value[0] = 'E';
                                value[1] = (val / 10) + '0';
                                value[2] = (val % 10) + '0';
                                value[3] = '\n';
                                value[4] = '\0';

                                /* Send it to client*/
                                send(acceptsd, value, 5, 0);
                              }

                            /* Return Period of Food */
                            if ( dog_cmd == 'F' )
                              {
                                val = get_period_food();
                                value[0] = 'F';
                                value[1] = (val / 10) + '0';
                                value[2] = (val % 10) + '0';
                                value[3] = '\n';
                                value[4] = '\0';

                                /* Send it to client*/
                                send(acceptsd, value, 5, 0);
                              }

                            /* Return Period of Water */
                            if ( dog_cmd == 'G' )
                              {
                                val = get_period_water();
                                value[0] = 'G';
                                value[1] = (val / 10) + '0';
                                value[2] = (val % 10) + '0';
                                value[3] = '\n';
                                value[4] = '\0';

                                /* Send it to client*/
                                send(acceptsd, value, 5, 0);
                              }

                            /* Increase Value! */
                            if ( dog_cmd == 'I' )
                              {
                                increase_value();
                              }

                            /* Decrease Value! */
                            if ( dog_cmd == 'H' )
                              {
                                decrease_value();
                              }

                            dog_cmd = 0;
                            first_digit = 1;
                          }
                      }
                      else
                         {
                            //valid command is only A-G and 0-9
                            dog_cmd = 0;
                         }
                           
                
             }

             /* Disconnect and wait for new connection */

             connected = 0;
          }
          else
          {
             /* client closed connection (ret == 0) or other error (ret < 0)*/
             connected = 0;
          }
          /* Close socket descriptor */

          close(acceptsd);
          usleep(100000);
        }
    }

errout_with_acceptsd:
  close(acceptsd);

errout_with_socket:
  close(listensd);
  return 0;
}

