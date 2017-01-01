#include <stdio.h>
#include <stdlib.h>
#include "server/password/hash_function.h"
#include "server/password/password_handle.h"

int main() {
	printf("%lu\n", hash_function("SIMON"));
	printf("%lu\n", hash_function("SIMON"));
	printf("%lu\n", hash_function("HESLO"));
	printf("%lu\n", hash_function("heslasdfasdfasdfo"));
	save_password("simon", "abcd");
	save_password("simon1", "abcde");
	save_password("simon2", "abcdef");
	printf("%d\n", authenticate("simon", "abcd"));
	printf("%d\n", authenticate("simon1", "abcde"));
	printf("%d\n", authenticate("simon2", "abcdef"));
	printf("%d\n", authenticate("simon", "absdfcd"));
	printf("%d\n", authenticate("simon", "absdfcd"));
}

