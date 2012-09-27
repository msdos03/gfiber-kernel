
General LSP information
=================================================

Contents:
---------
  1.  Default kernel configuration
  2.  Marvell LSP File locations
  3.  Procedure for Porting a new Customer Board (KW)
  4.  MTD (Memory Technology Devices) Support
  5.  Network
    5.1 Network driver
    5.2 Gateway driver 
    5.3 eth-tool
  6.  Phone driver
  7.  SATA
  8.  USB in HOST mode
  9. Real Time Clock
  10. CESA
  11. Kernel configuration
    11.1 General Configuration
    11.2 Run-Time Configuration
    11.3 Compile-Time Configuration 
  12. Debugging Tools
  13. CPUFREQ
  14. CPU-Idle
  15. UBIFS




1.  Default kernel configuration
---------------------------------

	Board			Default Configuration
==========================================================================================
	DB-88F6500-BP		mv88f6500_defconfig
	RD-88F6510-SFU		mv88f6500_defconfig
	RD-88F6550-GW		mv88f6500_defconfig
	RD-88F6530-MDU		mv88f6500_defconfig

2.  Marvell LSP File locations
-------------------------------
    o  core directory: 
       - /arch/arm/mach-feroceon-kw2/...

    o drivers:  
       - /arch/arm/plat-feroceon/...


3.  Procedure for Porting a new Customer Board (relevant only for KW2)
-----------------------------------------------
The following are the steps for porting a new customer board to the Marvell LSP:

    o Add the Board Specific configuration definitions:
	File location:
	~/arch/arm/mach-feroceon-kw2/kw2_family/boardEnv/mvBoardEnvSpec.h

	- MPP pin configuration. Each pin is represented by a nible. Refer the
	  SoC Datasheet for detailed information about the options and values
	  per pin.

	- MPP pin direction (input or output). Each MPP pin is represented
	  with a single bit (1 for input and 0 for output).

	- MPP pin level (default level, high or low) if the MPP pin is a GPIO
	  and configured to output.

	- Specify the Board ID. This is needed to identify the board. This is
	  supposed to be synchronized with the board ID passed by the UBoot.

    o Add the Board Specific configuration tables:
	File location:
	~/arch/arm/mach-feroceon-kw2/kw2_family/boardEnv/mvBoardEnvSpec.c

	The following configuration options are listed in the order they are
	present in the "MV_BOARD_INFO" structure.

	- boardName: Set the board name string. This is displayed by both Uboot and Linux
	  during the boot process.

	- pBoardMppConfigValue (MV_BOARD_MPP_INFO): This structure arranges the MPP pins
	  configuration. This is usually not modified.

	- intsGppMask: Select MPP pins that are supposed to operate as
	  interrupt lines.

	- pDevCsInfo (MV_DEV_CS_INFO):Specify the devices connected on the device bus 
	  with the Chip select configuration.

	- pBoardPciIf (MV_BOARD_PCI_IF): This is the PCI Interface table with the PCI 
	  device number and MPP pins assigned for each of the 4 interrupts A, B, C and D.

	- pBoardTwsiDev (): List of I2C devices connected on the TWSI
	  interface with the device ID Addressing mode (10 or 7 bit).

	- pBoardMacInfo (MV_BOARD_MAC_INFO): Specifies the MAC speed and the Phy address
  	  per Ethernet interface.

	- pBoardGppInfo (): List of MPP pins configured as GPIO pins with special functionality.

	- pLedGppPin (MV_U8): array of the MPP pins connected to LEDs.

	- ledsPolarity: Bitmap specifying the MPP pins to be configured with
	  reverse polarity.

	- gppOutEnVal: This is usually defined in the mvCustomerBoardEnv.h
	  specifying the direction of all MPP pins.

	- gppPolarityVal: Not used.

	Finally update all of the configuration table sizes (xxxxxxxxx_NUM definitions)
  	according to the number of entries in the relevant table.

    o  Specify the memory map of your new board. 
	File location: ~/arch/arm/mach-feroceon-kw2/sysmap.c

	The following configurations should be done:
	- Look for the section in the file related to the SoC device you are using.

	- Add a new table with Address Decoding information (MV_CPU_DEC_WIN) for your board.
	  (Usually existing address decoding tables are compatible with most boards, the 
	  changes might be only in the Device Chip selects only).

	- In the function "mv_sys_map()", add a new "case:" statement (under the appropriate 
	  SoC type) with the your newly added board ID mapping it to the appropriate Address 
	  Decoding configuration table.


4.  MTD (Memory Technology Devices) Support
--------------------------------------------

A new MTD map driver has been added, this driver automatically detects the existing Flash devices
and mapps it into the Linux MTD subsystem. This new driver affect NOR flashes (CFI, SPI and Marvell). 
NAND flashes are supported separately and not not part of this driver.

The detection of MTD devices depends on the Linux kernel configuration options set (using the 
"make menuconfig" or "make xconfig" tools).
To have basic MTD Support the following options should be selected:
	-> Device Drivers                                                                                                   
          -> Memory Technology Devices (MTD)                                                                                
            -> Memory Technology Device (MTD) support (MTD [=y])                                                            

For CFI Flashes the following options should be selected
	-> Device Drivers                                                                                                   
          -> Memory Technology Devices (MTD)                                                                                
            -> Memory Technology Device (MTD) support (MTD [=y])                                                            
              -> RAM/ROM/Flash chip drivers 
		-> Detect flash chips by Common Flash Interface (CFI) probe

For Intel (and Intel compatible) Flashes the following options should be selected
	-> Device Drivers                                                                                                   
          -> Memory Technology Devices (MTD)                                                                                
            -> Memory Technology Device (MTD) support (MTD [=y])                                                            
              -> RAM/ROM/Flash chip drivers 
		-> Support for Intel/Sharp flash chips

For AMD (and AMD compatible) Flashes the following options should be selected
	-> Device Drivers                                                                                                   
          -> Memory Technology Devices (MTD)                                                                                
            -> Memory Technology Device (MTD) support (MTD [=y])                                                            
              -> RAM/ROM/Flash chip drivers 
		->  Support for AMD/Fujitsu flash chips

By default, the map driver maps the whole flash device as single mtd device (/dev/mtd0, /dev/mtd1, ..)
unless differently specified from the UBoot using the partitioning mechanism.
To use the flash partitioning you need to have this option selected in the kernel. To do this
you will need the following option selected:
 	-> Device Drivers                                                                                                   
          -> Memory Technology Devices (MTD)                                                                                
            -> Memory Technology Device (MTD) support (MTD [=y])   
	      -> MTD concatenating support

The exact partitioning is specified from the UBoot arguments passed to the kernel. The following 
is the syntax of the string to be added to the UBoot "booatargs" environment variable:

       'mtdparts=<mtd-id>:7m@0(rootfs),1m@7(uboot)ro' 
       where <mtd-id> can be one of options: 
       1) M-Flash => "marvell_flash"
       2) SPI-Flash => "spi_flash"
       3) NOR-Flash => "cfi_flash"

The latest release of the mtd-utils can be downloaded from http://www.linux-mtd.infradead.org.
(The main page has a link to the latest release of the mtd-utils package).
This package provides a set of sources that can be compiled and used to manage and debug MTD devices. 
These tools can be used to erase, read and write MTD devices and to retrieve some basic information.

The following is a list of useful commands:
To see a list of MTD devices detect by the kernel: "cat /proc/mtd"
To erase the whole MTD device: "./flash_eraseall /dev/mtd0"
To erase the whole MTD device and format it with jffs2: "./flash_eraseall -j /dev/mtd1"
To get device info (sectors size and count): "./flash_info /dev/mtd1"
To create jffs2 image for NAND flash(with eraseblock size 0x20000): 
           ./mkfs.jffs2 -l -e 0x20000 -n -d <path_to_fs> -o <output_file>

for NOR flash only:
===================
To protect all sectors: "./flash_lock /dev/mtd1 0x0 -1"
To unprotect all sectors: "./flash_unlock /dev/mtd1"

  NAND ECC:
  ========
  The Linux support 1 bit SW ECC protection.
  This release include support for 4 bit SW ECC Reed Solomon.
  To enable this support set the relevant config under the feroceon MTD options and configure 
  the U-Boot to use 4 bit ECC by setting the nandEcc env var to 4bit.

5. network
----------  

  5.1 Network driver
---------------------
    The Network driver supports creating Linux netwrok interfaces over
    Marvell SoC GbE ports.

    o Configure whether a GbE port is "connected" to Linux (a Linux network
      interface eth<i> is created) or "disconnected" from Linux (a Linux network
      interface eth<i> is not created) using kernel command line parameters
      which can be appended to the console parameter in U-Boot:
        - mv_port0_config=disconnected
        - mv_port1_config=disconnected
        - mv_port2_config=disconnected
      The default (if no parameter is passed) is that the port is connected.

    o mv_neta_tool user-space application supports runtime configutration of
      the driver supports displaying various statistics.

    o Additional runtime driver configuration and debug is supported via
      sysfs:
        - mount -t sysfs sysfs /sys/
        - cd /sys/devices/platform/neta/
        - cd to desired directory (e.g. switch, gbe...)
        - cat help to display options
 
  5.2 Gateway driver
---------------------
    The Gateway driver supports configuring multiple Linux network interfaces
    over a single GbE port.

    o Gateway driver support depends on the following kernel configuration
      options: 
        - Enable CONFIG_MV_INCLUDE_SWITCH   (enabled by default)
        - Enable CONFIG_MV_ETH_SWITCH       (enabled by default)

	o Each GbE port can be connected to a Switch (internal or on-board Switch).
	  The Gateway driver configuration is determined in boot time by a kernel command line parameter.
	  When GbE#0 is connected to the Switch, this command line parameter is called mv_net_config.
	  When GbE#1 is connected to the Switch, this command line parameter is called mv_net_config1.
	  When both GbE ports are connected to the Switch, both mv_net_config and mv_net_config1 are used.
	  If mv_net_config or mv_net_config1 are not passed from U-Boot as command line parameters, 
	  a default is taken from the compile-time kernel configuration.

    o Select working in Gateway mode / External Switch mode by setting the
      mvNetConfig parameter in U-Boot.
        - setenv mvNetConfig mv_net_config=0    means working in External
          Switch mode (non-Gateway mode). In this mode, Gateway driver
          features are not supported, and the Switch is not initialised by Linux. 
          However, the Switch driver (QD HAL) is loaded to provide a handle for further QD driver API calls.
          Link change prints for Switch ports are also supported.

          Note: the Gateway driver can be used by up to one GbE port connected to the Switch.
		  In a case when more than one GbE port is connected to the Switch, at least one of the 
		  mv_net_config parameters must be configured to work in External Switch mode.

          The rest of this section discusses working in Gateway driver mode.

    o Configure network interface (VLANs) initial setup by setting the
      mvNetConfig parameter in U-Boot:
      Sysntax: setenv mvNetConfig mv_net_config=<max-ifs>,(<mac-addr>,<port-list>)(...),mtu=<mtu-value>
      Example: setenv mvNetConfig mv_net_config=3,(00:aa:bb:cc:dd:ee,0:1)(00:11:22:33:44:55,2),mtu=1500
      means the maximum number of interfaces on the Switch is 3, and currently
      two interfaces are defined - the first includes ports 0 and 1 with MAC address 00:aa:bb:cc:dd:ee, 
      and the second includes port 2 with MAC address 00:11:22:33:44:55. Both
      interfaces share an MTU of 1500.

      Notes: 
        - Linux will create <max-ifs> network devices (in the example, eth0,
          eth1 and eth2) for the Gateway interfaces, even if not all interfaces are
          given a MAC address and port list. 
        - <max-ifs> should be a number between 1 and 4 (0 is used to select
          External Switch mode, see above). The Switch has maximum 4
          "external" ports that can be connected to RJ45 connectors on the
          board.

    o Interface name format is "eth<i>", e.g., eth0, eth1.

    o Runtime network interface (VLANs) configuration is supported via the
      mv_neta_tool commands port_add and port_del.
      Example: in the configuration example above, let's create a new netwrok
      interface, eth2, which includes Switch port 1 only: 
        - ifconfig down eth0
        - mv_neta_tool -port_del eth0 1
        - mv_neta_tool -port_add eth2 1
        - ifconfig up eth0
        - ifconfig eth2 192.168.0.1

      Note: the port parameter is the "logical" port number, between 0 and 3.
      Logical port numbering:
      - The description below refers to the DB board and assumes the RJ45
        connectors are facing to the right.
      - When Switch is connected to the 3xFE PHYs: the connector at the bottom
        is logical port 0, the one above it is logical port 1, and the one
        above that is logical port 2. The top connector of the 4xRJ45
        connector group is not connected to the Switch.
        If Switch Port 4 is connected to the internal GbE PHY, then the
        separate RJ45 connector of the GbE PHY is considered logical port 3.

      Use the mv_neta_tool netdev_sts command to see network device status.

    o Runtime MTU configuration for Gateway interfaces:
        - Initial MTU value is set in U-Boot parameter mvNetConfig (default is
          1500).
        - MTU value must be shared between all Gateway interfaces.
        - MTU value can be changed at runtime for all Gateway interfaces using
          this procedure: 
            - Down all Gateway interfaces using ifconfig eth<i> down.
            - Change MTU for all Gateway interfaces to the same new MTU value
              using ifconfig eth<i> mtu <value>.
            - Up Gateway interfaces as required using ifconfig eth<i> up.

    o Packets between the CPU and the Switch are controlled with Marvell Header.

    o Link status indication is implemented using an ISR.

    o Implementation: 
        - ~/arch/arm/plat-feroceon/mv_drivers_lsp/mv_neta/net_dev/
        - ~/arch/arm/plat-feroceon/mv_drivers_lsp/mv_switch/

  5.3 Ethtool support
  -----------------------
  This release introduces support for a standard ethtool. 
  Please note that for non-raw registers dump command the latest ethtool user space utility with Marvell patches is needed.

  The ethtool support should be enabled in kernel configuration:
   CONFIG_MV_ETH_TOOL:                                                                                                        
   -> System Type
     -> Feroceon SoC options  
       -> SoC Networking support 
         -> Networking Support
            -> Control and Statistics               

  The following ethtool commands are supported in current release:

  - ethtool DEVNAME                               Display standard information about device

  - ethtool -s |--change DEVNAME             Change generic options
            [ speed 10|100|1000 ]
            [ duplex half|full ]
            [ autoneg on|off ]

  - ethtool -c|--show-coalesce DEVNAME     Show coalesce options

  - ethtool -C|--coalesce DEVNAME             Set coalesce options
            [rx-usecs N]
            [tx-usecs N]

  - ethtool -i|--driver DEVNAME                    Show driver information

  - ethtool -d|--register-dump DEVNAME       Do a register dump
            [ raw on|off ]

  - ethtool -r|--negotiate DEVNAME              Restart N-WAY negotation

  - ethtool -p|--identify DEVNAME                Show visible port identification (e.g. blinking)
            [ TIME-IN-SECONDS ]

  - ethtool -S|--statistics DEVNAME             Show adapter statistics

6.  Phone driver
-------------------
	After boot process is completed, mv_phone.ko and phone_test.ko kernel modules are required in order to run
	various voice tests.
	The phone_test.ko module contains the following tests:
	1 - Self echo on `line0_id`
	2 - Loopback between 2 FXS ports(line0_id & line1_id)
	3 - Loopback between FXS and FXO ports(line0_id & line1_id respectively)
	4 - Ring on FXS line `line0_id`
	5 - Generate SW tones(300Hz, 630Hz, 1000Hz) on FXS line `line0_id`
	6 - Multi FXS loopback test between several pairs of lines(line0_id=<start_line> and line1_id=<end_line>)
	7 - SLIC register read

	For example, to run loopback test between 2 phones(using 2xFXS module), run the following commands:
	 > insmod mv_phone.ko phone_config=dev[0-1]:fxs
	 > insmod phone_test.ko line0_id=0 line1_id=1 test_id=2

	In order to run different test, unload both modules using the standard Linux `rmmod` command and reload them again with
	the requested `test_id` parameter.


7. SATA 
---------
	TBD


8.  USB in HOST mode
---------------------

The mode of the USB controller (device or host) is configured using the UBoot environment variables. 
To work in USB HOST mode, set the UBoot variable "usb0Mode"/"usb1Mode" to "host".
The USB driver uses the standart Linux ehci driver.


9.  Real Time Clock
---------------------

  The driver is found under ~/arch/arm/mach-feroceon-xx/rtc.c
To read the date and time from the integrated RTC unit, use the command "hwclock".
To set the time in the RTC from the current Linux clock, use the command "hwclock --systohc"


10.  CESA
----------
OpenSSL
-------
  see cesa/openssl/

IPsec
-----
  see cesa/openswan/


11. Kernel configuartion
-------------------------

 11.1 General Configuration:
 ---------------------------
- This release has support for sending requests with length up to 1MB for the
  SATA drives, in some cases, this feature can reduce the system performance,
  for example, running Samba and a client that performs sequential reads.
  Note that the user can modify the limit of the max request using the sysfs,
  this parameter is per block device, and it's defined by special file called
  'max_sectors_kb' under the queue directory of the block device under the sysfs.
  for example, the /sys/block/sda/queue/max_sectors_kb is for the /dev/sda
  device.

- In order to use block devices that are larget then 2TB, CONFIG_LBD should be enabled.
  fdisk doesn't support block devices that are larger then 2TB, instead 'parted' should be used.
  The msdos partition table doesn't support >2TB , you need GPT support by the kernel:
  File Systems
    Partition Types
      [*] Advanced partition selection
      [*] EFI GUID Partition support

 11.2 Run-Time Configuration:
 ----------------------------
  The following features can be configured during run-time:
    o  NFP mechanism:
  	 echo D > /proc/net/mv_eth_tool (disable NFP)
  	 echo E > /proc/net/mv_eth_tool (enable NFP)
    o TX enable race:
         mv_eth_tool -txen <port> 0/1 (0 - disable, 1 - enable)         
    o SKB reuse mechanism:
         mv_eth_tool -skb 0/1 (0 - disable, 1 - enable)
    o LRO support:
	 mv_eth_tool -lro <port> 0/1 (0 - disable, 1 - enable)
 
  * for more ethernet run-time configurations, see egigatool help.
  
 11.3 Compile-Time Configuration:
 --------------------------------
 The following features can be configured during compile-time only:   
    o L2 cache support
    o XOR offload for CPU tasks:
       - memcpy
       - copy from/to user
       - RAID5 XOR calculation
    o TSO
    o Multi Q support - for mv_gateway driver only.
    o CESA test tool support.   


12.  Debugging  Tools
----------------------

    o  Runtime debugging is supported through the /proc virtual FS.
       See ~/arch/arm/mach-feroceon-xx/proc.c

    o  mv_shell: Access memory, SoC registers, and SMI registers from user space.
       mv_eth_tool: Probe mv_ethernet driver for statistic counters.
       mv_cesa_tool: Probe CESA driver for statistic counters.
       These tools are found under ~/tools

    o The LSP supports kernel debugging using KGDB. Refer to AN232 "Using GDB to Debug the 
      Linux Kernel and Applications" for detailed information.

    o Early boot debugging is supported by the LSP. To enable this option configure the following
      settings in the kernel.
	-> Kernel hacking
	  -> Kernel low-level debugging functions
      You have this option you need first to enable the "Kernel debugging" tab first.
  
      
13. CPUFREQ
-----------

 13.1 Introduction:
 ------------------
   The cpufreq driver allows the cpu frequency to be adjusted either manually from userspace or
   automatically according to given policies. The available policies are defined when the kernel
   is configured. In order to get/set cpu frequency parameters the cpufreq utils are used.

 13.2 cpufreq kernel driver:
 -------------------------
   The driver implements power save on/off according to the desired cpu frequency.      

 13.3 kernel configuration:
 --------------------------
	- Enable Cpu Frequency scaling and choose userspace governor as default governor
	- Choose Default CPUFreq governor: userspace

	CONFIG_CPU_FREQ=y
	CONFIG_CPU_FREQ_TABLE=y
	# CONFIG_CPU_FREQ_DEBUG is not set
	CONFIG_CPU_FREQ_STAT=y
	# CONFIG_CPU_FREQ_STAT_DETAILS is not set	
	# CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE is not set
	CONFIG_CPU_FREQ_DEFAULT_GOV_USERSPACE=y
	CONFIG_CPU_FREQ_GOV_PERFORMANCE=y
	# CONFIG_CPU_FREQ_GOV_POWERSAVE is not set
	CONFIG_CPU_FREQ_GOV_USERSPACE=y
	# CONFIG_CPU_FREQ_GOV_ONDEMAND is not set
	# CONFIG_CPU_FREQ_GOV_CONSERVATIVE is not set
	CONFIG_CPU_FREQ_FEROCEON_KW2=y


 13.4 cpufrequtils installation:
 -------------------------------
   On a debian system it suffices to say:
   apt-get install cpufrequtils
	

 13.5 Using the cpufreq utilties:
 --------------------------------
   -Display information:
	% cpufreq-info -h

	cpufrequtils 004: cpufreq-info (C) Dominik Brodowski 2004-2006	
	Report errors and bugs to cpufreq@lists.linux.org.uk, please.
	Usage: cpufreq-info [options]
	Options:
	  -c CPU, --cpu CPU    CPU number which information shall be determined about
	  -e, --debug          Prints out debug information
	  -f, --freq           Get frequency the CPU currently runs at, according
	                       to the cpufreq core *
	  -w, --hwfreq         Get frequency the CPU currently runs at, by reading
	                       it from hardware (only available to root) *
	  -l, --hwlimits       Determine the minimum and maximum CPU frequency allowed *
	  -d, --driver         Determines the used cpufreq kernel driver *
	  -p, --policy         Gets the currently used cpufreq policy *
	  -g, --governors      Determines available cpufreq governors *
	  -a, --affected-cpus  Determines which CPUs can only switch frequency at the
	                       same time *
	  -s, --stats          Shows cpufreq statistics if available
	  -o, --proc           Prints out information like provided by the /proc/cpufreq
	                       interface in 2.4. and early 2.6. kernels
	  -m, --human          human-readable output for the -f, -w and -s parameters
	  -h, --help           Prints out this screen

	If no argument or only the -c, --cpu parameter is given, debug output about
	cpufreq is printed which is useful e.g. for reporting bugs.
	For the arguments marked with *, omitting the -c or --cpu argument is
	equivalent to setting it to zero


	Example Usage:
	% cpufreq-info
	cpufrequtils 004: cpufreq-info (C) Dominik Brodowski 2004-2006
	Report errors and bugs to cpufreq@lists.linux.org.uk, please.
	analyzing CPU 0:
	  driver: kw_cpufreq
	  CPUs which need to switch frequency at the same time: 0
	  hardware limits: 400 MHz - 1.20 GHz
	  available frequency steps: 400 MHz, 1.20 GHz
	  available cpufreq governors: userspace, performance
	  current policy: frequency should be within 400 MHz and 1.20 GHz.
	                  The governor "userspace" may decide which speed to use
	                  within this range.
	  current CPU frequency is 1.20 GHz (asserted by call to hardware).
	  cpufreq stats: 400 MHz:0.00%, 1.20 GHz:0.00%  (6)
 	


   - Setting new cpu frequency:
	% cpufreq-set -h

	cpufrequtils 004: cpufreq-set (C) Dominik Brodowski 2004-2006
	Report errors and bugs to cpufreq@lists.linux.org.uk, please.
	Usage: cpufreq-set [options]
	Options:	
	  -c CPU, --cpu CPU        number of CPU where cpufreq settings shall be modified
	  -d FREQ, --min FREQ      new minimum CPU frequency the governor may select
	  -u FREQ, --max FREQ      new maximum CPU frequency the governor may select
	  -g GOV, --governor GOV   new cpufreq governor
	  -f FREQ, --freq FREQ     specific frequency to be set. Requires userspace
	                           governor to be available and loaded
	  -h, --help           Prints out this screen

	Notes:
	1. Omitting the -c or --cpu argument is equivalent to setting it to zero
	2. The -f FREQ, --freq FREQ parameter cannot be combined with any other parameter
	   except the -c CPU, --cpu CPU parameter
	3. FREQuencies can be passed in Hz, kHz (default), MHz, GHz, or THz
	   by postfixing the value with the wanted unit name, without any space
	   (FREQuency in kHz =^ Hz * 0.001 =^ MHz * 1000 =^ GHz * 1000000).


	Example usage:

	% cpufreq-set -f 1.2GHz
	% cpufreq-set -f 400MHz
	


 13.6 Dynamic Frequency Scaling
 ------------------------------
   It is possible to let a background daemon (e.g. hald-addon-cpufreq) decide how to
   scale the cpu frequency according to the system load. For this purpose proceed as follows:
	- Configure kernel with "ondemand" governor.
	- Set the default governor to be "userspace".
	- The governor can also be set from userspace by "cpufreq-set -g"

	- The behaviour of the hald-addon-cpufreq daemon can be configured through sysfs at
	% ls /sys/devices/system/cpu/cpu0/cpufreq/ondemand/
	  ignore_nice_load  sampling_rate      sampling_rate_min
           powersave_bias    sampling_rate_max  up_threshold		

 13.7 Controlling CPU Freq through procfs
 ----------------------------------------
   Automatic CPU frequency scaling can be disabled / enabled using procfs:
   echo disable > /proc/mv_pm/cpu_freq
   echo enable > /proc/mv_pm/cpu_freq

   While automatic CPU frequency scaling is disabled, it's possible to
   change the CPU frequency to fast / slow mode:
   echo fast > /proc/mv_pm/cpu_freq
   echo slow > /proc/mv_pm/cpu_freq

   Changing the CPU frequency manually will not update the BogoMIPS (in
   /proc/cpuinfo).

14. CPU-Idle
------------
   CPU Idle framework enables the kernel to select the idle mode in
   which the system will be running.
   In order to enable CPU-Idle framework select the following in the
   kernel config:
	CONFIG_CPU_IDLE=y

   It's possible to enable / disable CPU-Idle framework from userspace
   using the following commands:
     - echo disable > /proc/mv_pm/cpu_idle
     - echo enable > /proc/mv_pm/cpu_idle

15. UBIFS
----------

  14.1 Getting the sources:
  -------------------------
	- mtd utils with ubifs support: git://git.infradead.org/mtd-utils.git
	
  14.2 Compiling mtd utils:
  -------------------------

	The mtd utils have to be compiled both for arm and for x86 since typically the
	file system will be generated on a x86 system.
	The lzo and uuid libraries are needed for compiling and for running the ubi utilities.

  14.3 UBI utilities on the target system:
  ----------------------------------------

	The following utilities should be on the target system:
	- ubiformat, ubinfo, ubimkvol, ubirmvol, ubiupdatevol, ubiattach, ubidetach


  14.4 Building a ubifs root file system:
  ---------------------------------------

	- In the following I assume that the root file system is situated in the directory rootfs.
 	  Several of the parameters 
  	  appearing in the following example need to be adjusted to the user's needs and only serve as examples.

	- Create a configuration file ubinize.cfg with the following contents:

	[ubifs]
	mode=ubi
	image=tmp_rootfs.img
	vol_id=0
	vol_size=90MiB
	vol_type=dynamic
	vol_name=rootfs
	vol_flags=autoresize	


	- Execute the following commands:

  	% mkfs.ubifs -g 2 -v -r rootfs -m 2KiB -e 124KiB -c 2047 -o tmp_rootfs.img

  	% ubinize -v -o rootfs_ubi.img -m 2KiB -p 128KiB -s 2048 -O 2048 ubinize.cfg
  
  	% rm tmp_rootfs.img


	- The file rootfs_ubi.img contains the ubifs image of the root file system.


  14.5 Burning ubifs image to flash:
  ----------------------------------

	Assume we would like to burn an image with the name /tmp/rootfs_ubi.img to the
 	mtd partition mtd2. For this purpose execute the following commands on the target system:

	% ubiformat /dev/mtd2 -s 2048 -O 2048 -f /tmp/rootfs_ubi.img


  14.6 Booting a ubifs root file system:
  --------------------------------------

	Assume that mtd2 contains the root file system in the ubivolume named rootfs. In this case
 	the following parameters have to be added to the bootargs:

	ubi.mtd=2,2048 root=ubi0:rootfs rootfstype=ubifs


 14.7 Creating/mounting ubi partitions at run time:
 --------------------------------------------------

	Assume that we want to create a ubi file system with a size of 32MB on mtd2.
	To this purpose execute the following commands:

	% ubiformat /dev/mtd2 -s 2048

	% ubiattach /dev/ubi_ctrl -m 2 -O 2048

	% ubimkvol /dev/ubi0 -N some_name -s 32MiB

	% mkdir -p /mnt/some_name

	% mount -t ubifs ubi0:some_name /mnt/some_name

