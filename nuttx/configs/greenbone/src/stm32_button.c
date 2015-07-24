/****************************************************************************
 * Driver for Calypso keypad hardware
 *
 *   Copyright (C) 2014 Alan Carvalho de Assis
 *   Author: Alan Carvalho de Assis <acassis@gmail.com>
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

#include <nuttx/config.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/fs/fs.h>

#include <stdint.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <debug.h>

#include "greenbone.h"

#include "stm32_gpio.h"

/****************************************************************************
 * HW access
 ****************************************************************************/

#define NUM_BUTTONS	6

#define KEY_FOOD        (GPIO_INPUT | GPIO_CNF_INFLOAT | GPIO_MODE_INPUT | GPIO_EXTI | GPIO_PORTB | GPIO_PIN14)
#define KEY_WATER       (GPIO_INPUT | GPIO_CNF_INFLOAT | GPIO_MODE_INPUT | GPIO_EXTI | GPIO_PORTB | GPIO_PIN0)
#define KEY_PERIOD      (GPIO_INPUT | GPIO_CNF_INFLOAT | GPIO_MODE_INPUT | GPIO_EXTI | GPIO_PORTA | GPIO_PIN5)
#define KEY_TIME        (GPIO_INPUT | GPIO_CNF_INFLOAT | GPIO_MODE_INPUT | GPIO_EXTI | GPIO_PORTA | GPIO_PIN3)
#define KEY_UP          (GPIO_INPUT | GPIO_CNF_INFLOAT | GPIO_MODE_INPUT | GPIO_EXTI | GPIO_PORTE | GPIO_PIN4)
#define KEY_DOWN        (GPIO_INPUT | GPIO_CNF_INFLOAT | GPIO_MODE_INPUT | GPIO_EXTI | GPIO_PORTE | GPIO_PIN5)

static const uint16_t g_buttons[NUM_BUTTONS] =
{
  KEY_FOOD, KEY_WATER, KEY_PERIOD, KEY_TIME, KEY_UP, KEY_DOWN
};

/****************************************************************************
 * LEDs: Fileops Prototypes and Structures
 ****************************************************************************/

typedef FAR struct file		file_t;

static int     button_open(file_t *filep);
static int     button_close(file_t *filep);
static ssize_t button_read(file_t *filep, FAR char *buffer, size_t buflen);
static ssize_t button_write(file_t *filep, FAR const char *buf, size_t buflen);

static const struct file_operations button_ops = {
	button_open,		/* open */
	button_close,		/* close */
	button_read,		/* read */
	button_write,		/* write */
	0,			/* seek */
	0,			/* ioctl */
};

/****************************************************************************
 * Keypad: Fileops
 ****************************************************************************/

static int button_open(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static int button_close(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static ssize_t button_read(file_t *filep, FAR char *buf, size_t buflen)
{
  register uint8_t ret = 0;
  int i;

  if(buf == NULL || buflen < 1)
	/* Well... nothing to do */
	return -EINVAL;

  /* Check that state of each key */

  for (i = 0; i < NUM_BUTTONS; i++)
    {
      /* A LOW value means that the key is pressed for most keys.  The exception
       * is the WAKEUP button.
       */

      bool released = stm32_gpioread(g_buttons[i]);

      /* Accumulate the set of depressed (not released) keys */

      if (!released)
        {
          ret |= (1 << i);
        }
    }

  /* Return a single bit will buttons states */
  *buf = (char) (ret & 0x3F);
  return 1;
}

static ssize_t button_write(file_t *filep, FAR const char *buf, size_t buflen)
{
	/* Normally we don't write to buttons */
	return -EINVAL;
}


/****************************************************************************
 * Initialize device, add /dev/... nodes
 ****************************************************************************/

void up_button(void)
{
  int i, ret;

  lldbg("Creating Buttons Driver\n");

  for (i = 0; i < NUM_BUTTONS; i++)
    {
      stm32_configgpio(g_buttons[i]);
    }

  ret = register_driver("/dev/buttons", &button_ops, 0444, NULL);
  if (ret < 0)
    {
      	lldbg("register_driver failed: %d\n", ret);
	return;
    }
}

