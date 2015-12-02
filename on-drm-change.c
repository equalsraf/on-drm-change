///
/// on-drm-change cmd1 cmd2 cmd3
///
/// Call commands when drm changes ocurr, e.g.
///
///    on-drm-change "xrandr --auto"
///
/// **Each** command line argument is treated as
/// a different command, executed in order.
///
#include <stdio.h>
#include <libudev.h>
#include <sys/select.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	struct udev *udev = udev_new();
	if (udev == NULL) {
		fprintf(stderr, "Unable to initialize udev context");
		return -1;
	}

#ifdef DEBUG
	struct udev_enumerate *enumerate = udev_enumerate_new(udev);
	if (enumerate == NULL) {
		fprintf(stderr, "Unable to enumerate devices");
		return -1;
	}

	udev_enumerate_add_match_subsystem(enumerate, "drm");
	udev_enumerate_scan_devices(enumerate);
	struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
	if (devices == NULL) {
		fprintf(stderr, "Unable to enumerate devices");
		return -1;
	}

	// List devices
	struct udev_list_entry *dev_entry;
	udev_list_entry_foreach(dev_entry, devices) {
		const char *path = udev_list_entry_get_name(dev_entry);
		struct udev_device *dev = udev_device_new_from_syspath(udev, path);
		printf("%s [devtype=%s]\n", path, udev_device_get_devtype(dev));
	}
	udev_enumerate_unref(enumerate);
#endif

	// Setup monitor
	struct udev_monitor *monitor = udev_monitor_new_from_netlink(udev, "udev");
	if (monitor == NULL) {
		fprintf(stderr, "Unable to initialize udev monitor");
		return -1;
	}

	if (udev_monitor_filter_add_match_subsystem_devtype(monitor, "drm", "drm_minor") < 0) {
		fprintf(stderr, "Unable to setup udev monitor");
		return -1;
	}
	if (udev_monitor_filter_update(monitor) < 0) {
		fprintf(stderr, "Unable to setup udev monitor");
		return -1;
	}

	if (udev_monitor_enable_receiving(monitor) < 0) {
		fprintf(stderr, "Unable to listen in udev monitor");
		return -1;
	}

	// The same issue as in SO, receive_device does not block, use select
	// http://stackoverflow.com/questions/15687784/libudev-monitoring-returns-null-pointer-on-raspbian
	int fd = udev_monitor_get_fd(monitor);
	while(1) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		int ret = select(fd+1, &fds, NULL, NULL, NULL);
		if (ret > 0 && FD_ISSET(fd, &fds)) {
			struct udev_device *dev = udev_monitor_receive_device(monitor);
			if (dev == NULL) {
				continue;
			}

			printf("%s\n", udev_device_get_syspath(dev));
			for (int i=1; i<argc; i++) {
				system(argv[i]);
			}
			udev_device_unref(dev);
		}
	}

	udev_monitor_unref(monitor);
	udev_unref(udev);
	return 0;
}
