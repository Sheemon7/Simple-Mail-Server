#include <stdlib.h>
#include <stdio.h>

#include "login_helper.h"

int main() {
	login_helper *h = init();
	print(h);
	printf("Adding simon :%d\n", add_user(h, 0, "simon", 6, "heslo", 6));
	print(h);
	printf("Adding tomas :%d\n", add_user(h, 1, "tomas", 6, "bubu", 5));
	print(h);
	printf("Trying to add simon with another password: %d\n", add_user(h, 2, "simon", 6, "wrong", 6));
	print(h);
	printf("Trying to readd tomas with another fd: %d\n", add_user(h, 3, "tomas", 6, "bubu", 5));
	print(h);
	printf("Trying to remove non existing user: %d\n", remove_user_name(h, "dita"));
	print(h);
	printf("Trying to remove simon: %d\n", remove_user_name(h, "simon"));
	print(h);
	printf("Trying to remove non existing fd: %d\n", remove_user_fd(h, 10));
	print(h);
	printf("Trying to existing fd: %d\n", remove_user_fd(h, 3));
	print(h);

	destroy(&h);
	h = NULL;
}
