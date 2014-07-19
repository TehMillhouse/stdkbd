stdkbd
======

Creates a uinput device, reads characters from stdin and injects keyboard events for these characters.

Note: currently there's only support for numerical keys, but adding support for more is trivial.

You'll need to have the uinput module loaded (`modprobe uinput`) and the right permissions for creating uinput devices.
