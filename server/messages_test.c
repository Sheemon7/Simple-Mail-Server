#include <stdio.h>
#include <stdlib.h>

#include "messages.h"

int main() {
	char buf[256];

	messages_saver *s = init_saver();
	print_messages(s);
	printf("%d\n", get_saved_message(s, "simon", buf));
	printf("%d\n", save_message(s, "simon", "ahoj Simone"));
	print_messages(s);
	printf("%d\n", save_message(s, "tomas", "ahoj Tomasi"));
	print_messages(s);
	printf("%d\n", get_saved_message(s, "tomas", buf));
	printf("%s\n", buf);
	print_messages(s);
	printf("%d\n", get_saved_message(s, "simon", buf));
	printf("%s\n", buf);
	print_messages(s);

	for(int i = 0; i < 20; ++i) {
		printf("%d\n", save_message(s, "simon", "ahoj Simone"));
	}
	print_messages(s);
	for(int i = 0; i < 20; ++i) {
		printf("%d\n", get_saved_message(s, "simon", buf));
		printf("%s\n", buf);
	}
	print_messages(s);
	destroy_saver(&s);
	s = NULL;
}