/****************************************************************************
 * examples/hello/hello_main.c
 *
 *   Copyright (C) 2008, 2011-2012 Gregory Nutt. All rights reserved.
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
 * 3. Neither the name NuttX nor the names of its contributors may be
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "dogfeed.h"
#include "server.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/

#define DOGFEED_STACK_SIZE 1024
#define DOGFEED_PRIORITY 50

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

/*#define PERCENT_FOOD	1
#define PERIOD_FOOD	2
#define PERIOD_WATER	4
#define CONFIRM_MODE	8
#define KEY_UP          16
#define KEY_DOWN        32*/


#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif


int fd_led, fd_button, fd_display, fd_valve, fd_motor;

static const char *led = "/dev/leds";
static const char *button = "/dev/buttons";
static const char *display = "/dev/display";
static const char *valve = "/dev/valve";
static const char *motor = "/dev/motor";

static uint8_t period_food = 10;
static uint8_t period_water = 21;
static uint8_t percent_food = 50;
static uint8_t running = TRUE;
static uint8_t mode = 0;

static int     count = 0;
static int     miliseconds = 0;
static int     seconds = 0;
static int     minutes = 0;

void set_period_food(uint8_t pfood)
{
  if (pfood <= 24)
    {
      period_food = pfood;
      write_display(period_food);
    }
}

uint8_t get_period_food(void)
{
  return period_food;
}

void set_period_water(uint8_t pwater)
{
  if (pwater <= 24)
    {
      period_water = pwater;
      write_display(period_water);
    }
}

uint8_t get_period_water(void)
{
  return period_water;
}

void set_percent_food(uint8_t pfood)
{
  if (pfood < 100)
    {
      percent_food = pfood;
      write_display(percent_food);
    }
}

uint8_t get_percent_food(void)
{
  return percent_food;
}

void set_mode(uint8_t mod)
{
  if (mod == PERCENT_FOOD || mod == PERIOD_FOOD || mod == PERIOD_WATER || mod == CONFIRM_MODE)
    {
      mode = mod;
    }
  write_led(mod);
}

int increase_value(void)
{

   //we cannot change the display with it running
   if(running)
     return -1;

   if (mode == PERCENT_FOOD && percent_food < 99) {
       percent_food++;
       write_display(percent_food);
       return 0;
   }

   if (mode == PERIOD_FOOD && period_food < 24) {
       period_food++;
       write_display(period_food);
       return 0;
   }

   if (mode == PERIOD_WATER && period_water < 24) {
       period_water++;
       write_display(period_water);
       return 0;
   }

  //return error
  return -1;
}

int decrease_value(void)
{

   //we cannot change the display with it running
   if(running)
     return -1;

   if (mode == PERCENT_FOOD && percent_food > 0) {
       percent_food--;
       write_display(percent_food);
       return 0;
   }

   if (mode == PERIOD_FOOD && period_food > 0) {
       period_food--;
       write_display(period_food);
       return 0;
   }

   if (mode == PERIOD_WATER && period_water > 0) {
       period_water--;
       write_display(period_water);
       return 0;
   }

   //return error
   return -1;
}

uint8_t get_mode(void)
{
  return mode;
}

uint8_t read_button(void)
{
  uint8_t key;

  //Try to read a pressed button key
  read(fd_button, &key, 1);

  return key;
}

void write_led(uint8_t mled)
{
  if (mled == 0 || mled == PERCENT_FOOD || mled == PERIOD_FOOD || mled == PERIOD_WATER || mled == CONFIRM_MODE)
    write(fd_led, &mled, 1);
}

void write_display(uint8_t mdisp)
{
     write(fd_display, &mdisp, 1);
}

void set_confirm_mode(void)
{

  //if it is running don't do anything
  if (running)
     return;

  miliseconds = 0;
  seconds = 0;
  minutes = 0;
  count = 0;
  write_display(count);
  write_led(mode);
  set_running(TRUE);
}

void set_running(uint8_t status)
{
  if (status == TRUE || status == FALSE)
    running = status;
}

int dogfeed_main(int argc, char *argv[])
{
  uint8_t motor_enabled = FALSE, valve_enter_enabled = FALSE, valve_exit_enabled = FALSE;
  //FILE *fd_tsii;
  uint8_t btkey, lmode, blink = 0;
  int8_t max_count;
  int pid, i;

  /* Perform architecture-specific initialization (if available) */
 
  (void)nsh_archinitialize();

  /* Bring up the network */
 
  (void)nsh_netinit();

  fd_led = open(led, O_RDWR);
  if (fd_led < 0)
  {
      printf("Unable to open file %s\n", led);
      return -ENOENT;
  }

  fd_button = open(button, O_RDWR);
  if (fd_button < 0)
  {
      printf("Unable to open file %s\n", button);
      return -ENOENT;
  }

  fd_display = open(display, O_RDWR);
  if (fd_display < 0)
  {
      printf("Unable to open file %s\n", display);
      return -ENOENT;
  }

  fd_valve = open(valve, O_RDWR);
  if (fd_valve < 0)
  {
      printf("Unable to open file %s\n", valve);
      return -ENOENT;
  }

  fd_motor = open(motor, O_RDWR);
  if (fd_motor < 0)
  {
      printf("Unable to open file %s\n", motor);
      return -ENOENT;
  }

  /* Initial LED state */
  write_led(mode);

  /* Initial Display state */
  write_display(count);

  /* Then start the new daemon */

  pid = task_create("Dogfeeder Server", DOGFEED_PRIORITY,
                    DOGFEED_STACK_SIZE, create_server, NULL);
  if (pid < 0)
    {
      ndbg("Failed to start the Dogfeeder server: %d\n", pid);
      return pid;
    }

  //Main Loop, never return
  while(1) {
        btkey = read_button();

	//If a button key is pressed
	if (btkey) {
		printf ("Pressed KEY = %02X\n", btkey);
		if (btkey == PERCENT_FOOD){
			printf("Adjust Amount of Food!\n");
			set_mode(PERCENT_FOOD);
			count = get_percent_food();
			running = FALSE;
		}
		if (btkey == PERIOD_FOOD){
			printf("Adjust period to release food!\n");
			set_mode(PERIOD_FOOD);
			count = get_period_food();
			running = FALSE;
		}
		if (btkey == PERIOD_WATER){
			printf("Adjust water replacement period!\n");
			set_mode(PERIOD_WATER);
			count = get_period_water();
			running = FALSE;
		}
		if (btkey == CONFIRM_MODE){
			printf("Confirm configuration!\n");
			set_mode(CONFIRM_MODE);
			running = FALSE;
		}
		if (btkey == KEY_UP){
			printf("Pressed key UP!\n");
			if (!running) {
				count++;
			}
		}
		if (btkey == KEY_DOWN){
			printf("Pressed key DOWN!\n");
			if (!running) {
				count--;
			}
		}


		if (!running) {

                        lmode = get_mode();

                        if (lmode == PERCENT_FOOD)
                            max_count = 99;
                        else
                            max_count = 24;

			if (count < 0)
				count = 0;

			if (count > max_count)
				count = max_count;

			if (lmode == PERCENT_FOOD) {
				set_percent_food(count);
			}

			if (lmode == PERIOD_FOOD) {
				set_period_food(count);
			}

			if (lmode == PERIOD_WATER) {
				set_period_water(count);
			}

			if (lmode == CONFIRM_MODE) {
                                set_confirm_mode();
			}

		}
		for(i = 0; i < 1500000; i++){
			asm("nop");
		}

	}


	usleep(1); //wait 1ms

	if (running)
		miliseconds++;

	if (miliseconds>=50) {
		seconds++;
		miliseconds=0;

		write_led(blink);

		if (blink == 0)
			blink = 8;
		else
			blink = 0;
	}

	if (seconds>=60) {
		minutes++;
		seconds=0;
		write_display(minutes);
		if ((minutes % period_water) == 0)
			valve_enter_enabled = TRUE;
                if ((minutes % period_food) == 0)
                        motor_enabled = TRUE;
	}

	if(valve_enter_enabled) {
		if (percent_food < seconds)
			valve_enter_enabled = FALSE;
		write(fd_valve, &valve_enter_enabled, 1);
	}

	if(motor_enabled) {
		if (percent_food < (seconds * 2))
			motor_enabled = FALSE;
		write(fd_motor, &motor_enabled, 1);
	}
  }

  close(fd_led);
  close(fd_button);
  close(fd_display);
  close(fd_valve);

  return 0;
}

