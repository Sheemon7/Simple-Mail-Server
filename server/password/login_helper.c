#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "login_helper.h"

/* HELPER FUNCTIONS */
void destroy_user(logged_user **user);
logged_user *create_user(int fd, char *name, int name_length);
void print_user(logged_user *user);


login_helper *init() {
	login_helper *helper = malloc(sizeof(login_helper));
	helper->first = NULL;
	return helper;
}

void destroy(login_helper **helper) {
	logged_user *prev = NULL, *curr;
	for (curr = (*helper)->first; curr != NULL; curr = curr->next) {
		if(prev != NULL) {
			destroy_user(&prev);
			prev = curr;
		}
	}
	free(*helper);
	helper = NULL;
}

int add_user(login_helper *helper, int fd, char *name, int name_length) {
	 logged_user *user = create_user(fd, name, name_length);
	// check if exists - maybe not needed
	if (user_exists(helper, name)) { return -1; }
	user->next = helper->first;
	helper->first = user;
	++helper->size;
	return 1;
}

int remove_user_fd(login_helper *helper, int fd) {
	logged_user *curr, *prev = NULL;
	for(curr = helper->first; curr != NULL; curr = curr->next) {
		if (curr->fd == fd) {
			if (prev == NULL) {
				helper->first = curr->next;
			} else {
				prev->next = curr->next;
			}
			destroy_user(&curr);
			--helper->size;	
			return 1;
		}
		prev = curr;
	}
	return -1;
}

int remove_user_name(login_helper *helper, char *name) {
	logged_user *curr, *prev = NULL;
	for(curr = helper->first; curr != NULL; curr = curr->next) {
		if (!strcmp(curr->name, name)) {
			if (prev == NULL) {
				helper->first = curr->next;
			} else {
				prev->next = curr->next;
			}
			destroy_user(&curr);
			--helper->size;
			return 1;
		}
		prev = curr;
	}
	return -1;
}

void print(login_helper *helper) {
	printf("Currently there are %d users logged\n", helper->size);
	for(logged_user *curr = helper->first; curr != NULL; curr = curr->next) {
		print_user(curr);
	}
}

/* HELPER FUNCTIONS */

void print_user(logged_user *user) {
	printf("Username: %s\tFd: %d\n", user->name, user->fd);
}

void destroy_user(logged_user **user) {
	free((*user)->name);
	free(*user);
	user = NULL;
}


logged_user *create_user(int fd, char *name, int name_length) {
	logged_user *user = malloc(sizeof(logged_user));
	user->fd = fd;
	user->name = malloc(name_length * sizeof(char));
	user->next = NULL;
	return user;
}

