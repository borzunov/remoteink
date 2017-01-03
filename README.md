RemoteInk
=========

A tool for using PocketBook Pro/Touch E-Ink reader as a computer monitor on Linux

Purposes
--------

This application is designed for people who like to use E-Ink displays. It's suitable for tasks where color monitor and high reaction time aren't necessary: reading and writing texts, programming, etc.

<img src="http://habrastorage.org/files/ed8/5de/963/ed85de963ac44b7e930719ab3b54c3a5.jpg" width=650 />

*__Photo:__ Using Geany on Linux Mint with RemoteInk*

### Features

You can:
* Connect a reader to a computer using Wi-Fi or USB
* Configure connection parameters in UI on the reader
* Change the reader's screen orientation (portrait or landscape)
* Select which part of the monitor is visible or zoom it
* Track windows (active window is always visible) and save its own scale for each of them
* Easily switch between windows
* Adjust active window's size to dimensions of the reader's screen
* See a cursor and restrict its movements to the area visible on the reader
* Invert screen colors
* Customize keyboard shortcuts for most of the actions above
* Set a password for a connection
* Use the application with a typical computer (with *i686/amd64* architecture) or Raspberry Pi

### Compatibility

A client application is compatible with readers from *Pocketbook Pro/Touch* series (most of modern *Pocketbook* models with E-Ink screen). Testing was performed on *Pocketbook Touch* and *Touch 2*.

A server is compatible only with computers with *Linux* and *X11* window system.

### Warning about possible damage

Note that this program is experimental. Readers' screens are not intended to update so frequently, so using the program can lead to malfunction of the screen or reduce its lifetime. The developer of the program is not responsible for possible damage to your device.

Installation
------------

### Server

1. Install required dependencies:

	* *Debian* and its derivatives (*Ubuntu*, *Linux Mint*, etc.):

			$ sudo apt-get install git build-essential libfontconfig1-dev libimlib2-dev libxcb1-dev libx11-dev libxcb-keysyms1-dev libxcb-shm0-dev libxcb-xfixes0-dev fonts-freefont-ttf

	* *Fedora*:

			$ sudo dnf install git gcc make imlib2-devel fontconfig-devel libxcb-devel libX11-devel xcb-util-keysyms-devel gnu-free-sans-fonts

	* *Arch Linux*:

			$ sudo pacman -S git base-devel fontconfig imlib2 libxcb libx11 xcb-util-keysyms ttf-freefont

	If neither of these commands suits you, you can try to find similar packages in repositories of your system.

2. Clone the repository and build the daemon:

		$ git clone https://github.com/borzunov/remoteink.git
		$ cd remoteink/server
		$ make

3. Install binaries:

		$ sudo make install

	Note that uninstall command is also available:

		$ sudo make uninstall

4. Check shortcuts scheme. Default scheme is designed for keyboards with numpad (see [full description](#default-shortcuts-scheme)). If you don't have numpad or this scheme will be uncomfortable for you, you can change it as described [here](#configuration).

	Note that these shortcuts must not overlap with other system-wide shortcuts.

5. Set up a password to the server:
	
		$ sudo remoteinkd passwd

### Client

Extract an executable file from [this archive](https://github.com/borzunov/remoteink/releases/download/v0.2/remoteink.app.zip) (it's for *v0.2*) and place it into `applications/` folder in your reader's memory (the folder may be hidden on some devices).

It is not required, but if you want to build the client by yourself, you can install Pocketbook Pro SDK and compile the client using the following commands:

	$ cd remoteink/client
	$ make BUILD=arm_gnueabi

Then executable `obj_arm_gnueabi/remoteink.app` will be created.

How to use
----------

### Connection via Wi-Fi

1. Set up a wireless network, connect to it on the computer and find out its IP address. If you know a corresponding network interface (`wlan0` in this example), it can be done using the following command:

	<pre>
	$ ifconfig
	<i>[...]</i>
	<b>wlan0</b>     Link encap:Ethernet  HWaddr aa:bb:cc:dd:ee:ff  
			  inet addr:<b>192.168.0.101</b>  Bcast:192.168.0.255  Mask:255.255.255.0
			  inet6 addr: xxxx::xxx:xxxx:xxxx:xxxx/64 Scope:Link
			  UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
	<i>[...]</i>
	</pre>

2. Run the server using command:

		$ sudo remoteinkd start

	You can check that the server is started using command:

	<pre>
	$ sudo remoteinkd status
	RemoteInk v0.2 - Server
	[*] remoteinkd <b>is running</b> (PID 1234)
	</pre>

	If command doesn't show that server is running, go to [Troubleshooting](#troubleshooting) section.

3. Open *remoteink* application on the reader. Use an interface (it supports control via touchscreen or physical keys) to enter server's IP and password and press "Connect" button. Select your network if you weren't connected to it yet.

	If you don't see monitor contents, go to [Troubleshooting](#troubleshooting).
	
	<img src="http://habrastorage.org/files/f82/319/cf1/f82319cf16e34813b15e2f461d7e47f0.jpg" height=450 />

4. After work you can stop the daemon:

		$ sudo remoteinkd stop
		
	Or kick the current client (it's also possible to disconnect using "Back" reader's key):
	
		$ sudo remoteinkd kick

### Connection via USB

Working via USB is implemented using `g_ether.ko` Linux kernel module. It is supported by most of models of the compatible readers and must present on the server-side.

1. On the reader open menu *Settings* -> *Connectivity* -> *USB Mode* and select *Network over USB*.

	<img src="http://habrastorage.org/files/12e/3c2/1ee/12e3c21ee6fd45c5a25f00fb847db858.jpg" height=450 />

2. Connect the reader via a USB cable. If the described kernel module is present, a new network interface will appear. Then you can find out its IP address and follow instructions for a Wi-Fi network above.

Advice
------

* Try to disable cursor blinking when editing texts. It may reduce the reader's screen updates count.

Default shortcuts scheme
------------------------

A reader's screen is smaller than a computer's monitor, so we can see only part of the monitor on the reader (let's call it "*the frame*"). Shortcuts are introduced to comfortably move the frame during work with windows on the computer. This section describes purposes of each shortcut and keys corresponding to it in default scheme.

<img src="https://upload.wikimedia.org/wikipedia/commons/thumb/9/99/Numpad.svg/333px-Numpad.svg.png" height=350 />

There are two modes in the application: when window tracking is enabled and when not (default mode can be changed in a config file). If window tracking is enabled, switching between windows causes moving the frame in such a way that active window stays in the center of the reader's screen. If the window doesn't fit into the screen, initially only its left top part is visible. Nevertheless, you can move the frame relative to the active window.

`Ctrl`+`Alt`+`Num 0` &ndash; Toggle window tracking

`Alt`+`Tab` &ndash; Switch active window (DE's shortcut)

`Ctrl`+`Alt`+`D` &ndash; Show desktop (DE's shortcut)

`Ctrl`+`Alt`+`Num 2`/`4`/`6`/`8` &ndash; Move the frame

`Ctrl`+`Alt`+`Num 5` &ndash; Reset frame relative position to default

Also, you can zoom the visible image. Scale is stored separately for each window and different modes (default scale for each mode can be changed in the config). Default scale for a mode with disabled window tracking is so small that you can see the entire desktop. So this mode can be used if you want to switch between windows using mouse, move windows, look on desktop panels or the system tray.

`Ctrl`+`Alt`+`Num 3`/`9` &ndash; Zoom

`Ctrl`+`Alt`+`Num 1` &ndash; Reset scale to default for this mode

If an opened window doesn't fit into the reader's screen, you can adjust window size as necessary.

`Ctrl`+`Alt`+`Num 7` &ndash; Adjust window size (if possible)

Some additional opportunities are available. Generally usage of a mouse via the slow reader's screen is inconvinient, but there is a cursor capturing mode when the cursor becomes visible and its movement is bounded in the captured frame.

`Ctrl`+`Alt`+`Num Del` &ndash; Toggle cursor capturing

Also, there's an opportinity to invert screen colors, but a shorcut for it is disabled by default.

Configuration
-------------

Different settings (such as a shortcuts scheme, server's port or maximal FPS value) can be changed in `/etc/remoteinkd/config.ini`. The config has comments that help you to understand purpose and format of each option.

The password's hash is stored in `/etc/remoteinkd/passwd` and can be changed using command:

	$ sudo remoteinkd passwd

Note that you should restart the daemon so as to it uses new configuration and password.

Troubleshooting
---------------

### Check whether connection is not blocked

A reader's connection can be blocked by a firewall. You should add a rule that allows connections to *remoteinkd* (default port is *9312/tcp*) or temporarily disable the firewall.

### Look up error messages

Error messages can be displayed during daemon starting and controlling (don't forget to use `sudo` during these operations). Some connection errors can be displayed in the client-side.

Also, at the time of the daemon is running it uses *syslog* to log information and error messages. You can check the log file to find out a reason of occured errors, for example:

<pre>
$ grep remoteinkd /var/log/syslog | tail
<i>[...]</i>
Feb 13 19:54:22 hostname <b>remoteinkd: Unknown modifier in shortcut "Ctrk+Alt+KP_8"</b>
</pre>

In this case the daemon informs that one of shortcut's modifiers is incorrect. You should check the correctness of defined shortcuts in the config (actually, there's a typo in a modifier name - *Ctrk* instead of *Ctrl*).

### If monitor's color depth is unsupported

Only 24-bit color depth is supported yet. You can reconfigure your distribution to use this depth. For example, in *Raspbian* running on Raspberry Pi you can add the following lines to `/boot/config.txt` (and restart the system):

	framebuffer_depth=32
	framebuffer_ignore_alpha=1

### If shortcuts don't work well

Shortcuts are tested only in classic window managers yet. Disabling tiling mode or using more common WM may be helpful.

### If the daemon's CPU usage is too much

It is possible on machines with weak processors (including Raspberry Pi). Try to halve value of `MaxFPS` parameter in the config.

Alternatives
------------

* You can prefer a VNC client for Pocketbook created by *othb08me09zp* (and slightly modified by me). It uses the well-known cross-platform protocol, but may be less convenient for managing windows during daily work.

	[Download](https://goo.gl/iBnDGZ) | [Discussion](http://www.the-ebook.org/forum/viewtopic.php?t=21814) (in Russian)

What else can be implemented?
-----------------------------

* Scrolling or pressing mouse keys using a reader's touchscreen
* The client and the server can be ported to other platforms

What can be improved in the current implementation?
---------------------------------------------------

* Use `xcb-damage` (it can reduce CPU usage)
* Release resources and use sockets more carefully
* Support different screen depths (only 24-bit depth is supported yet)

Author
------

Copyright &copy; 2013-2017 Alexander Borzunov
