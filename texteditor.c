#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_FILENAME_LEN 1000
#define MAX_BUFFER_LEN 10000

// Node structure to store information about each text file
typedef struct TextFileNode {
    char filename[MAX_FILENAME_LEN];
    char content[MAX_BUFFER_LEN];
    struct TextFileNode* next;
} TextFileNode;

// Global variables
TextFileNode* files = NULL; // Linked list to store loaded text files
char currentDirectory[MAX_FILENAME_LEN] = ""; // Store the current directory path

// Function prototypes
void loadFiles();
void saveFiles();
void openFile(const char* filename);
void deleteFile(const char* filename);
void saveFile(const char* filename);
void appendToFile(const char* filename);
void listFiles();
void printHelp();

// Main function
int main() {
    // Set the current directory to the program's working directory
    if (getcwd(currentDirectory, sizeof(currentDirectory)) == NULL) {
        perror("Error getting current directory");
        exit(EXIT_FAILURE);
    }

    printf("Welcome to our Text Editor! Use the 'help' command for more info.\n");

    // Load existing files in the current directory into the linked list
    loadFiles();

    while (1) {
        char command[10];
        printf("Enter a command (open, save, delete, append, list, help, or exit): ");
        scanf("%s", command);

        if (strcmp(command, "open") == 0) {
            char filename[MAX_FILENAME_LEN];
            printf("Enter the filename to open: ");
            scanf("%s", filename);
            openFile(filename);
        } else if (strcmp(command, "save") == 0) {
            char filename[MAX_FILENAME_LEN];
            printf("Enter the filename to save: ");
            scanf("%s", filename);
            saveFile(filename);
        } else if (strcmp(command, "delete") == 0) {
            char filename[MAX_FILENAME_LEN];
            printf("Enter the filename to delete: ");
            scanf("%s", filename);
            deleteFile(filename);
        } else if (strcmp(command, "append") == 0) {
            char filename[MAX_FILENAME_LEN];
            printf("Enter the filename to append: ");
            scanf("%s", filename);
            appendToFile(filename);
        } else if (strcmp(command, "list") == 0) {
            listFiles();
        } else if (strcmp(command, "help") == 0) {
            printHelp();
        } else if (strcmp(command, "exit") == 0) {
            printf("Exiting the text editor.\n");
            break;
        } else {
            printf("Invalid command. Please use 'open', 'save', 'delete', 'append', 'list', 'help', or 'exit'.\n");
        }
    }

    return 0;
}

// Function to load existing files in the current directory into the linked list
void loadFiles() {
    DIR* dir;
    struct dirent* entry;

    dir = opendir(currentDirectory);
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Check if it's a regular file
            TextFileNode* newNode = (TextFileNode*)malloc(sizeof(TextFileNode));
            if (newNode == NULL) {
                perror("Memory allocation error");
                exit(EXIT_FAILURE);
            }
            strcpy(newNode->filename, entry->d_name);
            newNode->next = files;
            files = newNode;
        }
    }

    closedir(dir);
}

// Function to save all files in the linked list to their respective locations
void saveFiles() {
    TextFileNode* file = files;

    while (file != NULL) {
        char fullpath[MAX_FILENAME_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", currentDirectory, file->filename);

        FILE* savedFile = fopen(fullpath, "w");
        if (savedFile == NULL) {
            perror("Error saving file");
            exit(EXIT_FAILURE);
        }
        fputs(file->content, savedFile);
        fclose(savedFile);

        file = file->next;
    }
}

// Function to open and display the content of a text file
void openFile(const char* filename) {
    char fullpath[MAX_FILENAME_LEN];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", currentDirectory, filename);

    FILE* file = fopen(fullpath, "r");
    if (file == NULL) {
        printf("File not found.\n");
        return;
    }

    TextFileNode* newNode = (TextFileNode*)malloc(sizeof(TextFileNode));
    if (newNode == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    strcpy(newNode->filename, filename);
    while (fgets(newNode->content, MAX_BUFFER_LEN, file) != NULL) {
        printf("%s", newNode->content);
    }
    fclose(file);

    newNode->next = files;
    files = newNode;
}

// Function to delete a text file and remove it from the linked list
void deleteFile(const char* filename) {
    TextFileNode* current = files;
    TextFileNode* previous = NULL;

    while (current != NULL) {
        if (strcmp(current->filename, filename) == 0) {
            char fullpath[MAX_FILENAME_LEN];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", currentDirectory, filename);

            if (remove(fullpath) == 0) {
                printf("File deleted successfully.\n");
            } else {
                printf("Unable to delete the file.\n");
            }

            if (previous == NULL) {
                files = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            return;
        }

        previous = current;
        current = current->next;
    }

    printf("File not found.\n");
}

// Function to create or overwrite a text file and save content to it
void saveFile(const char* filename) {
    TextFileNode* newNode = (TextFileNode*)malloc(sizeof(TextFileNode));
    if (newNode == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    char fullpath[MAX_FILENAME_LEN];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", currentDirectory, filename);

    FILE* file = fopen(fullpath, "w");
    if (file == NULL) {
        printf("Could not create the file.\n");
        return;
    }

    strcpy(newNode->filename, filename);
    printf("Enter text. (Press Ctrl+Z and Enter to save and exit):\n");
    while (1) {
        if (fgets(newNode->content, MAX_BUFFER_LEN, stdin) == NULL) {
            if (feof(stdin)) {
                break;
            } else {
                printf("Error reading input.\n");
                break;
            }
        }
        fputs(newNode->content, file);
    }
    fclose(file);

    newNode->next = files;
    files = newNode;
    printf("File saved.\n");
}

// Function to append content to an existing text file
void appendToFile(const char* filename) {
    TextFileNode* file = files;

    while (file != NULL) {
        if (strcmp(file->filename, filename) == 0) {
            // Move to the end of the content
            while (file->next != NULL) {
                file = file->next;
            }

            char fullpath[MAX_FILENAME_LEN];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", currentDirectory, filename);

            FILE* appendFile = fopen(fullpath, "a");
            if (appendFile == NULL) {
                printf("Could not open the file for appending.\n");
                return;
            }

            fseek(appendFile, 0, SEEK_END);  // Move to the end of the file

            printf("Enter text to append. Press Ctrl+Z and Enter to finish:\n");

            // Use getchar to read input character by character
            int ch;
            while ((ch = getchar()) != EOF) {
                fputc(ch, appendFile);
            }

            fclose(appendFile);
            printf("File '%s' appended successfully.\n", filename);
            return;
        }
        file = file->next;
    }

    printf("File not found.\n");
}

// Function to list all files in the current directory
void listFiles() {
    TextFileNode* file = files;

    if (file == NULL) {
        printf("No files in the current directory.\n");
        return;
    }

    printf("Files in the current directory:\n");

    while (file != NULL) {
        printf("%s\n", file->filename);
        file = file->next;
    }
}

// Function to print information about available commands
void printHelp() {
    printf("Available commands:\n");
    printf("  open    - Open a file\n");
    printf("  save    - Save content to a file\n");
    printf("  delete  - Delete a file\n");
    printf("  append  - Append content to a file\n");
    printf("  list    - List all files in the current directory\n");
    printf("  help    - Display help information\n");
    printf("  exit    - Exit the text editor\n");
}
