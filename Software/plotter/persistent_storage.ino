#include <EEPROM.h>
#include "MachineDefs.h"

void eepromSetup(){
    EEPROM.begin(512);
    //eepromReset();
}

void eepromReset(){
    SER_PRINTLN("EEPROM: Store");

    eepromWriteLong(EEPROM_LEFT_ADDR, 150);
    eepromWriteLong(EEPROM_RIGHT_ADDR, 150);
    eepromWriteLong(EEPROM_DISPARITY_ADDR, 300);
    eepromWriteLong(EEPROM_STOPPED_AT_ADDR, 100);
    eepromWriteLong(EEPROM_CURRENT_PLOT_ADDR, 100);
    eepromWriteFloat(EEPROM_PRINT_SIZE_ADDR, 1);
    eepromWriteFloat(EEPROM_CENTER_X_ADDR, 150);
    eepromWriteFloat(EEPROM_CENTER_Y_ADDR, 150);

    eepromCommit();
}
 
void eepromWriteLong(int addr, long data) {
  for(int b=0 ; b<4 ; b++) {
    EEPROM.write(addr+b, data & 0xff);
    data = data >> 8;
  }
}

void eepromWriteFloat(int addr, float data) {
  long tmp = *(long*)&data;
  eepromWriteLong(addr, tmp);
}

long eepromReadLong(int addr) {
  long ret = 0;
  for(int b=0 ; b<4 ; b++) {
    ret = (ret<<8) | EEPROM.read(addr+(3-b));
  }
  return ret;
}

float eepromReadFloat(int addr) {
  long tmp = eepromReadLong(addr);  
  return *(float*)&tmp;
}

void eepromCommit(){
  EEPROM.commit();
}
