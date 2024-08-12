#include "samd21/include/component/pm.h"
#include <cstdint>

#define TC3 ((Tc *)0x42002C00UL)
#define TC4 ((Tc *)0x42003000UL)
#define TC5 ((Tc *)0x42003400UL)
#define TC6 ((Tc *)0x42003800UL)
#define TC7 ((Tc *)0x42003C00UL)

class TIMER_DEAKIN {
public:
  // Tc is a structure data type
  // timer-ptr points to the address of the Timers
  Tc* timer_ptr = nullptr;
  bool config_timer(int timer_num, uint32_t starting_count,
                    uint32_t ending_count, float timer_resolution) {
    /*
      timer_num: represents the number from 3 - 7 for selecting the timer from TC3-TC7, discrete value
      starting_count: it represents the count value of timer, its typically large, suitable for timer counts
      ending_count: provides large range and ensures that negative values are not mistakenly used.
      timer_resolution: represents the resolution of the timer in seconds and may include fractional values.
    */

    // Timer should be within TC3-TC7
    if (timer_num < 3 || timer_num > 7) 
      return false;

    // Calculating the prescaler, required frequency and the count value
    uint32_t clock_freq = 48000000; // 48MHz
    uint32_t resolution_in_ticks = static_cast<uint32_t>(timer_resolution * 10000); // Convert seconds to ticks
    uint32_t period_ticks = ending_count - starting_count;
    uint32_t required_freq = period_ticks * resolution_in_ticks;
    uint32_t prescaler = clock_freq / required_freq;
    uint16_t countValue = period_ticks;


    if (timer_num == 3) timer_ptr = TC3;
    else if (timer_num == 4) timer_ptr = TC4;
    else if (timer_num == 5) timer_ptr = TC5;
    else if(timer_num == 6) timer_ptr = TC6;
    else if(timer_num == 7) timer_ptr = TC7;

    if (timer_ptr == nullptr) return false;
    /* GCLK and Timer Configuration */
    disableTimer(timer_ptr);
    configureClock(timer_num);
    configureTimer(timer_ptr, prescaler);
    setCompareValue(timer_ptr, countValue);
    enableTimer(timer_ptr);

    if(timer_ptr->COUNT16.CC[0].reg != 0 && timer_ptr->COUNT16.CTRLA.bit.ENABLE != 0){
      return true;
    }
    return false;
  }

  /**
  * @brief Configures the clock for the wit function.
  * @param[in] period The number of time in ms for delay.
  */
  void wait(float period_0_1ms) {
    // If no timer has been configured, configure one now
    if (timer_ptr == nullptr) {
        Serial.println("No timer configured. Configuring TC3.");
        config_timer(3, 0, period_0_1ms * 48, 0.0001); // Configuring TC3 as a fallback
    }

    // Proceed with the wait using the configured timer
    Tc* timer = timer_ptr;
    if (timer == nullptr) {
        Serial.println("No available timers");
        return;
    }
    
    // Convert the period from 0.1ms to ms
    uint32_t period_ms = period_0_1ms * 100;
    uint32_t countValue = period_ms * 48; // 48 ticks per 0.1 ms for 48MHz clock
    disableTimer(timer);
    configureTimer(timer, 1); // Use a prescaler of 1 for the most precise timing
    setCompareValue(timer, countValue);

    timer->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0; // Clear any pending interrupt flags
    enableTimer(timer);

    uint32_t startTime = millis();
    uint32_t timeout = (uint32_t)(period_0_1ms * 10 + 100); // Timeout set to the period + 100ms buffer

    while (!(timer->COUNT16.INTFLAG.bit.MC0)) {
        if (millis() - startTime > timeout) {
            Serial.println("Wait timeout");
            break;
        }
    }

    if (timer->COUNT16.INTFLAG.bit.MC0) {
        timer->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
        // Serial.println("Timer completed successfully");
    } 
    // else {
    //     Serial.println("Timer did not reach compare value");
    // }

    disableTimer(timer);
  }

  /*
    The return type of getting count values of the timers should be unsigned 32-bit integer
    because it can hold large range of positive numbers
  */
  /**
  * @brief Get the count of timer: TC3
  */
  uint32_t getTC3_count(){
    return TC3->COUNT16.COUNT.reg;
  }

  /**
  * @brief Get the count of timer: TC4
  */
  uint32_t getTC4_count(){
    return TC4->COUNT16.COUNT.reg;
  }

  /**
  * @brief Get the count of timer: TC5
  */
  uint32_t getTC5_count(){
    return TC5->COUNT16.COUNT.reg;
  }

   /**
  * @brief Get the count of timer: TC6
  */
  uint32_t getTC6_count(){
    return TC6->COUNT16.COUNT.reg;
  }

   /**
  * @brief Get the count of timer: TC7
  */
  uint32_t getTC7_count(){
    return TC7->COUNT16.COUNT.reg;
  }

private:
  // Helper function for wait() function
  /**
  * @brief Gets an available timer that is not currently in use.
  * @return Pointer to an available timer, or nullptr if none are available
  */
  Tc* getAvailableTimer(){
    if(TC3 != timer_ptr) return TC3;
    else if(TC4 != timer_ptr) return TC4;
    else if(TC5 != timer_ptr) return TC5;
    // else if(TC6 != timer_ptr) return TC5;
    // else if(TC7 != timer_ptr) return TC7;
    else return nullptr;
  }

  // Helper functions for configuring timer
  void disableTimer(Tc* timer) {
    timer->COUNT16.CTRLA.bit.ENABLE = 0;
  }

   /**
  * @brief configures the clock for the timer
  */
  void configureClock(int timer_num) {
    uint16_t clkctrl_value = 0x0000;
    if (timer_num == 3) clkctrl_value = 0x451B; // TCC2, TC3
    else if (timer_num == 4 || timer_num == 5) clkctrl_value = 0x451C; // TC4, TC5
    else if (timer_num == 6 || timer_num == 7) clkctrl_value = 0x451D; // TC6, TC7

    // Enable clock for the timer
    PM->APBCMASK.bit.TC3_ = (timer_num == 3);
    PM->APBCMASK.bit.TC4_ = (timer_num == 4);
    PM->APBCMASK.bit.TC5_ = (timer_num == 5);
    PM->APBCMASK.bit.TC6_ = (timer_num == 6);
    PM->APBCMASK.bit.TC7_ = (timer_num == 7);
    // Configure GCLK
    GCLK->GENDIV.reg = 0x00003005; // DIV: 48
    GCLK->GENCTRL.reg = 0x00010705; // SRC: DFLL48M, GENEN: 1
    GCLK->CLKCTRL.reg = clkctrl_value;
  }

  void configureTimer(Tc* timer, uint32_t prescaler) {
    uint16_t timer_value = 0x0722; // Default to 1024 prescaler

    // Set prescaler based on given value
    if (prescaler <= 1) timer_value = 0x0022;
    else if (prescaler <= 2) timer_value = 0x0122;
    else if (prescaler <= 4) timer_value = 0x0222;
    else if (prescaler <= 8) timer_value = 0x0322;
    else if (prescaler <= 16) timer_value = 0x0422;
    else if (prescaler <= 64) timer_value = 0x0522;
    else if (prescaler <= 256) timer_value = 0x0622;
    else timer_value = 0x0722;

    timer->COUNT16.CTRLA.reg = timer_value;
  }

  void setCompareValue(Tc* timer, uint16_t countValue) {
    timer->COUNT16.CC[0].reg = countValue;
  }

  void enableTimer(Tc* timer) {
    timer->COUNT16.CTRLA.bit.ENABLE = 1;
    while (GCLK->STATUS.bit.SYNCBUSY);
  }
};
