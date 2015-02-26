/**************************************************************************/
/*!
    @file     Adafruit_MCP4725.h
    @author   K. Townsend (Adafruit Industries)
	@license  BSD (see license.txt)

	This is a library for the Adafruit MCP4725 breakout board
	----> http://www.adafruit.com/products/935

	Adafruit invests time and resources providing this open source code,
	please support Adafruit and open-source hardware by purchasing
	products from Adafruit!

	@section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/
#ifndef Adafruit_MCP4725_h
#define Adafruit_MCP4725_h

#define MCP4725_CMD_WRITEDAC            (0x40)  // Writes data to the DAC
#define MCP4725_CMD_WRITEDACEEPROM      (0x60)  // Writes data to the DAC and the EEPROM (persisting the assigned value after reset)

class Adafruit_MCP4725{
 public:
  Adafruit_MCP4725(uint8_t addr);
  void setValue( uint16_t output);
  void setValue( uint16_t output, bool writeEEPROM );
  void begin();

 private:
  uint8_t _i2caddr;
};

/**************************************************************************/
/*!
    @brief  Instantiates a new MCP4725 class
*/
/**************************************************************************/
Adafruit_MCP4725::Adafruit_MCP4725(uint8_t addr) {
    _i2caddr = addr;
}

/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
void Adafruit_MCP4725::begin() {
  Wire.setSpeed(CLOCK_SPEED_400KHZ); // Spark Core way of setting i2c speed
  Wire.begin();
}

/**************************************************************************/
/*!
    @brief  Sets the output voltage to a fraction of source vref.  (Value
            can be 0..4095)

    @param[in]  output
                The 12-bit value representing the relationship between
                the DAC's input voltage and its output voltage.
    @param[in]  writeEEPROM
                If this value is true, 'output' will also be written
                to the MCP4725's internal non-volatile memory, meaning
                that the DAC will retain the current voltage output
                after power-down or reset.
*/
/**************************************************************************/

void Adafruit_MCP4725::setValue( uint16_t output) {
    setValue(output, false);
}

void Adafruit_MCP4725::setValue( uint16_t output, bool writeEEPROM ) {

  Wire.beginTransmission(_i2caddr);

  if (writeEEPROM) {
    Wire.write(MCP4725_CMD_WRITEDACEEPROM);
  } else {
    Wire.write(MCP4725_CMD_WRITEDAC);
  }

  Wire.write(output / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  Wire.write((output % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  Wire.endTransmission();

}


#endif