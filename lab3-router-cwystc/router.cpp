//#define DEBUG1


#include "router.h"
#include<string.h>
#include<assert.h>

#include<map>
#include<cstdlib>
#include<utility>
#include<iostream>
#define TYPE_DV 0
#define TYPE_DATA 1
#define TYPE_CONTROL 2

void pr(const DV & t){
    std::cout<<t.key<<"\n";
    std::cout<<t.too_num<<"\n";
    for (int i=0;i<t.too_num;++i){
        std::cout<<t.too[i].first<<' '<<t.too[i].second<<' '<<t.dis[i].first<<' '<<t.dis[i].second<<"\n";
    }
}

RouterBase* create_router_object() {
    return new Router;
}

void Router::router_init(int port_num, int external_port, char* external_addr, char* available_addr) {
	Port_num = port_num;
    for (int i=1;i<=Port_num;++i)
        ishost[i]=0,len[i]=-1;
    External_port = external_port;
    if (external_addr!=NULL){
        int l = strlen(external_addr);
        unsigned int tmp = 0;
        unsigned int t = 0;
        for (int i=0;i<l;++i)
            if (external_addr[i] == '.' || external_addr[i] == '/'){
                t = t*256+tmp;
                tmp=0;
            }
            else
                tmp = tmp * 10+external_addr[i]-48;
        tmp = 32-tmp;
        t = (t>>tmp)<<tmp;
        External_address = t;
        External_address_b = tmp;
    }
    else
        External_address_b = -1;
    
    if (available_addr!=NULL){
        int l = strlen(available_addr);
        unsigned int tmp = 0;
        unsigned int t = 0;
        for (int i=0;i<l;++i)
            if (available_addr[i] == '.' || available_addr[i] == '/'){
                t = t*256+tmp;
                tmp=0;
            }
            else
                tmp = tmp * 10+available_addr[i]-48;
        tmp = 32-tmp;
        t = (t>>tmp)<<tmp;
        Available_address = t;
        Available_address_b = tmp;
        for (int i=0;i<(1<<tmp);++i)
            mp[i]=0;
    }
    else
        Available_address_b = -1;
    

    return;
}

uint32_t u322big(uint32_t t){
    uint32_t res;
    *((unsigned char *)(&res)) = t>>24;
    *((unsigned char *)(&res) + 1) = (t>>16)%(1<<8);
    *((unsigned char *)(&res) + 2) = (t>>8)%(1<<8);
    *((unsigned char *)(&res) + 3) = (t)%(1<<8);
    return res;
}

uint32_t big2u32(uint32_t res){
    uint32_t t = 0;
    t = (t<<8)|(*((unsigned char *)(&res)));
    t = (t<<8)|(*((unsigned char *)(&res) + 1));
    t = (t<<8)|(*((unsigned char *)(&res) + 2));
    t = (t<<8)|(*((unsigned char *)(&res) + 3));
    return t;
}

uint32_t st2u32(char * p){
    uint32_t res = 0;
    uint32_t t=0;
    int cnt=0;
    for (int i=0;;++i){
        if (p[i]<'0' || p[i]>'9'){
            res=res*256+t;
            t=0;
            ++cnt;
            if (cnt==4) break;
        }
        else{
            t=t*10+p[i]-'0';
        }
    }
    return res;
}

struct header_t mkheader(uint32_t src, uint32_t dst, uint8_t tp, uint16_t len){
    struct header_t t;
    t.src = u322big(src);
    t.dst = u322big(dst);
    t.type = tp;
    t.length=len;
    return t;
}

void Router::calc(){
    std::map<std::pair<uint32_t,int> , std::pair<uint32_t,int> > M;
    M.clear();
    if (External_address_b!=-1)
        M[std::make_pair(External_address, External_address_b)] = std::make_pair((uint32_t)0, -External_port);
    for (int i = 1;i<=Port_num;++i)
        if (ishost[i])
            M[std::make_pair(hostaddr[i],(int)0)] = std::make_pair((uint32_t)0,-i);
    for (int i=1;i<=Port_num;++i){
        if (len[i]==-1) continue;
        for (int j=0;j<dv[i].too_num;++j){
            std::pair<uint32_t,int> TOO = dv[i].too[j];
            std::pair<uint32_t,int> DIS = dv[i].dis[j];

            if (DIS.second == key)
                continue;
            //poisoned reverse

            if (M.find(TOO)==M.end() )
                M[TOO]=std::make_pair((uint32_t)(DIS.first+len[i]),Key[i]);
            else{
                std::pair<uint32_t,int> tmp = M[TOO];
                if (DIS.first + len[i]<tmp.first){
                    M[TOO] = std::make_pair((uint32_t)(DIS.first+len[i]),Key[i]);
                }
            }
        }
    }
    DV tmp;
    tmp.key = key;
    for (std::map<std::pair<uint32_t,int> , std::pair<uint32_t,int> >::iterator it = M.begin();it!=M.end();++it){
        std::pair<uint32_t,int> TOO = (it->first);
        std::pair<uint32_t,int> DIS = (it->second);
        tmp.too[tmp.too_num] = TOO;
        tmp.dis[tmp.too_num] = DIS;
        ++tmp.too_num;
    }
    dv[0]=tmp;
}

int Router::router(int in_port, char* packet) {

    int tp = ((header_t * )packet) -> type;

    uint32_t d = big2u32(((header_t *)packet)->dst);
    uint32_t s = big2u32(((header_t *)packet)->src);

    if (in_port == External_port){
        bool ok = 0;
        if (Available_address_b!=-1 && d>=Available_address && d<=Available_address + ((1u<<Available_address_b)-1)){
            if (mp[d-Available_address]!=0){
                (((header_t *)packet)->dst) = u322big(mp[d-Available_address]);
                d = big2u32(((header_t *)packet)->dst);
                ok=1;
            }
        }
        if (!ok) return -1;
    }

    if (tp==TYPE_DV){
        assert(in_port>1);
        char * st = packet+sizeof(header_t);
        dv[in_port] = *((DV*)st);

        Key[in_port] = dv[in_port].key;

        DV tt=dv[0];
        calc();
#ifdef DEBUG
        printf("router %d entered TYPE_DV\n", key);
        for (int i=0;i<=Port_num;++i){
            
            printf("now print dv[%d] of router %d\n",i,key);
            pr(dv[i]);
        }
#endif
        if (!(tt == dv[0])){
            *((header_t *)packet) = mkheader(0,0,TYPE_DV, sizeof(DV));
            memmove(packet + sizeof(header_t), &dv[0], sizeof(DV));
            *((char *)(packet+sizeof(header_t)+sizeof(DV))) = 0;
            return 0;
        }

        return -1;
    }
    else if (tp==TYPE_DATA){
#ifdef DEBUG
        printf("router %d entered TYPE_DATA\n", key);
#endif
        for (int i=0;i<dv[0].too_num;++i){
            std::pair<uint32_t,int> TOO = dv[0].too[i];
            std::pair<uint32_t,int> DIS = dv[0].dis[i];
            if (d>=TOO.first && d<=TOO.first+((1u<<TOO.second)-1)){
                if (DIS.second>=0){
#ifdef DEBUG
                    printf("router %d are using forward to router\n", key);
#endif
                    for (int i=2;i<=Port_num;++i)
                        if (Key[i]==DIS.second){
#ifdef DEBUG
                            std::cout<<i<<"@201\n"; //!!!
#endif
                            return i;
                        }
                    assert(0);
                }
                //forward to router
                if (External_port!=0 && DIS.second==(-External_port)){
#ifdef DEBUG
                    printf("router %d are using forward to external\n", key);
#endif
                    if (Available_address_b!=-1){
                        for (int j=0;j<(1<<Available_address_b);++j)
                            if (mp[j]==s){
                                ((header_t *)packet)->src = u322big(j+Available_address);
#ifdef DEBUG
                                std::cout<<External_port<<"@207\n"; //!!!
#endif
                                return External_port;
                            }
                        for (int j=0;j<(1<<Available_address_b);++j)
                            if (mp[j]==0){
                                mp[j]=s;
                                ((header_t *)packet)->src = u322big(j+Available_address);
#ifdef DEBUG
                                std::cout<<External_port<<"@216\n"; //!!!
#endif
                                return External_port;
                            }
                    }
                    return -1;
                }
                //forward to external
#ifdef DEBUG
//std::cout<<(-DIS.second)<<"@225\n"; //!!!
printf("router %d are using forward to host\n", key);
#endif
                return (-DIS.second);
                //forward to host
            }
        }
#ifdef DEBUG
        printf("router %d buhui forward\n", key);
        for (int i=0;i<=Port_num;++i){
            printf("now print dv[%d] of router %d\n",i,key);
            pr(dv[i]);
        }
#endif
        return 1;
    }
    else{
        assert(tp==TYPE_CONTROL);
        char * st = (char *)packet + sizeof(header_t);
        if (st[0]=='0'){
            *((header_t *)packet) = mkheader(0,0,TYPE_DV, sizeof(DV));
            memmove(packet + sizeof(header_t), &dv[0], sizeof(DV));
            *((char *)(packet+sizeof(header_t)+sizeof(DV))) = 0;
            return 0;
        }
        else if (st[0]=='1'){
            uint32_t  t = st2u32(((char *)st)+2);
            assert(Available_address_b>=0);
#ifdef DEBUG1
            printf("now print mp[0..%d] of router %d\n",(1<<Available_address_b)-1, key);
            for (int i=0;i<(1<<Available_address_b);++i)
                printf("mp[%d] = %u\n",i,mp[i]);
            printf("%u\n",t);
            printf("%s\n",(char*)st);
#endif
            int tmp=0;
            for (int i=0;i<(1<<Available_address_b);++i)
                if (mp[i]==t){
                    mp[i]=0;
                    ++tmp;
                }
            assert(tmp==1);
            return -1;
        }
        else if (st[0]=='2'){
            int po = atoi(st+2);
            for (int i=2;;++i)
                if (st[i]==' '){
                    st+=i+1;
                    break;
                }
            assert(po>1 && po!=External_port);
            ishost[po]=0;
            int vl = atoi(st);
            len[po]=vl;

            DV tt = dv[0];
            calc();


            if (!(tt == dv[0])){
                *((header_t *)packet) = mkheader(0,0,TYPE_DV, sizeof(DV));
                memmove(packet + sizeof(header_t), &dv[0], sizeof(DV));
                *((char *)(packet+sizeof(header_t)+sizeof(DV))) = 0;
                return 0;
            }

            return -1;
        }
        else{
            assert(st[0]=='3');
            int po = atoi(st+2);
            for (int i=2;;++i)
                if (st[i]==' '){
                    st+=i+1;
                    break;
                }
            uint32_t ip = st2u32((char*)st);
            ishost[po]=1;
            hostaddr[po]=ip;
#ifdef DEBUG
            printf("router %d addhost %d to port %d\n",key,ip,po);
#endif
            DV tt = dv[0];
            calc();
#ifdef DEBUG
            for (int i=0;i<=Port_num;++i){
                printf("now print dv[%d] of router %d\n",i,key);
                pr(dv[i]);
            }
#endif
            if (!(tt == dv[0])){
                *((header_t *)packet) = mkheader(0,0,TYPE_DV, sizeof(DV));
                memmove(packet + sizeof(header_t), &dv[0], sizeof(DV));
                *((char *)(packet+sizeof(header_t)+sizeof(DV))) = 0;
                return 0;
            }

            return -1;
        }
    }
    assert(0);
}
