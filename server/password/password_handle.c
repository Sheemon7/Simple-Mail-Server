#include "hash_function.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *FNAME = "./passwords";

static FILE *open_password_file(char *mode) {
	FILE *f = fopen(FNAME, mode);
	if (f == NULL) {
		fprintf(stderr, "Error in opening password file!");	
	}
	return f;
}

static unsigned long get_password_hash(char *user) {
	FILE *f = open_password_file("r");
	char u[100];
	unsigned long psswd;
	while (fscanf(f, "%s %lu\n", u, &psswd) == 2) {
		printf("%s %s %lu\n", user, u, psswd);
		if (strcmp(u, user) == 0) {
			fclose(f);
			return psswd;
		}
	}
	fclose(f);
	return -1;	
}

int user_exists(char *user) {
	FILE *f = open_password_file("r");
	char u[100];
	unsigned long psswd;
	while (fscanf(f, "%s %lu\n", u, &psswd) == 2) {
		printf("%s %s %lu\n", user, u, psswd);
		if (strcmp(u, user) == 0) {
			fclose(f);
			return 1;
		}
	}
	fclose(f);
	return -1;	
}

int authenticate(char *user, char *psswd) {
	if (get_password_hash(user) == hash_function(psswd)) {
		return 1;
	} else {
		return -1;
	}
}

int save_password(char *user, char *psswd) {
	FILE *f = open_password_file("a+");
	unsigned long psswd_hash = hash_function(psswd);
	int ret;
	if (fprintf(f, "%s %lu\n", user, psswd_hash) == 2) {
		ret = 1;
	} else {
		ret = -1;
	}	
	fclose(f);
	return ret;
}

