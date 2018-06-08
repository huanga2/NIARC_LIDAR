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
    while(1) {
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
        usleep(200000);
    }

    return 0;
}

// Helper function that extends write() to work with const char*
int write_ex(int connfd, char *string) {
    return write(connfd, string, strlen(string));
}

void parse_output(char *input) {
    FILE *outcsv = fopen("out.csv", "w");
    input[strlen(input)-1] = '\0';

    double start_angle;
    double angular_step;
    int num_output;

    int i = 0; // Line count
    char buffer[100];

    while (input != NULL) {
        input++;
        sscanf(input, "%[^ ]", buffer);
        if (i <= 19) {
        }
        else if (i > 19 && i < 26) {
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
                    start_angle = (double) ((int)strtol(buffer, NULL, 16))/10000;
                    printf("Start angle: %2.0f\n", start_angle);
                    break;
                case(24):
                    angular_step = (double) strtol(buffer, NULL, 16)/10000;
                    printf("Angular step: %.2f\n", angular_step);
                    break;
                case(25):
                    num_output = strtol(buffer, NULL, 16);
                    printf("Number of outputs: %d\n", num_output);
                    break;
                default:
                    break;
            }
        }
        else if (i - 26 < num_output) {
            int dist = strtol(buffer, NULL, 16);
            printf("%.2f,%d\n", start_angle+angular_step*(i-26), dist);
            fprintf(outcsv, "%.2f,%d\n", start_angle+angular_step*(i-26), dist);
        }
        else if (i - 26 >= num_output) {
            printf("%s\n", buffer);
        }

        input = strchr(input, ' ');
        i++;
    }

    fclose(outcsv);
}