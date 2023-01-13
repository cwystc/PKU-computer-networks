#ifndef COMPNET_LAB4_SRC_SWITCH_H
#define COMPNET_LAB4_SRC_SWITCH_H

#include "types.h"

class SwitchBase {
 public:
  SwitchBase() = default;
  ~SwitchBase() = default;

  virtual void InitSwitch(int numPorts) = 0;
  virtual int ProcessFrame(int inPort, char* framePtr) = 0;
};

extern SwitchBase* CreateSwitchObject();

struct table_entry{
	mac_addr_t dst;
	int port;
	int counter;
	bool vld;
};

// TODO : Implement your switch class.
class Switch : public SwitchBase {
private:
	int Port_num;
	table_entry table[222];
public:
    void InitSwitch(int numPorts);
	int ProcessFrame(int inPort, char* framePtr);
};

#endif  // ! COMPNET_LAB4_SRC_SWITCH_H
