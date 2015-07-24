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

#define VALVE_ENTER	(GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTE | GPIO_PIN1)
#define VALVE_EXIT	(GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTE | GPIO_PIN0)

/****************************************************************************
 * LEDs: Fileops Prototypes and Structures
 ****************************************************************************/

typedef FAR struct file		file_t;

static int     valve_open(file_t *filep);
static int     valve_close(file_t *filep);
static ssize_t valve_read(file_t *filep, FAR char *buffer, size_t buflen);
static ssize_t valve_write(file_t *filep, FAR const char *buf, size_t buflen);

static const struct file_operations valve_ops = {
	valve_open,		/* open */
	valve_close,		/* close */
	valve_read,		/* read */
	valve_write,		/* write */
	0,			/* seek */
	0,			/* ioctl */
};

/****************************************************************************
 * Keypad: Fileops
 ****************************************************************************/

static int valve_open(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static int valve_close(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static ssize_t valve_read(file_t *filep, FAR char *buf, size_t buflen)
{
	register uint8_t reg;

	if(buf == NULL || buflen < 1)
		/* Well... nothing to do */
		return -EINVAL;

	return 1;
}

static ssize_t valve_write(file_t *filep, FAR const char *buf, size_t buflen)
{
	register uint8_t reg;

	if(buf == NULL || buflen < 1)
		/* Well... nothing to do */
		return -EINVAL;

	reg = (uint8_t) *buf;

	stm32_gpiowrite(VALVE_ENTER, (reg & 0x01));
	stm32_gpiowrite(VALVE_EXIT,  (reg & 0x02));

	return 1;
}


/****************************************************************************
 * Initialize device, add /dev/... nodes
 ****************************************************************************/

void up_valve(void)
{
	int ret;

	lldbg("Creating Valve Driver\n");

	stm32_configgpio(VALVE_ENTER);
	stm32_configgpio(VALVE_EXIT);

	ret = register_driver("/dev/valve", &valve_ops, 0444, NULL);
	if (ret < 0)
    	{
      		lldbg("register_driver failed: %d\n", ret);
		return;
    	}
}

