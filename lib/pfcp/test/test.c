#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

const char srcPath[] = "./testbin_pfcp";

int RunTest(const char *filename) {
    int status;
    static char buffer[0x80];
    memset(buffer, 0, 0x80);
    sprintf(buffer, "%s/%s", srcPath, filename);
    printf("[Testing] %s start\n", buffer);
    status = system(buffer);
    printf("[Testing] %s done\n", buffer);

    return status;
}

int main(int argc, char *argv[]) {
    
    if (argc == 1) {
        struct dirent *dirEntry;
        DIR *dir = opendir(srcPath); 
        if (dir == NULL) {
            printf("Could not open %s\n", srcPath);
            return 0;
        }
        while ((dirEntry = readdir(dir)) != NULL) {
            if (strcmp(dirEntry->d_name, ".") && 
                strcmp(dirEntry->d_name, "..") &&
                strcmp(dirEntry->d_name + strlen(dirEntry->d_name) - 2, ".c")) {
                printf("# %s\n", dirEntry->d_name);
                RunTest(dirEntry->d_name);
            }
        }
        closedir(dir);
    } else {
        for (int i = 1; i < argc; i++) {
            RunTest(argv[i]);
        }
    }

    return 0;
}
