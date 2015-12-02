
Listen to udev events for DRM devices and call a script - this is useful to detect when
you plug/unplug an external monitor, e.g. I use it to adjust xrandr settings.

The only requirement is libudev. I've included an example script for xrandr, just call

    on-drm-change "./screen-profile"

This was only tested using an Intel graphics card, no idea if udev relays events for
other cards, a quick way to find out is to execute 

    udevadm monitor

and try disconnecting your monitor.
