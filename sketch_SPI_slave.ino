/* SPI Slave Demo

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
#define SPI_SCK    13
#define SPI_MISO   12
#define SPI_MOSI   11
#define SPI_SS     10
#define RED_LED    2
#define YELLOW_LED 3
#define GREEN_LED  4
#define BLUE_LED   5

volatile byte data;

//Initialize SPI slave.
void SPI_SlaveInit(void) 
{ 
  // Initialize SPI pins.
  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  //pinMode(MISO, OUTPUT);
  pinMode(SS, INPUT);

  //Set Clock Phase/Polarity
  SPCR |= (1 << CPOL);
  
  //Make SPI as slave and enable SPI.
  SPCR |= (1 << SPE);

  //Activate SPI Interrupts.
  SPI.attachInterrupt();
}
  
ISR(SPI_STC_vect)
{
  data = SPDR;
}

// The setup() function runs right after reset.
void setup() 
{
  // Initialize serial communication 
  Serial.begin(500000);

  // Initialize LED pins.
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  // Initialize SPI Slave.
  SPI_SlaveInit();
  Serial.println("Slave Initialized");
}

// The loop function runs continuously after setup().
void loop() 
{
  switch((char)data)
  {
    case 'r':
    {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
      Serial.print("Data Received: ");
      Serial.println("Red LED ON");
      data = 0;
      break;
    }
    case 'y':
    {
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
      Serial.print("Data Received: ");
      Serial.println("Yellow LED ON");
      data = 0;
      break;
    }
    case 'g':
    {
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, LOW);
      Serial.print("Data Received: ");
      Serial.println("Green LED ON");
      data = 0;
      break;
    }
    case 'b':
    {
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);
      Serial.print("Data Received: ");
      Serial.println("Blue LED ON");
      data = 0;
      break;
    }
  }
}


   
   