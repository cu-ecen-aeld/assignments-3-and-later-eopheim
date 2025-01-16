#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <syslog.h>

int main(int argc, char *argv[]){
    
    openlog(NULL,0,LOG_USER);

    // First argument is file path, assumed created
    // Second argument is string to write to file specified by path
    

    // Error and exit if any parameters not specified
    // LOG_ERROR
    if(argc < 2){
        syslog(LOG_ERR, "Invalid number of arguments: %d", argc);
        
        closelog();

        return 1;
    }

    int fd;

    fd = creat(argv[1], 0644);
    
    if (fd == -1){
        syslog(LOG_ERR, "Unable to create file: %s", argv[1]);
        closelog();

        return 1;
    }

    ssize_t nr;
    nr = write(fd, argv[2], strlen(argv[2]));

    if (nr == -1){
        syslog(LOG_ERR, "Unable to write to file: %s", argv[1]);
        closelog();

        close(fd);

        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    closelog();
    close(fd);


    return 0;
}
