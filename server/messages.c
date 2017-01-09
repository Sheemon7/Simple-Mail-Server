#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "messages.h"

void destroy_message(message **m);
message *create_message(char *user, char *message);
void print_message(message *m);

messages_saver *init_saver(int capacity) {
	messages_saver *s = malloc(sizeof(messages_saver));
	s->capacity = capacity;
	s->size = 0;
	s->first = NULL;
}

void destroy_saver(messages_saver **saver) {
	message *prev = NULL, *curr;
	for (curr = (*saver)->first; curr != NULL; curr = curr->next) {
		if(prev != NULL) {
			destroy_message(&prev);
		}
		prev = curr;
	}
	if(prev != NULL) {
		destroy_message(&prev);
	}
	free(*saver);
	saver = NULL;
}

/* 0 if succesfull 1 if full */
int save_message(messages_saver *s, char *to, char *content) {
	if (s->size >= s->capacity) { return 1; }
	++s->size;
	message *m = create_message(to, content);
	if (s->size == 1) {
		s->first = m;
	} else {
		message *curr, *prev;
		for(curr = s->first; curr != NULL; curr = curr->next) {
			prev = curr;
		}
		prev->next = m;
	}
	return 0;
}

/* 0 if succesfull 1 if no messagess */
int get_saved_message(messages_saver *s, char *to, char *content) {
	message *curr, *prev = NULL;
	for(curr = s->first; curr != NULL; curr = curr->next) {
		if (strcmp(curr->user, to) == 0) {
			strcpy(content, curr->content);
			if (prev == NULL) {
				s->first = curr->next;
			} else {
				prev->next = curr->next;
			}
			--s->size;
			destroy_message(&curr);
			return 0;
		}
		prev = curr;
	}
	return 1;
}

void print_messages(messages_saver *s) {
	printf("Currently there are %d messages saved\n", s->size);
	for(message *curr = s->first; curr != NULL; curr = curr->next) {
		print_message(curr);
	}
}

/* HELPER FUNCTIONS */

void print_message(message *m) {
	printf("Message to: %s\nContent: %s\n", m->user, m->content);
}

void destroy_message(message **m) {
	free((*m)->user);
	free((*m)->content);
}

message *create_message(char *user, char *content) {
	message *m = malloc(sizeof(message));
	m->user = malloc(strlen(user));
	strcpy(m->user, user);
	m->content = malloc(strlen(content));
	strcpy(m->content, content);
	m->next = NULL;
	return m;
}
