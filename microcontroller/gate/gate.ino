#include <SPI.h>
#include <RH_RF95.h>

// US frequency, change to 868 for EU
#define RF95_FREQ 915.0

// The pins for the Feather M0 RFM95
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define PIR_PIN 5

// Using a define to ensure minimal runtime penalty in production
#define DEBUG

#define SERIAL_DEBUG( message ) \
#ifdef DEBUG \
	Serial.println( message ); \
#endif

#define SERIAL_DEBUG_NO_NEWLINE( message ) \
#ifdef DEBUG \
	Serial.print( message ); \
#endif

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void reset_radio_module()
{
	digitalWrite( RFM95_RST, LOW );
	delay( 10 );
	digitalWrite( RFM95_RST, HIGH );
	delay( 10 );
}

void radio_module_setup()
{
	reset_radio_module();

	while (!rf95.init()) 
	{
		SERIAL_DEBUG("LoRa radio init failed");
		SERIAL_DEBUG("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
		while (1);
	} 
	// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
	if (!rf95.setFrequency(RF95_FREQ))
	{
		SERIAL_DEBUG("setFrequency failed");
		while (1);
	}

	SERIAL_DEBUG_NO_NEWLINE("Set Freq to: ");
	SERIAL_DEBUG(RF95_FREQ);

	// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	// The default transmitter power is 13dBm, using PA_BOOST.
	// If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
	// you can set transmitter powers from 5 to 23 dBm:
	rf95.setTxPower(23, false);
}

void pin_setup()
{
	// Set the radio module Reset pin to output and start it high
	pinMode( RFM95_RST, OUTPUT );
	digitalWrite( RFM95_RST, HIGH );
}

void setup()
{
#ifdef DEBUG
	Serial.begin(115200);
#endif

	pin_setup();

	radio_module_setup();
}

void loop()
{
}
