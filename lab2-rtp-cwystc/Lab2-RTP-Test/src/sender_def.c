#include <stdint.h>
#include <sys/types.h>
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
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include "rtp.h"
#include "util.h"

#define START 0
#define END 1
#define DATA 2
#define ACK 3

static void Perror(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

static void setnonblocking(int sockfd) {
    int flag = fcntl(sockfd, F_GETFL, 0);
    if (flag < 0) {
        Perror("fcntl F_GETFL fail");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) {
        Perror("fcntl F_SETFL fail");
    }
}

static rtp_header_t Header_con(uint8_t tp, uint16_t len, uint32_t seq_num){
	rtp_header_t t;
	t.type = tp;
	t.length = len;
	t.seq_num = seq_num;
	t.checksum = 0;
	return t;
}

static int corrupted(char * buf, int len){
	uint32_t sum = (((rtp_header_t * )buf)->checksum);
	(((rtp_header_t * )buf)->checksum)=0;
	
	if (compute_checksum(buf, len) != sum)
		return 1;
	else
		return 0;
}
//end ...

#define MAXLEN 105000000
static char con[MAXLEN+10];
//static int ex = 0;
static char buf[1600]; //!!!
static uint32_t ws;
static int SOCKFD;
static int conlen;
static int packet[100010];
static struct sockaddr_in servaddr;
static int pas;
static int ok[100010];

static void SENDEND(){
	rtp_header_t t = Header_con(END, 0, pas);
	t.checksum = compute_checksum(&t, sizeof(rtp_header_t));
	sendto(SOCKFD, &t, sizeof(rtp_header_t), 0, (struct sockaddr *)( &servaddr), sizeof(struct sockaddr));
}

static void sendpac(int n){
	rtp_header_t t = Header_con(DATA, packet[n+1] - packet[n], n);
	memcpy(buf, &t, sizeof(rtp_header_t));
	memcpy(buf + sizeof(rtp_header_t), con+packet[n], t.length);
	t.checksum = compute_checksum(buf, sizeof(rtp_header_t) + t.length);
	((rtp_header_t *)buf) -> checksum = t.checksum;
	sendto(SOCKFD, buf, sizeof(rtp_header_t) + t.length, 0, (struct sockaddr *)( &servaddr), sizeof(struct sockaddr));
	
//	printf("send pac %d\n",n);
}

/**
 * @brief 用于建立RTP连接
 * @param receiver_ip receiver的IP地址
 * @param receiver_port receiver的端口
 * @param window_size window大小
 * @return -1表示连接失败，0表示连接成功
 **/
int initSender(const char* receiver_ip, uint16_t receiver_port, uint32_t window_size){
	
//	if (ex) return -1;
	
	printf("hahaha\n");
	
	ws = window_size;

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons((uint16_t)receiver_port);
	inet_pton(AF_INET, receiver_ip, &servaddr.sin_addr);
	
	SOCKFD = sockfd;
	
	printf("ggg\n");
	
	rtp_header_t t = Header_con(START, 0, 19260817);
	t.checksum = compute_checksum(&t, sizeof(rtp_header_t));
	sendto(sockfd, &t, sizeof(rtp_header_t), 0, (struct sockaddr *)( &servaddr), sizeof(struct sockaddr));
	
	printf("hhh\n");
	
	setnonblocking(sockfd);
	
	clock_t start = clock();
	clock_t end;
	while (1){
//		printf("sender\n");
		end = clock();
		if ((double)(end-start) / CLOCKS_PER_SEC>=10){
			SENDEND();
			return -1;
		}
		int len = recvfrom(sockfd, buf, 1550, 0, NULL, NULL);
		if (len<=0) continue;
		if (corrupted(buf,len)){
			SENDEND();
			return -1;
		}
		if ((((rtp_header_t*)buf)->seq_num) == 19260817 && (((rtp_header_t*)buf)->type)==ACK){
			printf("initSender successfully!!!\n");
			return 0;
		}
	}
	
}


/**
 * @brief 用于发送数据 
 * @param message 要发送的文件名
 * @return -1表示发送失败，0表示发送成功
 **/
int sendMessage(const char* filename){
	
//	if (ex) return -1;
	FILE * f = fopen(filename, "rb");
	if (f == NULL) assert(0);
	conlen = fread(con, 1, MAXLEN, f);
	fclose(f);
	pas = 0;
	for (int i=0;i<conlen;i+=1450){
		packet[pas++]=i;
	}
	packet[pas]=conlen;
	
	printf("the size of file is %d bytes.\n",conlen);
	
	int base=0;
	clock_t begin,end;
	while (base<pas){
		
//		printf("now the base of sender is %d\n",base);
		
		begin = clock();
		for (int i=base;i<base+ws;++i){
			if (i>=pas) break;
			sendpac(i);
		}
		//time is up, start timer and retransmit all the packets in the window
		while (1){
			int len = recvfrom(SOCKFD, buf, 1500, 0, NULL, NULL);
			if (len>0){
				if (corrupted(buf,len) == 0){
					if ( ((rtp_header_t *)buf)->type==ACK && ((rtp_header_t *)buf)->seq_num > base){
						base = ((rtp_header_t *)buf)->seq_num;
						
					//	printf("now the base of sender is %d !!!\n",base);
						/*
						assert(base<=pas);
						if (base == pas)
							break;
						begin = clock();
						*/
						break;
					}
				}
			}
			end = clock();
			if ((double)(end-begin) / CLOCKS_PER_SEC>=0.1){
				
				break;
			}
		}
	}
	
	printf("sendMessage successfully!!!\n");
	
	return 0;
		
}



/**
 * @brief 用于发送数据 (优化版本的RTP)
 * @param message 要发送的文件名
 * @return -1表示发送失败，0表示发送成功
 **/
int sendMessageOpt(const char* filename){
//	if (ex) return -1;
	FILE * f = fopen(filename, "rb");
	if (f == NULL) assert(0);
	conlen = fread(con, 1, MAXLEN, f);
	fclose(f);
	pas = 0;
	for (int i=0;i<conlen;i+=1450){
		packet[pas++]=i;
	}
	packet[pas]=conlen;
	
	printf("the size of file is %d bytes.\n",conlen);
	
	int base=0;
	clock_t begin,end;
	while (base<pas){
		
//		printf("now the base of sender is %d\n",base);
		
		begin = clock();
		for (int i=base;i<base+ws;++i){
			if (i>=pas) break;
			if (ok[i]==0)
				sendpac(i);
		}
		//time is up, start timer and retransmit all the packets in the window
		while (1){
			int len = recvfrom(SOCKFD, buf, 1500, 0, NULL, NULL);
			if (len>0){
				if (corrupted(buf,len) == 0){
					if ( ((rtp_header_t *)buf)->type==ACK){
						ok[((rtp_header_t *)buf)->seq_num]=1;
						
						if (ok[base]){
							while (ok[base] && base<pas) ++base;
							
							break;
						}
					}
				}
			}
			end = clock();
			if ((double)(end-begin) / CLOCKS_PER_SEC>=0.1){
				
				break;
			}
		}
	}
	
	printf("sendMessage successfully!!!\n");
	
	return 0;

}


/**
 * @brief 用于断开RTP连接以及关闭UDP socket
 **/
void terminateSender(){
	assert(SOCKFD>0);
//	if (ex) return;
	SENDEND();
	clock_t start = clock();
	clock_t end;
	while (1){
		end = clock();
		if ((double)(end-start) / CLOCKS_PER_SEC>=10){
			close(SOCKFD);
			return;
		}
		int len = recvfrom(SOCKFD, buf, 1500, 0, NULL, NULL);
		if (len<=0) continue;
		if (corrupted(buf,len)) continue;
		rtp_header_t t;
		memcpy(&t, buf, sizeof(rtp_header_t));
		if (t.seq_num == pas && t.type==ACK){
			close(SOCKFD);
			return;
		}
	}
	
	
}
