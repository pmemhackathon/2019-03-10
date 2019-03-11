/*
 * volatile_pmem.c -- example showing libmemkind for volatile pmem use
 */
#include <memkind.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* for error messages */
#define MAXMSG 1024
char message[MAXMSG];

int main(int argc, char *argv[])
{
	struct memkind *pmem_kind;	/* memory "kind" for pmem allocs */
	size_t pmem_size;
	int retval;

	if (argc != 3) {
		fprintf(stderr, "usage: %s pmem-dir nmegs\n", argv[0]);
		exit(1);
	}
	pmem_size = atoi(argv[2]) * 1024 * 1024;
	printf("Creating memkind for %ld bytes of pmem at %s\n",
			pmem_size, argv[1]);

	/* allocate a 100M pool from pmem */
	retval = memkind_create_pmem(argv[1], pmem_size, &pmem_kind);
	if (retval) {
		memkind_error_message(retval, message, MAXMSG);
		errx(1, "memkind_create_pmem: %s", message);
	}

	/* allocation examples without NULL checks for brevity... */

	/* works as expected, caller must remember to use free() on it */
	char *string = malloc(100);

	/* allocates appropriate "kind" of volatile memory */
	char *dstring = memkind_malloc(MEMKIND_DEFAULT, 100);
	char *pstring = memkind_malloc(pmem_kind, 100);
	perror("pmem_kind");

	printf("%p string\n", (void *)string);
	printf("%p dstring\n", (void *)dstring);
	printf("%p pstring\n", (void *)pstring);

	printf("to see where the memory comes from, use:\n");
	printf("cat /proc/%d/maps\n", getpid());

	pause();

	exit(0);
}
