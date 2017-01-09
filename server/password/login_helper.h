#ifndef LOGIN_HELPER_H
#define LOGIN_HELPER_H

typedef struct logged_user {
	int fd, available;
	char *name;	
	char *password;
	char *last_msg;
	struct logged_user *next;
} logged_user;

typedef struct login_helper {
	/* linked list of currently logged in users */
	logged_user *first;
	int size;
} login_helper;


login_helper *init();
void destroy(login_helper **helper);
/* returns 1 in case of succesfull login or account creation, -1 if user exists with different pass */
int add_user(login_helper *helper, int fd, char *name, int name_length, char *password, int password_length);
int remove_user_fd(login_helper *helper, int fd);
int remove_user_name(login_helper *helper, char *name);
logged_user *get_user_fd(login_helper *helper, int fd, int *code);
logged_user *get_user_name(login_helper *helper, char *name, int *code);
void print(login_helper *helper);

#endif
