#include "bus.h"
#include "unity.h"
#include "bus_slave.h"
#include "bus_master.h"

void test_find_handler {
  uint8 cmd = 
  mib_state.bus_command.command = cmd;
  TEST_ASSERT_EQUAL_INT(cmd, find_handler());

  //assert cmd = cmd
  
}



