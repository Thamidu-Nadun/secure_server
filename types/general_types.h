#ifndef GENERAL_TYPES_H
#define GENERAL_TYPES_H

struct Command {
    char command_name[100]; // command name
    char args[256]; // command arguments
} typedef Command;

struct Response {
    int code; // response code
    int sid; // server ID
    char *message; // response message
} typedef Response;

enum ServerResponseTypes{
    OK, ERR
} typedef ServerResponseTypes;

struct User{
    char username[256];
    char password[256];
} typedef User;

typedef struct {
    char username[256];
    char token[256];
    time_t expiry;
} session_t;

#endif