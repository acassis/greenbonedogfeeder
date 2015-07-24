/****************************************************************************
 * Driver for STM32 2 digits 7 segments display
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <debug.h>

#include "up_arch.h"
#include "up_internal.h"

#include "stm32_gpio.h"

/****************************************************************************
 * HW access
 ****************************************************************************/

#define GPIO_DSP1_A (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN11)
#define GPIO_DSP1_B (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN12)
#define GPIO_DSP1_C (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN6)
#define GPIO_DSP1_D (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN8)
#define GPIO_DSP1_E (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN9)
#define GPIO_DSP1_F (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN10)
#define GPIO_DSP1_G (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN7)
#define GPIO_DSP1_DP (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN7)

#define GPIO_DSP2_A (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN2)
#define GPIO_DSP2_B (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN1)
#define GPIO_DSP2_C (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN7)
#define GPIO_DSP2_D (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN5)
#define GPIO_DSP2_E (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN4)
#define GPIO_DSP2_F (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN3)
#define GPIO_DSP2_G (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN6)
#define GPIO_DSP2_DP (GPIO_OUTPUT | GPIO_CNF_OUTPP | GPIO_MODE_50MHz| GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN0)

uint32_t digits1[10] = {0x01<<6, 0x3d<<6, 0x12<<6, 0x18<<6, 0x2c<<6, 0x48<<6, 0x40<<6, 0x1d<<6, 0x00<<6, 0x08<<6};
uint32_t digits2[10] = {0x41, 0x7D, 0x89, 0x19, 0x35, 0x13, 0x03, 0x79, 0x01, 0x11};

/****************************************************************************
 * LEDs: Fileops Prototypes and Structures
 ****************************************************************************/

double floor(double x);

typedef FAR struct file		file_t;

static int     display_open(file_t *filep);
static int     display_close(file_t *filep);
static ssize_t display_read(file_t *filep, FAR char *buffer, size_t buflen);
static ssize_t display_write(file_t *filep, FAR const char *buf, size_t buflen);

static const struct file_operations display_ops = {
	display_open,		/* open */
	display_close,		/* close */
	display_read,		/* read */
	display_write,		/* write */
	0,			/* seek */
	0,			/* ioctl */
};

/****************************************************************************
 * Keypad: Fileops
 ****************************************************************************/

static int display_open(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static int display_close(file_t *filep)
{
	/* Nothing to do */

	return OK;
}

static ssize_t display_read(file_t *filep, FAR char *buf, size_t buflen)
{
	register uint8_t reg;

	if(buf == NULL || buflen < 1)
		/* Well... nothing to do */
		return -EINVAL;

	*buf = 'A';

	return 1;
}

static ssize_t display_write(file_t *filep, FAR const char *buf, size_t buflen)
{
	register uint8_t value, dgt1, dgt2;
	uint32_t val;

	if(buf == NULL || buflen < 1)
		/* Well... nothing to do */
		return -EINVAL;

	value = *buf;

	if (value >= 100)
		return 1;

	printf("Trying to write %d\n", value);

	dgt1 = floor(value / 10);
        dgt2 = value - (dgt1 * 10);

        val = getreg32(STM32_GPIOC_ODR);
        val = val & 0xFFFFE03F;
        val = val | digits1[dgt1];
        putreg32(val, STM32_GPIOC_ODR);

        val = getreg32(STM32_GPIOD_ODR);
        val = val & 0xFFFFFF00;
        val = val | digits2[dgt2];
        putreg32(val, STM32_GPIOD_ODR);

        //putreg32(digits1[dgt1], STM32_GPIOC_ODR);
        //putreg32(digits2[dgt2], STM32_GPIOD_ODR);

	return 1;
}


/****************************************************************************
 * Initialize device, add /dev/... nodes
 ****************************************************************************/

void up_display(void)
{
	int ret;
	uint32_t val;

	lldbg("Creating Display Driver\n");

	stm32_configgpio(GPIO_DSP1_A);
	stm32_configgpio(GPIO_DSP1_B);
	stm32_configgpio(GPIO_DSP1_C);
	stm32_configgpio(GPIO_DSP1_D);
	stm32_configgpio(GPIO_DSP1_E);
	stm32_configgpio(GPIO_DSP1_F);
	stm32_configgpio(GPIO_DSP1_G);
	stm32_configgpio(GPIO_DSP1_DP);

	stm32_configgpio(GPIO_DSP2_A);
	stm32_configgpio(GPIO_DSP2_B);
	stm32_configgpio(GPIO_DSP2_C);
	stm32_configgpio(GPIO_DSP2_D);
	stm32_configgpio(GPIO_DSP2_E);
	stm32_configgpio(GPIO_DSP2_F);
	stm32_configgpio(GPIO_DSP2_G);
	stm32_configgpio(GPIO_DSP2_DP);

	val = getreg32(STM32_GPIOC_ODR);
	val = val & 0xFFFFE03F;
	val = val | digits1[0];
	putreg32(val, STM32_GPIOC_ODR);

	val = getreg32(STM32_GPIOD_ODR);
	val = val & 0xFFFFFF00;
	val = val | digits2[0];
	putreg32(val, STM32_GPIOD_ODR);

	ret = register_driver("/dev/display", &display_ops, 0666, NULL);
	if (ret < 0){
          printf("register_driver() failed: %d\n", ret);
	}
}

double floor(double x)
{
	if(x>0)return (int)x;
	return (int)(x-0.9999999999999999);
}

