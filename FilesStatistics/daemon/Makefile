INSTALL_DIR=/usr/local
CFLAGS=-g
file_mon: main.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm file_mon


install: file_mon
	cp file_mon $(INSTALL_DIR)/bin/file_mon
	cp file_mon.conf $(INSTALL_DIR)/etc/file_mon.conf
	mkdir -p /var/lib/file_mon/

uninstall:
	rm -f $(ISTALL_DIR)/bin/file_mon
	rm -rf /var/lib/file_mon
