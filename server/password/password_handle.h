#ifndef PASSWORD_HANDLE_H
#define PASSWORD_HANDLE_H

int authenticate(char *user, char *psswd);
int save_password(char *user, char *psswd);
int user_exists(char *user);

#endif
