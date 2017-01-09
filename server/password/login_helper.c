#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "login_helper.h"

/* HELPER FUNCTIONS */
void destroy_user(logged_user **user);
logged_user *create_user(int fd, char *name, int name_length, char *password, int password_length);
void print_user(logged_user *user);


login_helper *init() {
	login_helper *helper = malloc(sizeof(login_helper));
	helper->first = NULL;
	helper->size = 0;
	return helper;
}

void destroy(login_helper **helper) {
	logged_user *prev = NULL, *curr;
	for (curr = (*helper)->first; curr != NULL; curr = curr->next) {
		if(prev != NULL) {
			destroy_user(&prev);
		}
		prev = curr;
	}
	if(prev != NULL) {
		destroy_user(&prev);
	}
	free(*helper);
	helper = NULL;
}

int add_user(login_helper *helper, int fd, char *name, int name_length, char *password, int password_length) {
	++helper->size;
	logged_user *curr;
	for (curr = helper->first; curr != NULL; curr = curr->next) {
		if (strcmp(curr->name, name) == 0) {
			if (strcmp(curr->password, password) == 0) {
				curr->fd = fd;
				curr->available = 1;
				print(helper);
				return 1;
			} else {
				print(helper);
				return -1;
			}
		}
	}
	logged_user *user = create_user(fd, name, name_length, password, password_length);
	user->next = helper->first;
	helper->first = user;
	print(helper);
	return 1;
}

int remove_user_fd(login_helper *helper, int fd) {
	logged_user *curr, *prev = NULL;
	for(curr = helper->first; curr != NULL; curr = curr->next) {
		if (curr->fd == fd) {
			/*if (prev == NULL) {
				helper->first = curr->next;
			} else {
				prev->next = curr->next;
			}
			destroy_user(&curr);*/
			curr->available = 0;
			--helper->size;	
			print(helper);
			return 1;
		}
		prev = curr;
	}
	print(helper);
	return -1;
}

int remove_user_name(login_helper *helper, char *name) {
	logged_user *curr, *prev = NULL;
	for(curr = helper->first; curr != NULL; curr = curr->next) {
		if (!strcmp(curr->name, name)) {
			/*if (prev == NULL) {
				helper->first = curr->next;
			} else {
				prev->next = curr->next;
			}
			destroy_user(&curr);*/
			curr->available = 0;
			--helper->size;
			print(helper);
			return 1;
		}
		prev = curr;
	}
	print(helper);
	return -1;
}

logged_user *get_user_fd(login_helper *helper, int fd, int *code) {\
	logged_user *curr = NULL;
	for(curr = helper->first; curr != NULL; curr = curr->next) {
		if (curr->fd == fd) {
			break;
		}
	}
	if (curr != NULL && curr->available) {
		*code = 0;
	} else {
		*code = -1;
	}
	return curr;
}

logged_user *get_user_name(login_helper *helper, char *name, int *code) {
	logged_user *curr = NULL;
	for(curr = helper->first; curr != NULL; curr = curr->next) {
		if (!strcmp(curr->name, name)) {
			break;
		}
	}
	if (curr != NULL && curr->available) {
		*code = 0;
	} else {
		*code = -1;
	}
	return curr;
}

void print(login_helper *helper) {
	printf("Currently there are %d users logged\n", helper->size);
	printf("-----------------------------\n");
	for(logged_user *curr = helper->first; curr != NULL; curr = curr->next) {
		if (curr->available) {
			print_user(curr);
			printf("\n-----------------------------\n");
		}
	}
}

/* HELPER FUNCTIONS */

void print_user(logged_user *user) {
	printf("\tUsername: %s\tPassword: %s\tFd: %d\tLast Message: %s", user->name, user->password, user->fd, user->last_msg);
}

void destroy_user(logged_user **user) {
	free((*user)->name);
	free((*user)->password);
	free((*user)->last_msg);
	free(*user);
	user = NULL;
}


logged_user *create_user(int fd, char *name, int name_length, char *password, int password_length) {
	logged_user *user = malloc(sizeof(logged_user));
	user->fd = fd;
	user->available = 1;
	user->name = malloc(name_length * sizeof(char));
	strcpy(user->name, name);
	user->password = malloc(password_length * sizeof(char));
	strcpy(user->password, password);
	user->next = NULL;
	user->last_msg = NULL;
	return user;
}

