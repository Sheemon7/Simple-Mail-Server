#ifndef MESSAGES_H
#define MESSAGES_H

typedef struct message {
	char *user;
	char *content;
	struct message *next;
} message;

typedef struct message_saver {
	int size;
	int capacity; // limited to 16
	message *first;

} messages_saver;

messages_saver *init_saver();
void destroy_saver(messages_saver **s);
/* 0 if succesfull 1 if full */
int save_message(messages_saver *s, char *to, char *content);
/* 0 if succesfull 1 if no messagess */
int get_saved_message(messages_saver *s, char *to, char *content);
void print_messages(messages_saver *s);

#endif