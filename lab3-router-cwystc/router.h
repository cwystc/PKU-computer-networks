#include "router_prototype.h"
#include <stdint.h>
#include <cstdlib>
#include <utility>

struct header_t{
    uint32_t src;
    uint32_t dst;
    uint8_t type;
    uint16_t length;
};

struct DV{
    int key;
    int too_num;
    std::pair<uint32_t,int> too[610];
    std::pair<uint32_t,int> dis[610];
    //dis.second: key of next hop | -port
    DV(){
        too_num = 0;
    }
    bool operator == (const DV & A) const{
        if (too_num!=A.too_num)
            return 0;
        for (int i=0;i<too_num;++i){
            if (too[i]!=A.too[i])
                return 0;
            if (dis[i]!=A.dis[i])
                return 0;
        }
        return 1;
    }
};

class Router : public RouterBase {
private:
    int key;
    int Key[610];

	int Port_num;
	int External_port;
    uint32_t External_address;
    int External_address_b;
    uint32_t Available_address;
    int Available_address_b;

    bool ishost[610];
    uint32_t hostaddr[610];
    uint32_t mp[256]; //available to inner IP
    int len[610]; // length of the edge at port i 

    DV dv[410];

public:
    Router(){
        key = rand();
    }
    void router_init(int port_num, int external_port, char* external_addr, char* available_addr);
    int router(int in_port, char* packet);
    void calc();
};
