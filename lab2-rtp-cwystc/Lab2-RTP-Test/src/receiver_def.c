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

static uint32_t ws;
//static int ex = 0;
static char buf[1600]; //!!!
static int SOCKFD;
static char CON[105000000];
static char con[555][1555];
static int conlen[555],CONLEN;
static struct sockaddr_in servaddr, cliaddr;
static int ok[100010];
static int pas;


static void SENDACK(int N){
	rtp_header_t t = Header_con(ACK, 0, N);
	t.checksum = compute_checksum(&t, sizeof(rtp_header_t));
	
	sendto(SOCKFD, &t, sizeof(rtp_header_t), 0, (struct sockaddr *)( &cliaddr), sizeof(struct sockaddr));
}

/**
 * @brief 开启receiver并在所有IP的port端口监听等待连接
 * 
 * @param port receiver监听的port
 * @param window_size window大小
 * @return -1表示连接失败，0表示连接成功
 */
int initReceiver(uint16_t port, uint32_t window_size){
//	if (ex) return -1;
	ws = window_size;
	

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sockfd > 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons((uint16_t)port);
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))!=0){
		printf("bind failed!!!!\n");
		assert(0);
	}
	
	
	SOCKFD = sockfd;
	
	setnonblocking(sockfd);
	
	clock_t start = clock();
	clock_t end;
	while (1){
	//	printf("receiver\n");
		end = clock();
		if ((double)(end-start) / CLOCKS_PER_SEC>=10){
			printf("initReceiver failed!!!\n");
			return -1;
		}
		int l = sizeof(struct sockaddr_in);
		int len = recvfrom(sockfd, buf, 1500, 0, (struct sockaddr *)(&cliaddr), (socklen_t *)(&l));
		if (len<=0) continue;
		
//		printf("receiver\n");
		
		if (corrupted(buf,len)){
			
			return -1;
		}
		
		
		rtp_header_t t;
		memcpy(&t, buf, sizeof(rtp_header_t));
		if (t.type == END){
//			printf("receive a packet with type END\n");
			return -1;
		}
		if (t.type == START){
			SENDACK(t.seq_num);
//			printf("initReceiver successfully!!!\n");
			return 0;
		}
		
	}
	
}

/**
 * @brief 用于接收数据并在接收完后断开RTP连接
 * @param filename 用于接收数据的文件名
 * @return >0表示接收完成后到数据的字节数 -1表示出现其他错误
 */
int recvMessage(char* filename){

//	printf("run into recvMessage!!!\n");
	
//	if (ex) return -1;
	FILE * f = fopen(filename, "wb");
	if (f == NULL) assert(0);
	int N=0;
	clock_t begin = clock();
	clock_t end;
	int CONLEN=0;
	while (1){
		
//		printf("receiver\n");
		
		end=clock();
		if ((double)(end-begin) / CLOCKS_PER_SEC>=10)
			return -1;
		int l = sizeof(struct sockaddr_in);
		int len = recvfrom(SOCKFD, buf, 1500, 0, (struct sockaddr *)(&cliaddr), (socklen_t *)(&l));
		if (len>0){
			if (corrupted(buf,len) == 0){
				rtp_header_t t;
				memcpy(&t, buf, sizeof(rtp_header_t));
				if (t.type == END){
					SENDACK(N);
					break;
				}
				
			//	printf("receive pack %d !!!\n",t.seq_num);
				
				if (t.seq_num>=N && t.seq_num < N + ws){
					memcpy(con[t.seq_num%ws],buf+sizeof(rtp_header_t),t.length);
					conlen[t.seq_num%ws] = t.length;
					ok[t.seq_num]=1;
					while (ok[N]){
						memcpy(CON+CONLEN,con[N%ws],conlen[N%ws]);
						CONLEN += conlen[N%ws];
						++N;
					}
				}
				
				SENDACK(N);
			}
			begin = clock();
		}
	}
	
	fwrite(CON, 1, CONLEN, f);
	fclose(f);
	
	printf("recvMessage successfully!!!\n");
	
	return 0;
}

/**
 * @brief 用于接收数据并在接收完后断开RTP连接 (优化版本的RTP)
 * @param filename 用于接收数据的文件名
 * @return >0表示接收完成后到数据的字节数 -1表示出现其他错误
 */
int recvMessageOpt(char* filename){
//	printf("run into recvMessageOpt!!!\n");
	
//	if (ex) return -1;
	FILE * f = fopen(filename, "wb");
	if (f == NULL) assert(0);
	int N=0;
	clock_t begin = clock();
	clock_t end;
	int CONLEN=0;
	while (1){
		
//		printf("receiver\n");
		
		end=clock();
		if ((double)(end-begin) / CLOCKS_PER_SEC>=10)
			return -1;
		int l = sizeof(struct sockaddr_in);
		int len = recvfrom(SOCKFD, buf, 1500, 0, (struct sockaddr *)(&cliaddr), (socklen_t *)(&l));
		if (len>0){
			if (corrupted(buf,len) == 0){
				rtp_header_t t;
				memcpy(&t, buf, sizeof(rtp_header_t));
				
				SENDACK(t.seq_num);
				
				if (t.type == END){
					break;
				}
				
			//	printf("receive pack %d !!!\n",t.seq_num);
				
				if (t.seq_num>=N && t.seq_num < N + ws){
					memcpy(con[t.seq_num%ws],buf+sizeof(rtp_header_t),t.length);
					conlen[t.seq_num%ws] = t.length;
					ok[t.seq_num]=1;
					while (ok[N]){
						memcpy(CON+CONLEN,con[N%ws],conlen[N%ws]);
						CONLEN += conlen[N%ws];
						++N;
					}
				}
				
			}
			begin = clock();
		}
	}
	
	fwrite(CON, 1, CONLEN, f);
	fclose(f);
	
	printf("recvMessageOpt successfully!!!\n");
	
	return 0;

}

/**
 * @brief 用于接收数据失败时断开RTP连接以及关闭UDP socket
 */
void terminateReceiver(){
	printf("run into terminateReceiver!!!\n");
	
//	if (ex) return;
	assert(SOCKFD>0);
	close(SOCKFD);
}
