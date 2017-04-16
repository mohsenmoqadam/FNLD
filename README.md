FNLD
=====
FNLD is `Nokia 5110 LCD` driver for FreeBSD OS. `Nokia 5110 LCD` is low-cost, low power, and stable GLCD. 
At the moment many libraries developed based on C, C++, and Python. These libraries suitable for use with Arduino and Linux-based OS
like Raspbian. 
If your FreeBSD boots on `Raspberry Pi 2`, FNLD help you for sending and showing strings on `Nokia 5110 LCD`. 
In the following, I first present you how to connect `Nokia 5110 LCD` to your `Raspberry Pi 2` and next illustrate how to prepare
FreeBSD and use this driver.

1-Hardware Connections
-----
You can connect `Nokia 5110 LCD` to `Raspberry Pi 2` SPI bus and use one of its chip select pins: CE0 or CE1. As shown in the below figure, I use CE0. This connection required when we upgrade FDT.
```
        +----------------------+                                            +-----------+
	|                      |36: GPIO.16                           1: RST|           |
	|                      +--------------------------------------------+           |
	|                      |24: GPIO.6 (SPI.CE0)                  2: CE |           |
	|                      +--------------------------------------------+           |
	|                      |37: GPIO.20                           3: DC |           |
	|                      +--------------------------------------------+           |
	|                      |19: GPIO.10 (SPI.MOSI)                4: DIN|           |
	|                      +--------------------------------------------+   Nokia   |
	|    Raspberry Pi 2    |23: GPIO.11 (SPI.SCLK)                5: CLK|  LCD 5150 |
	|                      +--------------------------------------------+           |
	|                      |2:  5V                                6: VCC|           |
	|                      +--------------------------------------------+           |
	|                      |40: GPIO.21                           7: BL |           |
	|                      +--------------------------------------------+           |
	|                      |39: GND                               8: GND|           |
	|                      +--------------------------------------------+           |
	|                      |                                            +-----------+
	+----------------------+
```
2-Upgrade FDT
-----
Open `/usr/src/sys/boot/fdt/dts/arm/rpi2.dts` in your favorite editor (I use `ee`):	

	%ee /usr/src/sys/boot/fdt/dts/arm/rpi2.dts
	
And add on `axi` section after `gpio` definitions following codes:
		
	spi0 {
		nl5110 {
			compatible = "nokia,lcd5110";
			spi-chipselect = <0>;
		};
	};
		
Now we must build DTB file. DTB is the binary presentation of FDT and OS understand it. 
For this change your current directory and go to `/usr/src/sys/tools/fdt/`:
		
	%cd /usr/src/sys/tools/fdt/
		
Create an environment variable for declaring machine architecture:
		
	%setenv MACHINE arm
		
And run `make_dtb.sh` script, like bellow:
		
	%./make_dtb.sh /usr/src/sys /usr/src/sys/boot/fdt/dts/arm/rpi2.dts .
		
New DTB file created at the current location and you must move it to `/boot/msdos/` in your MMC:	
		
	%cp rpi2.dtb /boot/msdos/
		
And finally reboot the OS:
		
	%reboot
		

3-Download and Compile
-----
Use `git` command for downloading FNLD:
		
	%git clone https://github.com/mohsenmoqadam/FNLD
		
Then go to FNLD directory:
		
	%cd FNLD
		
And compile it:
		
	%make	

4-Load and use
-----
For load FNLD’s KLD we use `kldload` command:
		
	%kldload lcd5110.ko
		
You can use `echo` command for sending a string to `Nokia 5110 LCD`:
		
	%echo "Hello World!" > /dev/lcd5110

![lcd5110](https://github.com/mohsenmoqadam/FNLD/lcd5110.jpg "Hello World!")
	
5-Notes
-----
If you don't have FreeBSD source tree, first download and insert it at `/usr/src`. for this you can use `svn` command like this:
	
	%svn co svn://svn.freebsd.org/base/stable/11 /usr/src

