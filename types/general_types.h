struct Command {
    char *command_name;
    void *args;
} typedef Command;

struct Response {
    int code; // response code
    int sid; // server ID
    char *message; // response message
} typedef Response;

enum ServerResponseTypes{
    OK, ERR
} typedef ServerResponseTypes;