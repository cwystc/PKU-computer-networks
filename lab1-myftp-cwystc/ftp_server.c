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
char * SERV_IP;
int state;
int listenfd;
int connfd;
int SERV_PORT;
struct sockaddr_in cliaddr, servaddr;

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
			printf("send error!!!\n");
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
			printf("recv error!!!\n");
			assert(0);
		}
		if (b == 0){
			printf("Client has closed connection\n");
			return ret;
		}
		ret += b;
	}
	return ret;
}

void doit(){
	state = IDLE;
	
	
	while (1){
		if (state == IDLE){
			if (Recv(connfd, buf, 12)<12){
			//	printf("Why Client connection closed???\n");
				close(connfd);
				return;
			}
			state=OpenConnection;
			continue;
		}
		if (state == OpenConnection){
			struct Header t = Header_con(0xA2, 1, 12);
			Send(connfd, (char *)(&t), 12);
			state = Authentication;
			continue;
		}
		if (state == Authentication){
			if (Recv(connfd, buf, 12)<12){
			//	printf("Why Client connection closed???\n");
				close(connfd);
				return;
			}
			uint32_t len = ntohl(*((uint32_t*)(buf+8)));
			assert(len>=13);
			len-=12;
			if (Recv(connfd, buf, (int)len)< (int)len){
			//	printf("Why Client connection closed???\n");
				close(connfd);
				return;
			}
			int ok;
			if (strcmp(buf,"user 123123")==0)
				ok=1;
			else
				ok=0;
			struct Header t = Header_con(0xA4, ok, 12);
			Send(connfd, (char *)(&t), 12);
			if (ok==0){
				close(connfd);
				return;
			}
			state = MAIN;
		}
		if (state == MAIN){
			if (Recv(connfd, buf, 12)<12){
			//	printf("Why Client connection closed???\n");
				close(connfd);
				return;
			}
			
			if (((unsigned char)buf[6])==0xA5){
				FILE * f = popen("ls -1", "r");
				int len = (int)fread(buf, 1, MAXLEN, f);
				pclose(f);
				struct Header t = Header_con(0xA6, 0, 12 + len + 1);
				Send(connfd, (char *)(&t), 12);
				buf[len]=0;
				Send(connfd, buf, len+1);
				continue;
			}
			if (((unsigned char)buf[6])==0xA7){
				uint32_t len = ntohl(*((uint32_t*)(buf+8)));
				assert(len>=13);
				len-=12;
				if (Recv(connfd, buf, (int)len)<(int)len){
				//	printf("Why Client connection closed???\n");
					close(connfd);
					return;
				}
				int ok;
				if (access(buf,R_OK)!=0)
					ok=0;
				else
					ok=1;
				struct Header t = Header_con(0xA8, ok, 12 );
				Send(connfd, (char *)(&t), 12);
				if (ok==0) continue;
				
				FILE * f = fopen(buf, "rb");
				len = fread(buf, 1, MAXLEN, f);
				fclose(f);
				
				t=Header_con(0xFF, 0, 12 + (int)len);
				Send(connfd, (char *)(&t), 12);
				Send(connfd, buf, (int)len);
				continue;
			}
			if (((unsigned char)buf[6])==0xA9){
				uint32_t len = ntohl(*((uint32_t*)(buf+8)));
				assert(len>=13);
				len-=12;
				if (Recv(connfd, buf, (int)len)<(int)len){
				//	printf("Why Client connection closed???\n");
					close(connfd);
					return;
				}
				struct Header t = Header_con(0xAA, 0, 12 );
				Send(connfd, (char *)(&t), 12);
				FILE * f = fopen(buf, "wb");
				
				if (Recv(connfd, buf, 12)<12){
				//	printf("Why Client connection closed???\n");
					close(connfd);
					return;
				}
				
				len = ntohl(*((uint32_t*)(buf+8)));
				assert(len>=12);
				len-=12;
				if (Recv(connfd, buf, len)<len){
				//	printf("Why Client connection closed???\n");
					close(connfd);
					return;
				}
				fwrite(buf, 1, len, f);
				fclose(f);
				continue;
			}
			if (((int)s[6])==0xAB){
				struct Header t=Header_con(0xAC, 0, 12);
				Send(connfd, (char *)(&t), 12);
				close(connfd);
				return;
			}
			assert(0);
		}
	}
}
int main(int argc, char ** argv) {
	if (argc!=3){
		printf("format: ./ftp_server IP Port\n");
		return 0;
	}
	SERV_PORT=(int)strtol(argv[2], (char **)NULL, 10);
	SERV_IP=(char *)argv[1];
	
	printf("%d\n",SERV_PORT);
	printf("%s\n",SERV_IP);
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons((uint16_t)SERV_PORT);
	inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	listen(listenfd, 1024);
	while (1){
		int t = (int)sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &t);
		doit();
	}
	
	

    return 0;
}

