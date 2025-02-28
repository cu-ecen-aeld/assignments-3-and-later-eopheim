#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>

#define BUF_SIZE 500

bool caught_sig = false;


static void handler(int signum){
    if(signum == SIGINT)
        caught_sig = true;
    if(signum == SIGTERM)
        caught_sig = true;
}

int main(int argc, char *argv[]){
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = handler;

    if(sigaction(SIGTERM, &new_action, NULL) != 0){
        syslog(LOG_ERR, "Error registering for SIGTERM");
        return -1;
    }

    if(sigaction(SIGINT, &new_action, NULL) != 0){
        syslog(LOG_ERR, "Error registering for SIGINT");
        return -1;
    }

    const char * filename = "/var/tmp/aesdsocketdata";

    int sockfd, s;
    ssize_t nread;
    socklen_t peer_addrlen;
    const char * port = "9000";
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    struct sockaddr_storage peer_addr;

    // Allocate memory for TCP buffer
    char * buf = (char *)malloc(BUF_SIZE);
    if(buf == NULL){
        syslog(LOG_ERR, "Error: malloc failed");
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // iPV4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Any

    s = getaddrinfo(NULL, port, &hints, &result);
    if(s != 0){
        syslog(LOG_ERR, "getaddrinfo failed");
        return -1;
    }

    // Find an available socket for server
    for(rp = result; rp != NULL; rp = rp->ai_next){
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if(sockfd == -1)
            continue;

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
            syslog(LOG_ERR, "Error setsockopt SO_RESUEADDR");

        if(bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0){
            break; // Success

        }
    }
    
    freeaddrinfo(result);
    
    if(rp == NULL){
        syslog(LOG_ERR, "Could not bind");
        return -1;
    }

    pid_t pid;

    // If -d fork
    if(argc == 2){
        if(strcmp(argv[1], "-d") == 0){
            printf("Starting daemon\n");
            pid = fork();
        }
    }
    else
        pid = 0;

    if(pid == -1){
        syslog(LOG_ERR, "fork failed");
        // Close file and socket. Free buffer memory allocation
        close(sockfd);
        free(buf);
        remove(filename);
        return -1;
    }  
    else if(pid == 0){
        if(listen(sockfd, 10) == -1){
            syslog(LOG_ERR, "Error listening");
            return -1;
        }

        // Open and close file to ensure empty when starting
        int fd;
        fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
        close(fd);
        

        while(!caught_sig){
            
            fd = open(filename, O_RDWR | O_APPEND);

            peer_addrlen = sizeof(peer_addr);
            
            int acptfd = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addrlen);

            if(caught_sig){
                close(acptfd);
                break;
            }
                
            if(acptfd == -1){
                syslog(LOG_ERR, "accept error. continuing...");
                continue;
            }

            // Get client IP address and write to syslog
            char peer_ip[12];
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) &peer_addr;
            inet_ntop(AF_INET, &(ipv4->sin_addr), peer_ip, sizeof(peer_addr));
            syslog(LOG_DEBUG, "Accepted connection from %s", peer_ip);

            // Get messages from client and append to file
            while(1){
                nread = recv(acptfd, buf, BUF_SIZE, 0);
                if(nread <= 0){
                    syslog(LOG_ERR, "recv error");
                    break;
                }

                // Write to file
                write(fd, buf, nread);

                if(buf[nread-1] == '\n')
                    break;
            }
            // Wait for writes to disk to complete and close file
            fsync(fd);
            close(fd);

            // Open file
            fd = open(filename, O_RDWR);

            char buf1[1];

            bool transmitting = true;
            while(transmitting){
                // Read file one byte at a time
                unsigned long int cnt = 0;

                while(cnt < BUF_SIZE){
                    if((nread = read(fd, buf1, 1)) <= 0)
                        break;
                    buf[cnt] = buf1[0];
                    cnt ++;
                }

                // Send message back to client
                if(send(acptfd, buf, cnt, 0) == -1)
                    syslog(LOG_ERR, "Error sending message back to client");

                if(nread <= 0)
                    transmitting = false;
            }

            // Close file and peer connection
            close(fd);
            close(acptfd);

            syslog(LOG_DEBUG, "Closed connection from %s", peer_ip);

        }
        syslog(LOG_DEBUG, "Caught signal, exiting");

        while(recv(sockfd, buf, sizeof(buf), 0) > 0){}
        close(sockfd);


        free(buf);
        remove(filename);
        return 0;
    }
    else
        return 0;
}
