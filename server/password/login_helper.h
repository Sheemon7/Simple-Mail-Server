#ifndef LOGIN_HELPER_H
#define LOGIN_HELPER_H

typedef struct logged_user {
	int fd;
	char *name;
	struct logged_user *next;
} logged_user;

typedef struct login_helper {
	/* linked list of currently logged in users */
	logged_user *first;
	int size;
} login_helper;


login_helper *init();
void destroy(login_helper **helper);
int add_user(login_helper *helper, int fd, char *name, int name_length);
int remove_user_fd(login_helper *helper, int fd);
int remove_user_name(login_helper *helper, char *name);
int user_exists(login_helper *helper, char *name);
void print(login_helper *helper);
#endif
