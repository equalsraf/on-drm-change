
on-drm-change: on-drm-change.c
	$(CC) $(CFLAGS) -std=c99 -ludev -o $@ $<
