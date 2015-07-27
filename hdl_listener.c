#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#define MYPORT "6000" // the port users will be connecting to

#define MAXBUFLEN 512

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void dump_buffer(unsigned n, const unsigned char* buf)
{
	const unsigned char *p, *end;
	unsigned i, j;

	end = buf + n;

	for (i = 0; ; i += 16) {
		p = buf + i;
		for (j = 0; j < 16; j++) {
			fprintf(stderr, "%02X ", p[j]);
			if (p + j >= end)
			goto BREAKOUT;
		}
		fprintf(stderr, " ");
		p = buf + i;
		for (j = 0; j < 16; j++) {
			fprintf(stderr, "%c", isprint(p[j]) ? p[j] :
			'.');
			if (p + j >= end)
			goto BREAKOUT;
		}
		fprintf(stderr, "\n");
	}

	BREAKOUT:
	return;
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	fd_set master,read_flags;
	struct timeval waitd;

	// HDL variables
	char command_code[2];
	char device_id[1];
	char channel_id[1];
	char current_val[1];

	// HDL monitoring device
	char mon_command_code[] = {0x00, 0x31};
	char mon_device_id[] = {0x5B}; // 91
	char mon_channel_id[] = {0x05}; // 5
	char mon_current_val[] = {0x00};

	// answerback buffer
	char answerback_buf[100];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	// set socket to non blocking
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	// non blocking sets
	FD_ZERO(&read_flags);
	FD_ZERO(&master);
	FD_SET(sockfd, &master);

	printf("listener: waiting to recvfrom...\n");

	while (1) {
		read_flags = master;

		waitd.tv_sec = 1;  // Make select wait up to 1 second for data
		waitd.tv_usec = 0; // and 0 milliseconds.

		if (select(sockfd+1, &read_flags, NULL, NULL, &waitd) < 0) {
			printf("socket select error\n");
			exit(1);
		}

		// Check if data is available to read
		if (FD_ISSET(sockfd, &read_flags)) {

			// flush internal buffer
			memset(&buf, 0, sizeof(char) * MAXBUFLEN);

			if ((numbytes = recv(sockfd, buf, MAXBUFLEN-1, 0)) > 0) {
				//printf("\n\n");
				//dump_buffer(numbytes-1, buf);
				//printf("\n\n");

				memcpy(command_code, &buf[21], 2);
				memcpy(device_id, &buf[24], 1);
				memcpy(channel_id, &buf[25], 1);
				memcpy(mon_current_val, &buf[26], 1);

				//dump_buffer(1, command_code);
				//dump_buffer(0, device_id);
				//dump_buffer(0, channel_id);
				//dump_buffer(0, mon_current_val);

				if (memcmp(&command_code, &mon_command_code, 2) == 0) {
					if (memcmp(&device_id, &mon_device_id, 1) == 0) {
						if (memcmp(&channel_id, &mon_channel_id, 1) == 0) {

							memset(&answerback_buf, 0, sizeof(char) * 100);
							sprintf (answerback_buf, "/root/ligth_answer 192.168.10.255 %d", mon_current_val[0]);
							system(answerback_buf);
							//printf("%s\n", answerback_buf);

							if (memcmp(&mon_current_val, &current_val, 1) != 0){
								current_val[0] = mon_current_val[0];
								printf("jopaixep\n");	
								fflush(stdout);
							}
						}
					}
				}
			}
			FD_CLR(sockfd, &read_flags);
		}
	}

	// exit
	close(sockfd);

	return 0;
}
