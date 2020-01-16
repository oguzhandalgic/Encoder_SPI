#include <SPI.h>
#include <stdint.h>

// Define clock and data pins for PIN_TOGGLING
#define SSI_CLK        5
#define SSI_DATA       6

// Define bit count, 5Byte (5*8bits = 40bits) of meaningful data
#define BIT_COUNT      40

// Encoder single turn resolution 23bits, i.e. 2^23
#define ENC_RESOLUTON  8388607.0

// Global variables
float angle = 0.0;
uint64_t rawData = 0;
uint32_t singleTurn = 0;

// Time measurement variables
uint32_t t1, t2;

// Function prototypes
uint64_t readData_SPI(void);
uint64_t readData_PIN_TOGGLE(void);
void printRawData(uint64_t rawData);
uint32_t getSingleTurn(uint64_t rawData);
float calculateAngle(uint32_t singleTurn);

// Adjust SPI settings, 1MHz clock speed
SPISettings settingsSPI(1000000, MSBFIRST, SPI_MODE1);


void setup()
{
  // Set the directions of clock and data for pin toggling method
  pinMode(SSI_CLK, OUTPUT);
  pinMode(SSI_DATA, INPUT);

  // Initialize serial port, 115200bps
  Serial.begin(115200);
  Serial.println("Test begin...");
  delay(1000);

  // Initalize SPI with CPOL = 0, CPHA = 1, MSB First, Freq = 1MHz
  SPI.begin();

  // Set clock pin HIGH
  digitalWrite(SSI_CLK, HIGH);
}


void loop()
{
  //rawData = readData_PIN_TOGGLE();

  //t1 = micros();
  rawData = readData_SPI();
  //t2 = micros();
  
  singleTurn = getSingleTurn(rawData);
  angle = calculateAngle(singleTurn);

  Serial.println(angle, 3);
  //Serial.println(t2-t1);
  
  //printRawData(rawData);
}


/*
 * Read data using SPI interface, much faster 
 * compared to pin toggling method
 */
uint64_t readData_SPI(void)
{
  uint8_t byteCount, data;
  uint64_t rawData = 0;

  SPI.beginTransaction(settingsSPI);
  for (byteCount = 0; byteCount < 5; byteCount++)
  {
    rawData |= data;
    data = SPI.transfer(0x00);
    rawData <<= 8;
  }
  SPI.endTransaction();

  return rawData;
}


/*
 * Read data using pin toggle method, slower compared to SPI
 */
uint64_t readData_PIN_TOGGLE(void)
{
  uint64_t data = 0;

  for (int i = 0; i < BIT_COUNT; i++)
  {
    data <<= 1;
    digitalWrite(SSI_CLK, LOW);
    delayMicroseconds(1);
    digitalWrite(SSI_CLK, HIGH);
    delayMicroseconds(1);
    data |= digitalRead(SSI_DATA);
  }
  delayMicroseconds(25);

  return data;
}


/*
 * Print the raw data received from the encoder this 
 * includes multi turn (16bits) and single turn (23bits)
 * values combined. See below visualization
 * 
 * MSB                             LSB   
 * bit40  39   ...  23   22 ... 1  bit0
 *       |   MultiTurn  | SingleTurn  |
 */
void printRawData(uint64_t rawData)
{

  Serial.print((uint8_t)(rawData >> 32) & 0xFF, HEX);
  Serial.print(" ");
  Serial.print((uint8_t)(rawData >> 24) & 0xFF, HEX);
  Serial.print(" ");
  Serial.print((uint8_t)(rawData >> 16) & 0xFF, HEX);
  Serial.print(" ");
  Serial.print((uint8_t)(rawData >> 8)  & 0xFF, HEX);
  Serial.print(" ");
  Serial.print((uint8_t)(rawData)     & 0xFF, HEX);
  Serial.println();

}


/*
 * Obtain single turn value, which is 23bit in my encoder
 */
uint32_t getSingleTurn(uint64_t rawData)
{
  uint32_t singleTurn = 0;
  singleTurn = (rawData & 0xFFFFFF) >> 1;
  return singleTurn;
}


/*
 * Calculate the position of the encoder in terms of 360 degrees
 */
float calculateAngle(uint32_t singleTurn)
{
  return (singleTurn * 360.0 / ENC_RESOLUTON);
}
