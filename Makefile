all: libnss_loopback.so.2 nsstest.so

CFLAGS += -Wall -Wextra

libnss_loopback.so.2: nss_loopback.c
	$(CC) -o $@ -fPIC -shared $<

nsstest.so: nsstest.c
	$(CC) -o $@ -fPIC -shared $<

.PHONY: all
