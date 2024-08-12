#define HIGH true
#define LOW false

class GPIO_DEAKIN {
  volatile PortGroup *portA = &PORT->Group[0];
  volatile PortGroup *portB = &PORT->Group[1];

public:
  bool Config_GPIO(char PortNum, char PinNum, String Mode) {
    if (PortNum == 'A') {
      if (Mode == "OUTPUT") {
        portA->DIR.reg |= (1 << PinNum);
        return true;
      } else {
        portA->DIR.reg &= ~(1 << PinNum);
        return false;
      }
    } else if (PortNum == 'B') {
      if (Mode == "OUTPUT") {
        portB->DIR.reg |= 1 << PinNum;
        return true;
      } else {
        portB->DIR.reg &= ~(1 << PinNum);
        return false;
      }
    }
    return false;
  }
  bool Write_GPIO(char PortNum, char PinNum, bool state) {
    if (PortNum == 'A') {
      if (state == HIGH) {
        portA->OUT.reg |= 1 << PinNum;
        return true;
      } else if (state == LOW) {
        portA->OUT.reg &= ~(1 << PinNum);
        return false;
      }
    } else if (PortNum == 'B') {
      if (state == HIGH) {
        portB->OUT.reg |= 1 << PinNum;
        return true;
      } else if (state == LOW) {
        portB->OUT.reg &= ~(1 << PinNum);
        return false;
      }
    }
    return false;
  }
  bool Read_GPIO(char PortNum, char PinNum) {
    uint32_t pinMask = (1 << PinNum);
    if (PortNum == 'A') {
      if (portA->IN.reg & pinMask)
        return true;
      else
        return false;
    } else if (PortNum == 'B') {
      if (portB->IN.reg & pinMask)
        return true;
      else
        return false;
    }
    return false;
  }
};
