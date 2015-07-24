/****************************************************************************
 * drivers/sensors/mpl115a.c
 * Character driver for the Freescale MPL115A1 Barometer Sensor
 *
 *   Copyright (C) 2011, 2013 Gregory Nutt. All rights reserved.
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

#include <stdlib.h>
#include <fixedmath.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/kmalloc.h>
#include <nuttx/fs/fs.h>
#include <nuttx/spi/spi.h>
#include <nuttx/sensors/mpl115a.h>

#if defined(CONFIG_SPI) && defined(CONFIG_MPL115A)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private
 ****************************************************************************/

struct mpl115a_dev_s
{
  FAR struct spi_dev_s *spi; /* SPI interface */
  int16_t mpl115a_cal_a0;
  int16_t mpl115a_cal_b1;
  int16_t mpl115a_cal_b2;
  int16_t mpl115a_cal_c12;
};


/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline void mpl115a_configspi(FAR struct spi_dev_s *spi);
static uint8_t mpl115a_getreg8(FAR struct mpl115a_dev_s *priv, uint8_t regaddr);
static void mpl115a_updatecaldata(FAR struct mpl115a_dev_s *priv);
static uint16_t mpl115a_readpressure(FAR struct mpl115a_dev_s *priv);
static uint16_t mpl115a_readtemperature(FAR struct mpl115a_dev_s *priv);
static int mpl115a_getpressure(FAR struct mpl115a_dev_s *priv);

/* Character driver methods */

static int     mpl115a_open(FAR struct file *filep);
static int     mpl115a_close(FAR struct file *filep);
static ssize_t mpl115a_read(FAR struct file *, FAR char *, size_t);
static ssize_t mpl115a_write(FAR struct file *filep, FAR const char *buffer, size_t buflen);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_mpl115afops =
{
  mpl115a_open,
  mpl115a_close,
  mpl115a_read,
  mpl115a_write,
  0,
  0
#ifndef CONFIG_DISABLE_POLL
  , 0
#endif
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifndef CONFIG_SPI_OWNBUS
static inline void mpl115a_configspi(FAR struct spi_dev_s *spi)
{
  /* Configure SPI for the MPL115A */

  SPI_SETMODE(spi, SPIDEV_MODE0);
  SPI_SETBITS(spi, 8);
  SPI_SETFREQUENCY(spi, MPL115A_SPI_MAXFREQUENCY);
}
#endif

/****************************************************************************
 * Name: mpl115a_getreg8
 *
 * Description:
 *   Read from an 8-bit MPL115A register
 *
 ****************************************************************************/

static uint8_t mpl115a_getreg8(FAR struct mpl115a_dev_s *priv, uint8_t regaddr)
{
  uint8_t regval;

  /* If SPI bus is shared then lock and configure it */

#ifndef CONFIG_SPI_OWNBUS
  (void)SPI_LOCK(priv->spi, true);
  mpl115a_configspi(priv->spi);
#endif

  /* Select the MPL115A */

  SPI_SELECT(priv->spi, SPIDEV_PRESSURE, true);

  /* Send register to read and get the next byte */

  (void)SPI_SEND(priv->spi, regaddr);
  SPI_RECVBLOCK(priv->spi, &regval, 1);

  /* Deselect the MPL115A */

  SPI_SELECT(priv->spi, SPIDEV_PRESSURE, false);

  /* Unlock bus */

#ifndef CONFIG_SPI_OWNBUS
  (void)SPI_LOCK(priv->spi, false);
#endif

#ifdef CONFIG_MPL115A_REGDEBUG
  dbg("%02x->%02x\n", regaddr, regval);
#endif
  return regval;
}

/****************************************************************************
 * Name: mpl115a_updatecaldata
 *
 * Description:
 *   Update Calibration Coefficient Data
 *
 ****************************************************************************/

static void mpl115a_updatecaldata(FAR struct mpl115a_dev_s *priv)
{
  /* Get a0 coefficient */
  priv->mpl115a_cal_a0 = mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_A0_MSB << 1)) << 8;
  priv->mpl115a_cal_a0 |= mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_A0_LSB << 1));
  sndbg("a0 = %d\n", priv->mpl115a_cal_a0);

  /* Get b1 coefficient */
  priv->mpl115a_cal_b1 = mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_B1_MSB << 1)) << 8;
  priv->mpl115a_cal_b1 |= mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_B1_LSB << 1));
  sndbg("b1 = %d\n", priv->mpl115a_cal_b1);

  /* Get b2 coefficient */
  priv->mpl115a_cal_b2 = mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_B2_MSB << 1)) << 8;
  priv->mpl115a_cal_b2 |= mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_B2_LSB << 1));
  sndbg("b2 = %d\n", priv->mpl115a_cal_b2);

  /* Get c12 coefficient */
  priv->mpl115a_cal_c12 = mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_C12_MSB << 1)) << 8;
  priv->mpl115a_cal_c12 |= mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_C12_LSB << 1));
  sndbg("c12 = %d\n", priv->mpl115a_cal_c12);

  return;
}

/****************************************************************************
 * Name: mpl115a_readpressure
 *
 * Description:
 *   Read pressure from MPL115A
 */
static uint16_t mpl115a_readpressure(FAR struct mpl115a_dev_s *priv)
{
  uint16_t pressure;

  /* Start a new convertion */
  mpl115a_getreg8(priv, (MPL115A_CONVERT << 1));

  /* Delay 5ms */
  usleep(5000);

  pressure = mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_PADC_MSB << 1)) << 8;
  pressure |= mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_PADC_LSB << 1));

  sndbg("Pressure = %d\n", pressure >> 6);
  return (pressure >> 6);
}

/****************************************************************************
 * Name: mpl115a_readtemperature
 *
 * Description:
 *   Read temperature from MPL115A
 */
static uint16_t mpl115a_readtemperature(FAR struct mpl115a_dev_s *priv)
{
  uint16_t temperature;

  /* We don't need to force a new convertion, since readpressure() did it */

  temperature = mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_TADC_MSB << 1)) << 8;
  temperature |= mpl115a_getreg8(priv, MPL115A_BASE_CMD | (MPL115A_TADC_LSB << 1));

  sndbg("Temperature = %d\n", temperature >> 6);
  return (temperature >> 6);
}

/****************************************************************************
 * Name: mpl115a_getpressure
 *
 * Description:
 *   Calculate the Barometrc Pressure using the temperature compensated
 *   See Freescale AN3785 and MPL115A1 datasheet
 *
 ****************************************************************************/

static int mpl115a_getpressure(FAR struct mpl115a_dev_s *priv)
{
  int32_t c12x2, a1, a1x1, y1, a2x2, pcomp;
  uint16_t padc, tadc, pressure;

  /* Check if coefficient data were read correctly */
  if ( (priv->mpl115a_cal_a0 == 0) || (priv->mpl115a_cal_b1 == 0) ||
       (priv->mpl115a_cal_b2 == 0) || (priv->mpl115a_cal_c12 == 0) )
    {
      mpl115a_updatecaldata(priv);
    }

  /* Read temperature and pressure */
  padc = mpl115a_readpressure(priv);
  tadc = mpl115a_readtemperature(priv);

  /* These code are from Freescale AN3785 */
  c12x2 = ((int32_t)priv->mpl115a_cal_c12 * tadc) >> 11;
  a1 = (int32_t) (priv->mpl115a_cal_b1 + c12x2);
  a1x1 = a1 * padc;
  y1 = (((int32_t)priv->mpl115a_cal_a0) << 10) + a1x1;
  a2x2 = (((int32_t)priv->mpl115a_cal_b2) * tadc) >> 1;
  pcomp = (y1 + a2x2) >> 9;

  pressure = (((((int32_t)pcomp) * 1041) >> 14) + 800);

  sndbg("Final Pressure = %d\n", pressure);
  return pressure;
}

/****************************************************************************
 * Name: mpl115a_open
 *
 * Description:
 *   This function is called whenever the MPL115A1 device is opened.
 *
 ****************************************************************************/

static int mpl115a_open(FAR struct file *filep)
{
  return OK;
}

/****************************************************************************
 * Name: mpl115a_close
 *
 * Description:
 *   This routine is called when the LM-75 device is closed.
 *
 ****************************************************************************/

static int mpl115a_close(FAR struct file *filep)
{
  return OK;
}

/****************************************************************************
 * Name: mpl115a_read
 ****************************************************************************/

static ssize_t mpl115a_read(FAR struct file *filep, FAR char *buffer, size_t buflen)
{
  FAR struct inode         *inode = filep->f_inode;
  FAR struct mpl115a_dev_s *priv  = inode->i_private;
  int16_t                  press;

  if (!buffer)
    {
      sndbg("Buffer is null\n");
      return -1;
    }

  if ( buflen != 2)
    {
      sndbg("You need to read only 16 bits (2 bytes)\n");
      return -1;
    }

  /* Get the requested number of samples */
  press = mpl115a_getpressure(priv);
  buffer[0] = (press & 0xFF);
  buffer[1] = (press & 0xFF00) >> 8;

  return 2;
}

/****************************************************************************
 * Name: mpl115a_write
 ****************************************************************************/

static ssize_t mpl115a_write(FAR struct file *filep, FAR const char *buffer,
                          size_t buflen)
{
  return -ENOSYS;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: mpl115a_register
 *
 * Description:
 *   Register the MPL115A character device as 'devpath'
 *
 * Input Parameters:
 *   devpath - The full path to the driver to register. E.g., "/dev/temp0"
 *   i2c - An instance of the I2C interface to use to communicate with MPL115A
 *   addr - The I2C address of the LM-75.  The base I2C address of the MPL115A
 *   is 0x48.  Bits 0-3 can be controlled to get 8 unique addresses from 0x48
 *   through 0x4f.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int mpl115a_register(FAR const char *devpath, FAR struct spi_dev_s *spi)
{
  FAR struct mpl115a_dev_s *priv;
  int ret;

  /* Initialize the MPL115A device structure */

  priv = (FAR struct mpl115a_dev_s *)kmm_malloc(sizeof(struct mpl115a_dev_s));
  if (!priv)
    {
      sndbg("Failed to allocate instance\n");
      return -ENOMEM;
    }

  priv->spi             = spi;
  priv->mpl115a_cal_a0  = 0;
  priv->mpl115a_cal_b1  = 0;
  priv->mpl115a_cal_b2  = 0;
  priv->mpl115a_cal_c12 = 0;

  /* Read the coefficient value */
  mpl115a_updatecaldata(priv);

  /* Register the character driver */

  ret = register_driver(devpath, &g_mpl115afops, 0666, priv);
  if (ret < 0)
    {
      sndbg("Failed to register driver: %d\n", ret);
      kmm_free(priv);
    }

  return ret;
}
#endif /* CONFIG_SPI && CONFIG_MPL115A */
