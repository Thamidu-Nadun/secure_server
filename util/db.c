#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../types/general_types.h"

#define DB_PATH "users.txt"

void lock_db(int fd);
void unlock_db(int fd);
void add_user(const char* username, const char* password);
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

void add_user(const char* username, const char* password) {
    FILE* file = fopen(DB_PATH, "a");
    if (file == NULL) {
        perror("Could not open database file");
        return;
    }
    lock_db(fileno(file)); // Lock the file for writing

    User* existing_user = get_user(username);
    if (existing_user) {
        printf("User '%s' already exists. Skipping addition.\n", username);
        free(existing_user);
        unlock_db(fileno(file)); 
        fclose(file);
        return;
    }
    fprintf(file, "%s:%s\n", username, password);
    printf("User '%s' added successfully.\n", username);
    unlock_db(fileno(file)); // Unlock the file
    fclose(file);    
}

/**
 * Retrieves a user from the database by username
 * @param username The username to search for
 * @return A pointer to a User struct if found, or NULL if not found
 * @note free the returned User struct | cuz return pointer to memory
 */
User* get_user(const char* username){
    FILE* file = fopen(DB_PATH, "r");
    if (file == NULL) {
        perror("Could not open database file");
        return NULL;
    }
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        char user_buff[256], pass_buff[256];

        if (sscanf(line, "%[^:]:%[^\n]", user_buff, pass_buff) == 2) {
            if (strcmp(user_buff, username) == 0){
                User* user = malloc(sizeof(User));
                if (!user) {
                    perror("Memory allocation failed");
                    fclose(file);
                    return NULL;
                }

                memset(user, 0, sizeof(User));

                strncpy(user->username, user_buff, sizeof(user->username) - 1);
                strncpy(user->password, pass_buff, sizeof(user->password) - 1);

                fclose(file);
                return user;
            }
        }
    }

    fclose(file);
    return NULL; // User not found
}

int main() {
    add_user("user1", "password1");
    add_user("user2", "password2");

    User* u1 = get_user("user1");
    if (u1) {
        printf("Retrieved user: %s with password: %s\n", u1->username, u1->password);
        free(u1);
    } else {
        printf("User not found.\n");
    }
    return 0;
}