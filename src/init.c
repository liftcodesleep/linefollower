#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void update_remote_url(const char *repository_directory) {
    char command[1024];

    // Get the current remote URL
    snprintf(command, sizeof(command), "git config --get remote.origin.url");
    FILE *remoteURL = popen(command, "r");

    if (remoteURL == NULL) {
        perror("Error opening pipe for remote URL");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    fgets(buffer, sizeof(buffer), remoteURL);

    // Check if the remote URL uses HTTPS
    if (strstr(buffer, "https://") != NULL) {
        // Replace HTTPS with SSH in the remote URL
        snprintf(command, sizeof(command), "git remote set-url origin git@github.com:%s",
                 buffer + strlen("https://"));

        // Update the remote URL to use SSH
        if (system(command) == -1) {
            perror("Error updating remote URL");
            exit(EXIT_FAILURE);
        }

        printf("Updated remote URL to: git@github.com:%s\n", buffer + strlen("https://"));
    }

    pclose(remoteURL);
}

void git_pull(const char *branch) {
    char command[1024];

    // Get the current working directory
    char current_directory[1024];
    if (getcwd(current_directory, sizeof(current_directory)) == NULL) {
        perror("Error getting current working directory");
        exit(EXIT_FAILURE);
    }

    // Update the remote URL if needed
    update_remote_url(current_directory);

    // Perform git pull in the current directory
    snprintf(command, sizeof(command), "git checkout %s", branch);
    if (system(command) == -1) {
        perror("Error checking out branch");
        exit(EXIT_FAILURE);
    }

    snprintf(command, sizeof(command), "git pull origin %s", branch);
    if (system(command) == -1) {
        perror("Error pulling changes");
        exit(EXIT_FAILURE);
    }

    printf("Git pull successful!\n");
}

int main() {
    // Specify the branch you want to pull (default is "main")
    const char *branch = "main";

    // Call the function to perform git pull
    git_pull(branch);

    return 0;
}
