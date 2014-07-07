#include <nss.h>
#include <stdlib.h>

static void __attribute__((constructor))
nsstest_ctor(void)
{
	const char *db = getenv("NSSTEST_DB"), *config = getenv("NSSTEST_CONFIG");
	if (db && config)
		__nss_configure_lookup(db, config);
}
