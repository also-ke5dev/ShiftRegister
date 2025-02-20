#include "Arduino.h"
#include <SPI.h>

#define SPI_4MHZ    4000000
#define SPI_8MHZ    8000000
#define SPI_12MHZ   12000000
#define SPI_16MHZ   16000000
#define SPI_20MHZ   20000000
#define SPI_25MHZ   25000000
#define SPI_30MHZ   30000000
#define SPI_40MHZ   40000000

#define SPI_SPEED   SPI_8MHZ

/*  
    74xx595
    Function        Name    Pin     Tie     Logic
    --------        ----    ---     ---     -----
    output enable   OE      13      0V      active low
    master reset    MR      10      5V      active low

    74xx164
    Function        Name    Pin     Tie     Logic
    --------        ----    ---     ---     -----

*/

class ShiftRegister
{
    public:

        ShiftRegister()
        {
            _data = -1;
            _clock = -1;
            _latch = -1;
            _width = -1;
            _toggle_delay = 10;
            _last_data = 0;
            _mode = data_modes::Virtual; 
            _initialize();       
        }

        ShiftRegister(uint8_t data_pin, uint8_t clock_pin)
        {
            _data = data_pin;
            _clock = clock_pin;
            _latch = -1;
            _width = 8;
            _toggle_delay = 10;
            _last_data = 0;
            _mode = data_modes::shiftX;
            _initialize();
        }

        ShiftRegister(uint8_t data_pin, uint8_t clock_pin, uint8_t latch_pin, uint8_t default_value = 0x00)
        {
            _data = data_pin;
            _clock = clock_pin;
            _latch = latch_pin;
            _width = 8;
            _toggle_delay = 10;
            _last_data = default_value;
            _mode = data_modes::shiftX;
            _initialize();
        }

        ShiftRegister(SPIClass spi, uint8_t latch_pin, uint8_t default_value = 0x00)
        {
            _spi = spi;
            _latch = latch_pin;
            _width = 8;
            _toggle_delay = 10;
            _last_data = default_value;
            _mode = data_modes::SPI;
            _initialize();
        }

        ~ShiftRegister() { _destroy(); }
 
        void Write(uint8_t data)
        {
            _last_data = data;            
            switch (_mode)
            {
                case data_modes::shiftX:
                    shiftOut(_data, _clock, MSBFIRST, data);
                    break;
                case data_modes::SPI:
                    _spi.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));
                    _spi.transfer(data);
                    break;
            }
            if (_latch > -1) 
            {
                digitalWrite(_latch, LOW);
                digitalWrite(_latch, HIGH);
            }
            if (_mode == data_modes::SPI) _spi.endTransaction();
        }

        uint8_t Read() { return _last_data; }

        void DigitalWrite(uint8_t pin, bool state)
        {
            if (state)
            {
                bitSet(_last_data, pin);
            }
            else
            {
                bitClear(_last_data, pin);
            }
            Write(_last_data);
        }

        bool DigitalRead(uint8_t pin)
        {
            return (bit_is_set(_last_data, pin));
        }

        void DigitalToggle(uint8_t pin, int64_t us_delay = -1)
        {
            if (us_delay == -1) us_delay = _toggle_delay;
            bool state = DigitalRead(pin);
            DigitalWrite(pin, !state);
            if (us_delay > 0) delayMicroseconds(us_delay);
            DigitalWrite(pin, state);
        }

        void Clear() { Write(0); }

        void ClearHigh() { Write(0xff); }

        void SetDigitalToggleDelay(int64_t _delay) { _toggle_delay = _delay; }

    private:

        uint8_t _width;
        int _data, _clock, _latch, _last_data, _toggle_delay;
        bool* _cells;
        enum data_modes { Virtual, SPI, shiftX } _mode;
        SPIClass _spi;

        void _initialize()
        {
            if (_mode == data_modes::shiftX)
            {
                if (_data > -1) pinMode(_data, OUTPUT);
                if (_clock > -1) pinMode(_clock, OUTPUT);
                if (_latch > -1) pinMode(_latch, OUTPUT);
            }
            else if (_mode == data_modes::SPI)
            {
                _spi.begin();
                if (_latch > -1) pinMode(_latch, OUTPUT);
            }
            else if (_mode == data_modes::Virtual)
            {
                return;
            }

            // allocate real space for hardware
            if (_width < 1) _width = 1;
            _cells = new bool[_width];           

            // set all outputs to the default
            Write(_last_data);  
        }

        void _destroy()
        {
            if (_width > 0) delete _cells;
        }
};