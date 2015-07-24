README
======

  This README discusses issues unique to NuttX configurations for the
  GreenBone STM32F107.

  The board include:

    - 6 User Keys: Food, Water, Period, Confirm, Up, Down;
    - 4 LEDs: (Food, Water, Confirmed) selected;
    - 7-segment display with 2 digits;
    - Full Bridge motor control;
    - I2C interface to measure food weight;
    - Ethernet 10/100Mbps.

Contents
========

  o User keys
  o LEDs
  o 7-SEG Display
  o Serial Console
    - Console Configuration
  o GreenBone DP83848 Ethernet Module
  o Toolchains
    - NOTE about Windows native toolchains
  o Configurations
    - Information Common to All Configurations
    - Configuration Sub-directories

User keys
=====================

  All pulled high and will be sensed low when depressed.

    SW2 PB11  Key FOOD
    SW3 PB0   Key WATER
    SW4 PA5   Key PERIOD
    SW5 PA3   Key TIME
    SW6 PE4   Key UP
    SW7 PE5   Key DOWN

LEDs
====

  There are four LEDs on the GreenBone STM32F107 board that can be controlled
  by software:  LED1 through LED4.  All pulled high and can be illuminated by
  driving the output to low

    LED1 PB15 (FOOD Selected)
    LED2 PB1  (WATER Selected)
    LED3 PA6  (PERIOD Selected)
    LED4 PA4  (TIME Selected)

7-SEG Display
=============

  There is a 7-SEG with 2 digits display connected to board this way:

  Digit 1: PC6 to PC12 (E, D, C, B, A, F, G)
  Digit 2: PD0 to PD7  (F, G, A, B, C, D, E, DP)

Serial Console
==============

  Console Configuration
  ---------------------
  The NuttX console is configured by default on USART1 at 115200 BAUD 8N1
  (8-bits, not parity, one stop bit).  These setting can, of course, easily
  be changed by reconfiguring NuttX.

GreenBone DP83848 Ethernet Module
================================

  Ethernet Connector
  ------------------

    ----------------------------- ------------------------ --------------------------------
           Connector J2            GPIO CONFIGURATION(s)
    PIN SIGNAL        LEGEND         (no remapping)                 DP83848C Board
    --- ------------- ----------- ------------------------ --------------------------------
    1   PA0           MII_CRS     N/A                      N/C
    2   PB11/SDA2     COM_TX_EN   GPIO_ETH_RMII_TX_EN      TX_EN
    3   PA3/LED_G2    MII_COL     N/A                      N/C
    4   PB12/NSS2     COM_TXD0    GPIO_ETH_RMII_TXD0       TXD0
    5   PA1           MII_RX_CLK  GPIO_ETH_RMII_REF_CLK    OSCIN
    6   PB13/SCK2     COM_TXD1    GPIO_ETH_RMII_TXD1       TXD1
    7   PB1/CD_RESET  MII_RXD3    N/A                      N/C
    8   PC4/LCDTP     COM_RXD0    GPIO_ETH_RMII_RXD0       RXD0
    9   PB0/BL_PWM    MII_RXD2    N/A                      N/C
    10  PC5           COM_RXD1    GPIO_ETH_RMII_RXD1       RXD1
    11  PB8/CAN1_RX   MII_TXD3    N/A                      N/C
    12  PC1/LED_R1    COM_MDC     GPIO_ETH_MDC             MDC
    13  PC2/LED_R2    MII_TXD2    N/A                      N/C
    14  PA2/LED_G1    COM_MDIO    GPIO_ETH_MDIO            MDIO
    15  PC3/ONEW      MII_TX_CLK  N/A                      N/C
    16  PB10/SCL2     RX_ER       N/A                      N/C
    17  PD2           GPIO1       N/A                      N/C
    18  PA7/MOSI1     COM_RX_DV   GPIO_ETH_RMII_CRS_DV     CRS_DIV
    19  PD3           GPIO2       N/A                      N/C
    20  PB5           COM_PPS_OUT N/A                      N/C
    21  VDD 3.3       VDD_3.3     N/A                      3.3V
    22  VDD 3.3       VDD_3.3     N/A                      3.3V
    23  GND           GND         N/A                      GND
    24  GND           GND         N/A                      GND
    --- ------------- ----------- ------------------------ --------------------------------

    NOTES:
    1. RMII interface is used
    2. There is a 50MHz clock on board the DP83848.  No MCO clock need be provided.

  Configuration
  -------------

    System Type -> STM32 Peripheral Support
      CONFIG_STM32_ETHMAC=y                  : Enable Ethernet driver

    System Type -> Ethernet MAC Configuration
      CONFIG_STM32_RMII=y                    : Configuration RM-II DP83848C PHY
      CONFIG_STM32_AUTONEG=y
      CONFIG_STM32_PHYADDR=1
      CONFIG_STM32_PHYSR=16
      CONFIG_STM32_PHYSR_SPEED=0x0002
      CONFIG_STM32_PHYSR_100MBPS=0x0000
      CONFIG_STM32_PHYSR_MODE=0x0004
      CONFIG_STM32_PHYSR_FULLDUPLEX=0x0004
      CONFIG_STM32_RMII_EXTCLK=y

    Device Drivers -> Networking Devices
      CONFIG_NETDEVICES=y                    : More PHY stuff
      CONFIG_ETH0_PHY_DP83848C=y

    Networking (required)
      CONFIG_NET=y                           : Enabled networking support
      CONFIG_NET_MULTIBUFFER=y               : Required by driver
      CONFIG_NSH_NOMAC=y

    Networking (recommended/typical)
      CONFIG_NSOCKET_DESCRIPTORS=10          : Socket-related
      CONFIG_NET_SOCKOPTS=y

      CONFIG_NET_ETH_MTU=650                 : Maximum packet size
      CONFIG_NET_ETH_TCP_RECVWNDO=650
      CONFIG_NET_TCP_READAHEAD=y             : Enable read-ahead buffering
      CONFIG_NET_TCP_READAHEAD_BUFSIZE=650

      CONFIG_NET_TCP=y                       : TCP support
      CONFIG_NET_NTCP_READAHEAD_BUFFERS=8

      CONFIG_NET_UDP=y                       : UDP support
      CONFIG_NET_UDP_CONNS=8

      CONFIG_NET_ICMP=y                      : ICMP support
      CONFIG_NET_ICMP_PING=y

      CONFIG_NSH_DRIPADDR=0x0a000001         : Network identity
      CONFIG_NSH_IPADDR=0x0a000002
      CONFIG_NSH_NETMASK=0xffffff00

    Network Utilities (basic)
      CONFIG_NETUTILS_TFTPC=y                : Needed by NSH unless to disable TFTP commands
      CONFIG_NETUTILS_DHCPC=y                : Fun stuff
      CONFIG_NETUTILS_TELNETD=y              : Support for a Telnet NSH console
      CONFIG_NSH_TELNET=y

      (also FTP, TFTP, WGET, NFS, etc. if you also have a mass storage
      device).

Toolchains
==========

  NOTE about Windows native toolchains
  ------------------------------------

  There are several limitations to using a Windows based toolchain in a
  Cygwin environment.  The three biggest are:

  1. The Windows toolchain cannot follow Cygwin paths.  Path conversions are
     performed automatically in the Cygwin makefiles using the 'cygpath'
     utility but you might easily find some new path problems.  If so, check
     out 'cygpath -w'

  2. Windows toolchains cannot follow Cygwin symbolic links.  Many symbolic
     links are used in Nuttx (e.g., include/arch).  The make system works
     around these problems for the Windows tools by copying directories
     instead of linking them.  But this can also cause some confusion for
     you:  For example, you may edit a file in a "linked" directory and find
     that your changes had no effect.  That is because you are building the
     copy of the file in the "fake" symbolic directory.  If you use a\
     Windows toolchain, you should get in the habit of making like this:

       make clean_context all

     An alias in your .bashrc file might make that less painful.

  3. Dependencies are not made when using Windows versions of the GCC.  This is
     because the dependencies are generated using Windows pathes which do not
     work with the Cygwin make.

       MKDEP                = $(TOPDIR)/tools/mknulldeps.sh

Configurations
==============

  Information Common to All Configurations
  ----------------------------------------
  Each SAM3U-EK configuration is maintained in a sub-directory and
  can be selected as follow:

    cd tools
    ./configure.sh greenbone-stm32f107/<subdir>
    cd -
    . ./setenv.sh

  Before sourcing the setenv.sh file above, you should examine it and perform
  edits as necessary so that TOOLCHAIN_BIN is the correct path to the directory
  than holds your toolchain binaries.

  And then build NuttX by simply typing the following.  At the conclusion of
  the make, the nuttx binary will reside in an ELF file called, simply, nuttx.

    make

  The <subdir> that is provided above as an argument to the tools/configure.sh
  must be is one of the following.

  NOTES:

  1. These configurations use the mconf-based configuration tool.  To
    change any of these configurations using that tool, you should:

    a. Build and install the kconfig-mconf tool.  See nuttx/README.txt
       and misc/tools/

    b. Execute 'make menuconfig' in nuttx/ in order to start the
       reconfiguration process.

  2. Unless stated otherwise, all configurations generate console
     output on USART1.

  3. Unless otherwise stated, the configurations are setup for
     Cygwin under Windows:

     Build Setup:
       CONFIG_HOST_WINDOWS=y                   : Windows operating system
       CONFIG_WINDOWS_CYGWIN=y                 : POSIX environment under windows

  4. All of these configurations use the CodeSourcery for Windows toolchain
     (unless stated otherwise in the description of the configuration).  That
     toolchain selection can easily be reconfigured using 'make menuconfig'.
     Here are the relevant current settings:

     System Type -> Toolchain:
       CONFIG_ARMV7M_TOOLCHAIN_CODESOURCERYW=y : GNU EABI toolchain for windows

     The setenv.sh file is available for you to use to set the PATH
     variable.  The path in the that file may not, however, be correct
     for your installation.

     See also the "NOTE about Windows native toolchains" in the section call
     "GNU Toolchain Options" above.

  4. These configurations all assume that the STM32F107VCT6 is mounted on
     board.  This is configurable; you can select the STM32F103VCT6 as an
     alternative.

  5. These configurations all assume that you are loading code using
     something like the ST-Link v2 JTAG.  None of these configurations are
     setup to use the DFU bootloader but should be easily reconfigured to
     use that bootloader is so desired.

  Configuration Sub-directories
  -----------------------------

  netnsh:

    This configuration directory provide the NuttShell (NSH) with
    networking support.

    NOTES:
    1. This configuration will work only on the version the greenbone
       board with the the STM32F107VCT6 installed.  If you have a board
       with the STM32F103VCT6 installed, please use the nsh configuration
       described below.

    2. There is no PHY on the base greenbone stm32f107 board.  You must
       also have the "GreenBone DP83848 Ethernet Module" installed on J2
       in order to support networking.

    3. Since networking is enabled, you will see some boot-up delays when
       the network connection is established.  These delays can be quite
       large if no network is attached (A production design to bring up the
       network asynchronously to avoid these start up delays).

    4. This configuration uses the default USART1 serial console.  That
       is easily changed by reconfiguring to (1) enable a different
       serial peripheral, and (2) selecting that serial peripheral as
       the console device.

    5. By default, this configuration is set up to build on Windows
       under either a Cygwin or MSYS environment using a recent, Windows-
       native, generic ARM EABI GCC toolchain (such as the CodeSourcery
       toolchain).  Both the build environment and the toolchain
       selection can easily be changed by reconfiguring:

       CONFIG_HOST_WINDOWS=y                   : Windows operating system
       CONFIG_WINDOWS_CYGWIN=y                 : POSIX environment under windows
       CONFIG_ARMV7M_TOOLCHAIN_CODESOURCERYW=y : CodeSourcery for Windows

    6. USB support is disabled by default.  See the section above entitled,
       "USB Interface"

    STATUS.  The first time I build the configuration, I get some undefined
    external references.  No idea why.  Simply cleaning the apps/ directory
    and rebuilding fixes the problem:

      make apps_clean all

  nsh:

    This configuration directory provide the basic NuttShell (NSH).

    NOTES:
    1. This configuration will work with either the version of the board
       with STM32F107VCT6 or STM32F103VCT6 installed.  The default
       configuration is for the STM32F107VCT6.  To use this configuration
       with a STM32F103VCT6, it would have to be modified as follows:

      System Type -> STM32 Configuration Options
         CONFIG_ARCH_CHIP_STM32F103VC=y
         CONFIG_ARCH_CHIP_STM32F107VC=n

    2. This configuration uses the default USART1 serial console.  That
       is easily changed by reconfiguring to (1) enable a different
       serial peripheral, and (2) selecting that serial peripheral as
       the console device.

    3. By default, this configuration is set up to build on Windows
       under either a Cygwin or MSYS environment using a recent, Windows-
       native, generic ARM EABI GCC toolchain (such as the CodeSourcery
       toolchain).  Both the build environment and the toolchain
       selection can easily be changed by reconfiguring:

       CONFIG_HOST_WINDOWS=y                   : Windows operating system
       CONFIG_WINDOWS_CYGWIN=y                 : POSIX environment under windows
       CONFIG_ARMV7M_TOOLCHAIN_CODESOURCERYW=y : CodeSourcery for Windows

    4. USB support is disabled by default.  See the section above entitled,
       "USB Interface"

    3. This configured can be re-configured to use either the GreenBone LCD
       module. NOTE:  The LCD module can only be used on the STM32F103 version
       of the board.  The LCD requires FSMC support.

          System Type -> STM32 Chip Selection:
            CONFIG_ARCH_CHIP_STM32F103VC=y      : Select STM32F103VCT6

          System Type -> Peripherals:
            CONFIG_STM32_FSMC=y                   : Enable FSMC LCD interface

          Device Drivers -> LCD Driver Support
            CONFIG_LCD=y                          : Enable LCD support
            CONFIG_NX_LCDDRIVER=y                 : LCD graphics device
            CONFIG_LCD_MAXCONTRAST=1
            CONFIG_LCD_MAXPOWER=255
            CONFIG_LCD_LANDSCAPE=y                : Landscape orientation
            CONFIG_LCD_SSD1289=y                  : Select the SSD1289
            CONFIG_SSD1289_PROFILE1=y

          Graphics Support
            CONFIG_NX=y

          Graphics Support -> Supported Pixel Depths
            CONFIG_NX_DISABLE_1BPP=y              : Only 16BPP supported
            CONFIG_NX_DISABLE_2BPP=y
            CONFIG_NX_DISABLE_4BPP=y
            CONFIG_NX_DISABLE_8BPP=y
            CONFIG_NX_DISABLE_24BPP=y
            CONFIG_NX_DISABLE_32BPP=y

          Graphics Support -> Font Selections
            CONFIG_NXFONTS_CHARBITS=7
            CONFIG_NXFONT_SANS22X29B=y
            CONFIG_NXFONT_SANS23X27=y

          Application Configuration -> Examples
            CONFIG_EXAMPLES_NXLINES=y
            CONFIG_EXAMPLES_NXLINES_BGCOLOR=0x0320
            CONFIG_EXAMPLES_NXLINES_LINEWIDTH=16
            CONFIG_EXAMPLES_NXLINES_LINECOLOR=0xffe0
            CONFIG_EXAMPLES_NXLINES_BORDERWIDTH=4
            CONFIG_EXAMPLES_NXLINES_BORDERCOLOR=0xffe0
            CONFIG_EXAMPLES_NXLINES_CIRCLECOLOR=0xf7bb
            CONFIG_EXAMPLES_NXLINES_BPP=16

       STATUS: Not working; reads 0x8999 as device ID.  This may perhaps
               be due to incorrect jumper settings

    6. This configuration has been used for verifying the touchscreen on
       on the GreenBone LCD module.  NOTE:  The LCD module can really only
       be used on the STM32F103 version of the board.  The LCD requires
       FSMC support (the touchscreen, however, does not but the touchscreen
       is not very meaningful with no LCD).

          System Type -> STM32 Chip Selection:
           CONFIG_ARCH_CHIP_STM32F103VC=y    : Select STM32F103VCT6

       With the following modifications, you can include the touchscreen
       test program at apps/examples/touchscreen as an NSH built-in
       application.  You can enable the touchscreen and test by modifying
       the default configuration in the following ways:

          Device Drivers
            CONFIG_SPI=y                       : Enable SPI support
            CONFIG_SPI_EXCHANGE=y              : The exchange() method is supported
            CONFIG_SPI_OWNBUS=y                : Smaller code if this is the only SPI device

            CONFIG_INPUT=y                     : Enable support for input devices
            CONFIG_INPUT_ADS7843E=y            : Enable support for the XPT2046
            CONFIG_ADS7843E_SPIDEV=2           : Use SPI2 for communication
            CONFIG_ADS7843E_SPIMODE=0          : Use SPI mode 0
            CONFIG_ADS7843E_FREQUENCY=1000000  : SPI BAUD 1MHz
            CONFIG_ADS7843E_SWAPXY=y           : If landscape orientation
            CONFIG_ADS7843E_THRESHX=51         : These will probably need to be tuned
            CONFIG_ADS7843E_THRESHY=39

          System Type -> Peripherals:
            CONFIG_STM32_SPI2=y                : Enable support for SPI2

          RTOS Features:
            CONFIG_DISABLE_SIGNALS=n           : Signals are required

          Library Support:
            CONFIG_SCHED_WORKQUEUE=y           : Work queue support required

          Applicaton Configuration:
            CONFIG_EXAMPLES_TOUCHSCREEN=y      : Enable the touchscreen built-int test

          Defaults should be okay for related touchscreen settings.  Touchscreen
          debug output on USART1 can be enabled with:

          Build Setup:
            CONFIG_DEBUG=y                     : Enable debug features
            CONFIG_DEBUG_VERBOSE=y             : Enable verbose debug output
            CONFIG_DEBUG_INPUT=y               : Enable debug output from input devices

       STATUS: Working

  highpri:

    This configuration was used to verify the NuttX high priority, nested
    interrupt feature.  This is a board-specific test and probably not
    of much interest now other than for reference.

    This configuration targets the greenbone board with the STM32F103VCT6
    mounted.  It uses TIM6 to generated high rate interrupts for the test.
