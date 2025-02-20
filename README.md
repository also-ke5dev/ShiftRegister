A header-only library for controlling 8 bit shift registers like the 74xx595 and variants in an Arduino friendly way. Can use SPI or shiftOut() depending on your hardware configuration. 

Usage is simple:


  ShiftRegister port = ShiftRegister(SPI, SR_LATCH);

  // set all pins LOW
  port.Clear();

  // sets the first pin HIGH
  port.DigitalWrite(1, HIGH);

  // toggles the second pin. 
  // since it was initialized as LOW, it pulses HIGH for the default number of microseconds (10)
  port.DigitalToggle(2);

  // write all pins at once. We use binary notation to set each output pin.
  port.Write(B00110011);

  

