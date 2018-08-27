#include "Arduino.h"

class Button {
  public:
    Button(int pin, void (*_onClick)());
    void loop();
        
   private:
    int _pin;
    int _prev_state = HIGH;
    int _current_state = HIGH;
    void (*_onClick)();
    bool _onClickCalled = false;
    
    unsigned long _state_change_millis = 0;
    void _onDebouncedStateChange(int state);
};

