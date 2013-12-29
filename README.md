VERSION
	1.0

NAME
	xkbdrelay - relay one X display's keyboard and mouse events
        to another X display

SYNTAX
	xkbdrelay <SENDER_DISPLAY> <RECEIVER_DISPLAY> [options]

SYNOPSIS

	This program will allow you to send keyboard and mouse events
	from a sender's X display into a receiver's X display.  It is useful
	if you don't have a keyboard/mouse hooked up to a machine
	on your network but want to control that machine from another
	machine that does.

	The program opens up a window on the sender's X display. 
	As long as that window has focus, it will relay all keystrokes and
	mouse movements to the specified receiver's X display.  Press the 
	'Grab' button to grab the mouse so the pointer will not leave the
        bounds of the window (does not work in cygwin).  Press Cntrl-Alt
	to release the mouse.  

	NOTE: X11R6 or above is required as the XTEST extension is used.

RUNNING 

	When running xkbdrelay on your RECEIVER, make sure the X Server 
	specified by SENDER_DISPLAY will allow remote connections 
	(using xhost +), is listening on port 6000 and port 6000 is
	accessible by RECEIVER.

	When running xkbdrelay on your SENDER, make sure the X Server 
	specified by RECEIVER_DISPLAY will allow remote connections 
	(using xhost +), is listening on port 6000 and port 6000 is
	accessible by SENDER.

	Example:

	./xkbdrelay computer_with_keyboard:0.0 no_keyboard_here:0.0

        NOTE:  The local ip address or loopback address (127.0.0.1) of
               the machine you are running xkbdrelay may not work if 
               that X server is not listening for incoming
               connections on port 6000.  In this case, use :0.0 instead.

	Example:

	./xkbdrelay :0.0 no_keyboard_here:0.0 (running on sender)

	./xkbdrelay computer_with_keyboard:0.0 :0.0 (running on receiver)

	A window should appear on your sender's display.  Make sure the window 
	has focus.  All keystrokes and mouse movements will be sent from the 
	sender to the receiver display.  Press 'Grab' button to keep 
	the mouse from moving outside the window.  Press Cntrl-Alt to release 
	the mouse.

OPTIONS

	-grab		grab the mouse immediately when the program is run

	-geometry <s>	standard X geometry string

TROUBLE SHOOTING

        You may have to enable incoming TCP connections on one of your
	X servers (depending on which one is going to receive the incoming
	connection).  By default, most systems disable this and re-enabling
	it depends on which window manager you are using.  Here are
	some common ways of enabling incoming TCP connections:

	Edit the file: /etc/X11/xinit/xserverrc
		Remove the '-nolisten tcp' and reboot

	Edit the file: /etc/gdm/gdm.schemas
		Change DisallowTCP from true to false and reboot

	Edit the file: /etc/lightdm/lightdm.conf
		Add xserver-allow-tcp=true and reboot

	If you receive DISPLAY connection errors, make sure you are
	able to export your display to a remote X server.  Test this as
	follows:

	export DISPLAY=remote_machine:0.0
	xclock

	If you get a connection error, you most likely have to disable
	the server access control restrictions on the remote machine:

	xhost + <hostname>
	or
	xhost + (to enable any client to connect)

	see xhost for more information on server access control.

ARGUMENTS

	SENDER_DISPLAY
		The DISPLAY string indicating where to create a keystroke
		grabbing window.  It is of the form host:screen.  Use :0.0
		if you are running xkbdrelay on the sender machine.

	RECEIVER_DISPLAY
		The DISPLAY string indicating where to send the keystrokes
		that are being grabbed from the grabbing window.  Use :0.0
		if you are running xkbdrelay on the receiver machine.

AUTHORS
	Randy Rossi - 2007
	randy.rossi@gmail.com

