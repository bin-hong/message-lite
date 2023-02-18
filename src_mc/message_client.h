
#define SERVER_PORT 3491

#define MESSAGE_MAX_LEN 1023
#define NAME_MAX_LEN 19
#define NAME_MIN_LEN  2
#define PASSWD_MAX_LEN 19
#define PASSWD_MIN_LEN 2

struct protocol{
	int cmd;
	int state;
	char name[50];
	char passwd[1024];
};
