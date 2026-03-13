/* SPI Slave Tx and Rx Demo

 *
 * SPI pin numbers:
 * SCK   13  // Serial Clock.
 * MISO  12  // Master In Slave Out.
 * MOSI  11  // Master Out Slave In.
 * SS    10  // Slave Select . Arduino SPI pins respond only if SS pulled low by the master
 *
 * Red LED    2
 * Yellow LED 3
 * Green LED  4
 * Blue LED   5
 
 */
#include <SPI.h>
#include<stdint.h>  
#include<string.h>
#define SPI_SCK    13
#define SPI_MISO   12
#define SPI_MOSI   11
#define SPI_SS     10
#define RED_LED    2
#define YELLOW_LED 3
#define GREEN_LED  4
#define BLUE_LED   5

volatile byte data;
volatile boolean dataReceived;
char dataTx[] = {
  'D', 'a', 't', 'a', ' ', 'R', 'e', 'c', 'e', 'i', 'v', 'e', 'd'
};
uint8_t dataInc;

//Initialize SPI slave.
void SPI_SlaveInit(void) 
{ 
  // Initialize SPI pins.
  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(SS, INPUT);

  //Set Clock Phase/Polarity
  SPCR |= (1 << CPOL);
  SPCR &= ~(1 << CPHA);
  
  //Make SPI as slave and enable SPI.
  SPCR |= (1 << SPE);

  //Activate SPI Interrupts.
  SPI.attachInterrupt();
}
  
ISR(SPI_STC_vect)
{
  data = SPDR;
  dataReceived = true;
}

// The setup() function runs right after reset.
void setup() 
{
  dataReceived = false;
  dataInc = 0;

  // Initialize serial communication 
  Serial.begin(31250);

  // Initialize LED pins.
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  // Initialize SPI Slave.
  SPI_SlaveInit();
  Serial.println("Slave Initialized");
  SPDR = (byte)dataTx[dataInc++];
}

// The loop function runs continuously after setup().
void loop() 
{
  if(dataReceived)
  {
    SPDR = (byte)dataTx[dataInc++];
    dataReceived = false;

    if(dataInc > 13)
    {
      dataInc = 0;
      SPDR = (byte)dataTx[dataInc++];
    }
  }

}


   
   
