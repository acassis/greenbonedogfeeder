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

#include "stm32_gpio.h"

/****************************************************************************
 * HW access
 ****************************************************************************/

#define LED_FOOD        (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN15)
#define LED_WATER       (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTB | GPIO_PIN1)
#define LED_PERIOD      (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTA | GPIO_PIN6)
#define LED_TIME        (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTA | GPIO_PIN4)

/****************************************************************************
 * LEDs: Fileops Prototypes and Structures
 ****************************************************************************/

typedef FAR struct file		file_t;

static int     leds_open(file_t *filep);
static int     leds_close(file_t *filep);
static ssize_t leds_read(file_t *filep, FAR char *buffer, size_t buflen);
static ssize_t leds_write(file_t *filep, FAR const char *buf, size_t buflen);

static const struct file_operations leds_ops = {
	leds_open,		/* open */
	leds_close,		/* close */
	leds_read,		/* read */
	leds_write,		/* write */
	0,			/* seek */
	0,			/* ioctl */
};

/****************************************************************************
 * Keypad: Fileops
 ****************************************************************************/

static int leds_open(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static int leds_close(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static ssize_t leds_read(file_t *filep, FAR char *buf, size_t buflen)
{
	register uint8_t reg;

	if(buf == NULL || buflen < 1)
		/* Well... nothing to do */
		return -EINVAL;

	reg = ~(stm32_gpioread(LED_TIME));
	reg = (reg << 1) | ~(stm32_gpioread(LED_PERIOD));
	reg = (reg << 1) | ~(stm32_gpioread(LED_WATER));
	reg = (reg << 1) | ~(stm32_gpioread(LED_FOOD));
	reg = reg & 0x0F;

	*buf = (char) reg;

	return 1;
}

static ssize_t leds_write(file_t *filep, FAR const char *buf, size_t buflen)
{
	register uint8_t reg;

	if(buf == NULL || buflen < 1)
		/* Well... nothing to do */
		return -EINVAL;

	reg = (uint8_t) *buf;

	stm32_gpiowrite(LED_FOOD, !(reg & 0x01));
	stm32_gpiowrite(LED_WATER, !(reg & 0x02));
	stm32_gpiowrite(LED_PERIOD, !(reg & 0x04));
	stm32_gpiowrite(LED_TIME, !(reg & 0x08));

	return 1;
}


/****************************************************************************
 * Initialize device, add /dev/... nodes
 ****************************************************************************/

void up_led(void)
{
	int ret;

	lldbg("Creating LED Driver\n");

	stm32_configgpio(LED_FOOD);
	stm32_configgpio(LED_WATER);
	stm32_configgpio(LED_PERIOD);
	stm32_configgpio(LED_TIME);

	ret = register_driver("/dev/leds", &leds_ops, 0444, NULL);
	if (ret < 0)
    	{
      		lldbg("register_driver failed: %d\n", ret);
		return;
    	}
}

