#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../types/general_types.h"
#include "secure.c"
#include "token.h"

#define DB_PATH "users.txt"

void lock_db(int fd);
void unlock_db(int fd);
int add_user(const char* username, const char* password);
User* get_user(const char* username);

void lock_db(int fd){
    struct flock lock;
    lock.l_type = F_WRLCK; // Write lock
    lock.l_whence = SEEK_SET; // Lock from the beginning
    lock.l_start = 0; // Start of the file
    lock.l_len = 0; // Lock the entire file
    
    fcntl(fd, F_SETLKW, &lock); // Wait until lock is acquired
}

void unlock_db(int fd){
    struct flock lock;
    lock.l_type = F_UNLCK; // Unlock
    lock.l_whence = SEEK_SET; // Unlock from the beginning
    lock.l_start = 0; // Start of the file
    lock.l_len = 0; // Unlock the entire file
    
    fcntl(fd, F_SETLK, &lock); // Release the lock
}

int add_user(const char* username, const char* password) {
    FILE* file = fopen(DB_PATH, "a");
    if (file == NULL) {
        perror("Could not open database file");
        return EXIT_FAILURE;
    }
    lock_db(fileno(file)); // Lock the file for writing

    User* existing_user = get_user(username);
    if (existing_user) {
        printf("User '%s' already exists. Skipping addition.\n", username);
        free(existing_user);
        unlock_db(fileno(file)); 
        fclose(file);
        return EXIT_FAILURE;
    }
    int chars = 6;
    char salt[chars + 1];
    generate_token(salt, sizeof(salt));

    char password_hash[65];
    create_password_hash(password, salt, password_hash);
    fprintf(file, "%s:%s:%s\n", username, salt, password_hash);
    printf("User '%s' added successfully.\n", username);
    unlock_db(fileno(file)); // Unlock the file
    fclose(file);
    return EXIT_SUCCESS;
}

/**
 * Retrieves a user from the database by username
 * @param username The username to search for
 * @return A pointer to a User struct if found, or NULL if not found
 * @note free the returned User struct
 */
User* get_user(const char* username){
    FILE* file = fopen(DB_PATH, "r");
    if (file == NULL) {
        perror("Could not open database file");
        return NULL;
    }
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        char user_buff[256], salt_buff[256], pass_buff[256];

        if (sscanf(line, "%[^:]:%[^:]:%[^\n]", user_buff, salt_buff, pass_buff) == 3) {
            if (strcmp(user_buff, username) == 0){
                User* user = malloc(sizeof(User));
                if (!user) {
                    perror("Memory allocation failed");
                    fclose(file);
                    return NULL;
                }

                memset(user, 0, sizeof(User));

                strncpy(user->username, user_buff, sizeof(user->username) - 1);
                strncpy(user->salt, salt_buff, sizeof(user->salt) - 1);
                strncpy(user->password, pass_buff, sizeof(user->password) - 1);

                fclose(file);
                return user;
            }
        }
    }

    fclose(file);
    return NULL; // User not found
}

int logout_user(const char* username) {
    // 1. find user in tokens.txt
    // 2. remove the line containing the user
    FILE* file = fopen("tokens.txt", "r");
    FILE* temp = fopen("tokens-temp.txt", "w");
    if (!file || !temp) {
        perror("Could not open tokens file");
        return EXIT_FAILURE;
    }
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, username) == NULL) {
            fputs(line, temp);
        }
    }
    fclose(file);
    fclose(temp);

    if (remove("tokens.txt") != 0) {
        perror("Could not remove tokens file");
        return EXIT_FAILURE;
    }

    if (rename("tokens-temp.txt", "tokens.txt") != 0) {
        perror("Could not rename temp tokens file");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// int main() {
//     add_user("user1", "password1");
//     add_user("user2", "password2");

//     User* u1 = get_user("user1");
//     if (u1) {
//         printf("Retrieved user: %s with password: %s\n", u1->username, u1->password);
//         free(u1);
//     } else {
//         printf("User not found.\n");
//     }
//     return 0;
// }