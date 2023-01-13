#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#define IDLE 0
#define OpenConnection 1
#define Authentication 2
#define MAIN 3
#define MAGIC_NUMBER_LENGTH 6
#define MAXLEN 2000000 //!!!
typedef char byte;
typedef char type;
typedef char status;

char s[2111111]; //!!!
char buf[2111111];
char buf1[2111111];
char * SERV_IP;
int state;
int sockfd;
int SERV_PORT;
struct sockaddr_in servaddr;


struct Header{
    byte m_protocol[MAGIC_NUMBER_LENGTH]; /* protocol magic number (6 bytes) */
    type m_type;                          /* type (1 byte) */
    status m_status;                      /* status (1 byte) */
    uint32_t m_length;                    /* length (4 bytes) in Big endian*/
} __attribute__ ((packed));

struct Header Header_con(int tp, int stat, uint32_t len){
	struct Header t;
	memcpy(t.m_protocol,"\xe3myftp",6);
	t.m_type=(byte)tp;
	t.m_status=(byte)stat;
	t.m_length = htonl(len);
	return t;
}

void Send(int sockfd, char * buf, int len){
	size_t ret = 0;
	while (ret < len) {
		size_t b = send(sockfd, buf + ret, len - ret, 0);
		if (b < 0){
			printf("send error!!!");
			assert(0);
		}
	//	if (b == 0) printf("socket Closed");
		ret += b;
	}
}

int Recv(int sockfd, char * buf, int len){
	size_t ret = 0;
	while (ret < len) {
		size_t b = recv(sockfd, buf + ret, len - ret, 0);
		if (b < 0){
			printf("recv error!!!");
			assert(0);
		}
		if (b == 0){
			printf("Server has closed connection");
			return ret;
		}
		ret += b;
	}
	return ret;
}

int main(int argc, char ** argv) {
	state = IDLE;
	
	while (1){
		if (state == IDLE){
			fgets(s, MAXLEN, stdin);
			int len = strlen(s);
			if (len<8){
				printf("format: open SERVER_IP SERVER_PORT\n");
				continue;
			}
			s[len-1]=0;
			--len;
			if (strncmp(s,"open ",5)!=0){
				printf("format: open SERVER_IP SERVER_PORT\n");
				continue;
			}
			int sp = -1;
			for (int i=5;i<len;++i)
				if (s[i]==' '){
					sp = i;
					break;
				}
			if (sp==-1){
				printf("format: open SERVER_IP SERVER_PORT\n");
				continue;
			}
			SERV_IP = s+5;
			s[sp]=0;
			if (sp+1>len-1){
				printf("format: open SERVER_IP SERVER_PORT\n");
				continue;
			}
			int gg=0;
			for (int i=sp+1;i<len;++i)
				if (s[i]<'0' || s[i]>'9') gg=1;
			if (gg){
				printf("format: open SERVER_IP SERVER_PORT\n");
				continue;
			}
			SERV_PORT=(int)strtol(s+sp+1, (char **)NULL, 10);
			
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&servaddr, sizeof(struct sockaddr_in));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons((uint16_t)SERV_PORT);
			inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);
			if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
				printf("Server is OFF\n");
				continue;
			}
			struct Header t=Header_con(0xA1,0,12);
			Send(sockfd, (char *)&t, 12);
			state=OpenConnection;
			continue;
		}
		if (state == OpenConnection){
			if (Recv(sockfd, buf, 12)<12){
				state  = IDLE;
				printf("Server is OFF\n");
				continue;
			}
			state = Authentication;
			printf("Server connection accepted.\n");
			continue;
		}
		if (state == Authentication){
			fgets(s, MAXLEN, stdin);
			int len=strlen(s);
			if (len<8){
				printf("format: auth USER PASS\n");
				continue;
			}
			s[len-1]=0;
			--len;
			if (strncmp(s,"auth ",5)!=0){
				printf("format: auth USER PASS\n");
				continue;
			}
			int sp = -1;
			int k=0;
			for (int i=5;i<len;++i)
				if (s[i]==' '){
					sp = i;
					++k;
					break;
				}
			if (k!=1){
				printf("format: open SERVER_IP SERVER_PORT\n");
				continue;
			}
			struct Header t=Header_con(0xA3, 0, 12+(len-5)+1);
			Send(sockfd, (char *)(&t), 12);
			Send(sockfd, s+5, (len-5)+1);
			if (Recv(sockfd, buf, 12)<12){
				state  = IDLE;
				printf("Why Server connection closed???\n");
				continue;
			}
			if (buf[7]==0){
				printf("Authentication rejected\n");
				state  = IDLE;
				continue;
			}
			if (buf[7]==1){
				printf("Authentication granted\n");
				state = MAIN;
				continue;
			}
			assert(0);
		}
		if (state == MAIN){
			fgets(s, MAXLEN, stdin);
			int len=strlen(s);
			s[len-1]=0;
			--len;
			if (strcmp(s, "ls")==0){
				struct Header t=Header_con(0xA5, 0, 12);
				Send(sockfd, (char *)(&t), 12);
				if (Recv(sockfd, buf, 12)<12){
					state  = IDLE;
					printf("Why Server connection closed???\n");
					continue;
				}
				buf[12]=0;
				uint32_t len1 = ntohl(*((uint32_t*)(buf+8)));
				assert(len1>=13);
				if (Recv(sockfd, buf, len1-12)<len1-12){
					state  = IDLE;
					printf("Why Server connection closed???\n");
					continue;
				}
				printf("----- file list start -----\n");
				printf("%s",buf);
				printf("----- file list end -----\n");
				continue;
			}
			if (len>=4 && strncmp(s, "get ", 4)==0){
				//s[4]~s[len-1]
				struct Header t=Header_con(0xA7, 0, 12 + (len-4) + 1);
				Send(sockfd, (char *)(&t), 12);
				Send(sockfd, s+4, (len-4) + 1);
				if (Recv(sockfd, buf, 12)<12){
					state  = IDLE;
					printf("Why Server connection closed???\n");
					continue;
				}
				if (buf[7]==0){
					printf("File does not exist\n");
					continue;
				}
				if (buf[7]==1){
					printf("File exists\n");
					if (Recv(sockfd, buf, 12)<12){
						state  = IDLE;
						printf("Why Server connection closed???\n");
						continue;
					}
					uint32_t len1 = ntohl(*((uint32_t*)(buf+8)));
					assert(len1>=12);
					len1-=12;
					if (Recv(sockfd, buf, len1)<len1){
						state  = IDLE;
						printf("Why Server connection closed???\n");
						continue;
					}
					buf[len1]=0;
					FILE * f = fopen(s+4, "wb");
					/*
					for (int i=0;i<len1;++i)
						fprintf(f, "%c",buf[i]);
					*/
					fwrite(buf, 1, len1, f);
					fclose(f);
					printf("File downloaded\n");
					continue;
				}
				assert(0);
			}
			if (len>=4 && strncmp(s, "put ", 4)==0){
				//s[4]~s[len-1]
				if (access(s+4,R_OK)!=0){
					printf("File does not exist\n");
					continue;
				}
				printf("File exists\n");
				FILE * f = fopen(s+4, "rb");
				uint32_t len1 = fread(buf, 1, MAXLEN, f);
				fclose(f);
				
				struct Header t=Header_con(0xA9, 0, 12 + (len-4) + 1);
				Send(sockfd, (char *)(&t), 12);
				Send(sockfd, s+4, (len-4) + 1);
				if (Recv(sockfd, buf1, 12)<12){
					state  = IDLE;
					printf("Why Server connection closed???\n");
					continue;
				}
				t=Header_con(0xFF, 0, 12 + (int)len1);
				Send(sockfd, (char *)(&t), 12);
				Send(sockfd, buf, (int)len1);
				printf("File uploaded\n");
				continue;
			}
			if (strcmp(s, "quit") ==0){
				struct Header t=Header_con(0xAB, 0, 12);
				Send(sockfd, (char *)(&t), 12);
				if (Recv(sockfd, buf, 12)<12){
				}
				close(sockfd);
				printf("Thank you\n");
				break;
			}
			printf("format: ls | get FILE | put FILE | QUIT_REQUEST\n");
			continue;
		}
	}
	
    return 0;
}


