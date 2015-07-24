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

/****************************************************************************
 * Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

#define AMOUNT_FOOD	1
#define PERIOD_FOOD	2
#define PERIOD_WATER	4
#define CONFIRM_MODE	8

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

static const char *led = "/dev/leds";
static const char *button = "/dev/buttons";
static const char *display = "/dev/display";
static const char *valve = "/dev/valve";

int dogfeed_main(int argc, char *argv[])
{
  int fd_led, fd_button, fd_display, fd_valve, i = 0;
  int miliseconds=0, seconds=0, minutes=0;
  //FILE *fd_tsii;
  uint8_t btkey, mode = 0, blink = 0;
  uint8_t period_food = 10, period_water = 30, amount_food = 50, running = TRUE, motor_enabled = FALSE, valve_enter_enabled = FALSE, valve_exit_enabled = FALSE;
  int8_t count = 0;

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

  write(fd_led, &mode, 1);

  write(fd_display, &count, 1);

  //Main Loop, never return
  while(1) {
	//Try to read a pressed button key
  	read(fd_button, &btkey, 1);

	//If a button key is pressed
	if (btkey) {
		printf ("Pressed KEY = %02X\n", btkey);
		if (btkey == 1){
			printf("Adjust Amount of Food!\n");
			mode = AMOUNT_FOOD;
			write(fd_led, &mode, 1);
			count = amount_food;
			running = FALSE;
		}
		if (btkey == 2){
			printf("Adjust period to release food!\n");
			mode = PERIOD_FOOD;
			write(fd_led, &mode, 1);
			count = period_food;
			running = FALSE;
		}
		if (btkey == 4){
			printf("Adjust water replacement period!\n");
			mode = PERIOD_WATER;
			write(fd_led, &mode, 1);
			count = period_water;
			running = FALSE;
		}
		if (btkey == 8){
			printf("Confirm configuration!\n");
			mode = CONFIRM_MODE;
			write(fd_led, &mode, 1);
			running = FALSE;
		}
		if (btkey == 16){
			printf("Pressed key UP!\n");
			if (!running) {
				count++;
			}
		}
		if (btkey == 32){
			printf("Pressed key DOWN!\n");
			if (!running) {
				count--;
			}
		}


		if (!running) {
			if(count < 0)
				count = 0;
			if(count > 99)
				count = 99;

			write(fd_display, &count, 1);

			if (mode == AMOUNT_FOOD) {
				amount_food = count;
			}

			if (mode == PERIOD_FOOD) {
				period_food = count;
			}

			if (mode == PERIOD_WATER) {
				period_water = count;
			}

			if (mode == CONFIRM_MODE) {
				miliseconds = 0;
				seconds = 0;
				minutes = 0;
				count = 0;
				write(fd_display, &count, 1);

				write(fd_led, &mode, 1);
				running = TRUE;
			}

		}
		for(i = 0; i < 1500000; i++){
			asm("nop");
		}

	}


	//write(fd_led, &count, 1);

	usleep(1); //wait 1ms

	if (running)
		miliseconds++;

	if (miliseconds>=50) {
		seconds++;
		miliseconds=0;

		write(fd_led, &blink, 1);

		if (blink == 0)
			blink = 8;
		else
			blink = 0;
	}

	if (seconds>=60) {
		minutes++;
		seconds=0;
		write(fd_display, &minutes, 1);
		if ((minutes % period_water) == 0)
			valve_enter_enabled = TRUE;
	}

	if(valve_enter_enabled) {
		if (amount_food < seconds)
			valve_enter_enabled = FALSE;
		write(fd_valve, &valve_enter_enabled, 1);
	}

  }

  close(fd_led);
  close(fd_button);
  close(fd_display);
  close(fd_valve);

  return 0;
}

