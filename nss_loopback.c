#define _GNU_SOURCE
#include <errno.h>
#include <netdb.h>
#include <nss.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <xlocale.h>

#define DOMAIN "lo.ldpreload.com"

static int
loopback_match(const char *name)
{
	static locale_t c_locale = NULL;
	size_t len;
	const char *suffix, *numloc, *endptr;
	int ret;

	if (c_locale == NULL) {
		c_locale = newlocale(LC_ALL_MASK, "C", NULL);
	}
	if (c_locale == NULL) {
		return -1;
	}

	len = strlen(name);
	if (len <= strlen("." DOMAIN)) {
		return -1;
	}
	suffix = name + len - strlen("." DOMAIN);
	if (strcmp(suffix, "." DOMAIN) != 0) {
		return -1;
	}
	numloc = suffix - 1;
	while ((numloc > name) && isdigit_l(*(numloc - 1), c_locale)) {
		numloc--;
	}

	if ((numloc >= suffix) || (suffix - numloc > 3)) {
		return -1;
	}

	ret = strtol_l(numloc, &endptr, 10, c_locale);
	if (ret < 255 && endptr == suffix) {
		return ret;
	} else {
		return -1;
	}
}

enum nss_status
_nss_loopback_gethostbyname_r(const char *name, struct hostent *result,
                              char *buffer, size_t buflen, int *errnop,
                              int *herrnop)
{
	fprintf(stderr, "gethostbyname %s\n", name);
	*errnop = ENOENT;
	*herrnop = HOST_NOT_FOUND;
	return NSS_STATUS_NOTFOUND;
}

enum nss_status
_nss_loopback_gethostbyname2_r(const char *name, int af, struct hostent *result,
                              char *buffer, size_t buflen, int *errnop,
                              int *herrnop)
{
	fprintf(stderr, "gethostbyname2 %s\n", name);
	*errnop = ENOENT;
	*herrnop = HOST_NOT_FOUND;
	return NSS_STATUS_NOTFOUND;
}

/*
_nss_files_gethostbyname2_r;
_nss_files_gethostbyname3_r;
_nss_files_gethostbyname4_r;
_nss_files_gethostbyaddr_r;
gethostbyaddr2?
gethostton, getntohost?
*/

int __attribute__((weak))
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: libnss_loopback.so.2 <hostname>\n");
		return 1;
	}
	printf("%d\n", loopback_match(argv[1]));
}
