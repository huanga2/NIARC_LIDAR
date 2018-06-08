#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>

#define LIDAR_IP "192.168.0.209"
#define LIDAR_PORT_A 2112

int write_ex(int connfd, char *string);
void parse_output(char *input);

int main(int argc, char **argv) {
    // Address settings
    struct sockaddr_in lidar_addr;
    memset(&lidar_addr, 0, sizeof(lidar_addr));
    lidar_addr.sin_family = AF_INET;                      // IPv4
    lidar_addr.sin_addr.s_addr = inet_addr(LIDAR_IP);
    lidar_addr.sin_port = htons(LIDAR_PORT_A);            // Commandline Port

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
	{
		perror("ERROR opening socket");
		exit(0);
	}

    if (connect(sockfd,(struct sockaddr *)&lidar_addr,sizeof(lidar_addr)) < 0) 
	{
		perror("ERROR connecting");
		exit(0);
	}

    // write(sockfd, "\x02\x02\x02\x02\x00\x00\x00\x0F\x73\x52\x4E\x20\x4C\x4D\x44\x73\x63\x61\x6E\x64\x61\x74\x61\x05", 25);
    write_ex(sockfd, "\02sRN LMDscandata\03");

    char buf[8192];

    int n;
    while((n = read(sockfd, buf, 8192))) {
        // printf("%s", buf);
        // fprintf(file, "%s", buf);
        // printf("\n%d\n", n);
        // fwrite(buf, 100, 1, file);
        if (buf[n-1] == '\03') {
            break;
        }
    }
    parse_output(buf);

    return 0;
}

// Helper function that extends write() to work with const char*
int write_ex(int connfd, char *string) {
    return write(connfd, string, strlen(string));
}

void parse_output(char *input) {
    input[strlen(input)-1] = '\0';

    int start_angle;
    int angular_step;
    int num_output;

    int i = 0; // Line count
    char buffer[100];

    while (input != NULL) {
        input++;
        sscanf(input, "%[^ ]", buffer);
        if (i > 19 && i < 26) {
            switch(i) {
                case(20):
                    printf("%s\n", buffer);
                    break;
                case(21):
                    printf("Scale factor:");
                    if (strcmp(buffer, "3F800000") == 0) {
                        printf(" 1\n");
                    }else {
                        printf(" 2\n");
                    }
                    break;
                case(22):
                    printf("Scale factor offset: 0\n");
                    break;
                case(23):
                    start_angle = strtol(buffer, NULL, 16);
                    printf("Start angle: %2.0f\n", (double) 1.0*start_angle/10000);
                    break;
                case(24):
                    angular_step = strtol(buffer, NULL, 16);
                    printf("Angular step: %.2f\n", (double) 1.0*angular_step/10000);
                    break;
                case(25):
                    num_output = strtol(buffer, NULL, 16);
                    printf("Number of outputs: %d\n", num_output);
                    break;
                default:
                    break;
            }
        }

        input = strchr(input, ' ');
        i++;
    }
}