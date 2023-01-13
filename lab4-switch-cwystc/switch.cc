//#define DEBUG
#include<cstdio>

#include "switch.h"

#include <stdint.h>
#include<assert.h>
const int Entry_num = 222;

SwitchBase* CreateSwitchObject() {
	// TODO : Your code.
	return new Switch;
}

void Switch::InitSwitch(int numPorts){
	Port_num = numPorts;
	for (int i=0;i<Entry_num;++i){
		table[i].vld = 0;
	}
}

int iden(const char * a, const char * b,int n){
	for (int i=0;i<n;++i)
		if (a[i]!=b[i]) return -1;
	return 0;
}

int Switch::ProcessFrame(int inPort, char* framePtr){
	if (inPort==1){
		for (int i=0;i<Entry_num;++i)
			if (table[i].vld){
				table[i].counter--;
				if (table[i].counter == 0)
					table[i].vld = 0;
			}
		return -1;
	}
	void * src = &(((ether_header_t *)framePtr)->ether_src)[0];
	void * dst = &(((ether_header_t *)framePtr)->ether_dest)[0];
	bool fnd=0;
	for (int i=0;i<Entry_num;++i)
		if (iden((const char *)(&table[i].dst[0]), (const char *)src, 6)==0){
			table[i].counter = 10;
			fnd=1;
		}
	if (!fnd){
		for (int i=0;i<Entry_num;++i)
			if (!table[i].vld){
				table[i].vld = 1;
				table[i].port = inPort;
				memmove(&table[i].dst[0], src, 6);
				table[i].counter = 10;
				fnd=1;
				break;
			}
		assert(fnd);
	}
	
	for (int i=0;i<Entry_num;++i)
		if (table[i].vld && iden((const char *)(&table[i].dst[0]), (const char *)dst, 6)==0){
			if (table[i].port==inPort)
				return -1;
#ifdef DEBUG
			printf("return value %d\n",table[i].port);
			// for (int j=0;j<Entry_num;++j)
			// 	if (table[j].vld){
			// 		for (int k=0;k<6;++k)
			// 			printf("%u ",(uint8_t)table[j].dst[k]);
			// 		printf("   ");
			// 		printf("%d\n",table[j].port);
			// 	}
			printf("gg\n");
			for (int k=0;k<6;++k)
				printf("%u ",*((uint8_t*)(dst+k)));
			printf("\n");
			printf("hh\n");
			for (int k=0;k<6;++k)
				printf("%u ",(uint8_t)(table[i].dst[k]));
			printf("\n");
			printf("%d\n",iden((const char *)(&table[i].dst[0]), (const char *)dst, 6));
#endif
			return table[i].port;
		}
	return 0;
}