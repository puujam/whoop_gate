#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

// US frequency, change to 868 for EU
#define RF95_FREQ 915.0

// The pins for the Feather M0 RFM95
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Using a define to ensure minimal runtime penalty in production
//#define DEBUG

#ifdef DEBUG
	#define SERIAL_DEBUG( message ) Serial.println( message );
#else
  #define SERIAL_DEBUG( message ) {}
#endif

#ifdef DEBUG
  #define SERIAL_DEBUG_NO_NEWLINE( message ) Serial.print( message );
#else
  #define SERIAL_DEBUG_NO_NEWLINE( message ) {}
#endif

#define SERVER_ADDRESS 1

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Create our reliable datagram manager
RHReliableDatagram manager( rf95, SERVER_ADDRESS );

uint8_t receive_buffer[ RH_RF95_MAX_MESSAGE_LEN ];

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

	while (!manager.init()) 
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

  // Set us to retry 3 times with a 200ms timeout
  //manager.setRetries( 3 );
  //manager.setTimeout( 200 );
}

void setup()
{
	// We always use serial on the hub
	Serial.begin(115200);

	radio_module_setup();
}

void loop()
{
	// Look if there's a message for us
	if ( manager.available() )
	{
		uint8_t buffer_length = sizeof( receive_buffer );
		uint8_t from_address = 0;

		// Get the message along with the transmitter's address
		if ( manager.recvfromAckTimeout( receive_buffer, &buffer_length, 200, &from_address ) )
		{
			Serial.print( from_address, HEX );
			Serial.print( ": " );
			Serial.println( (char *) receive_buffer );
		}
   else
   {
    SERIAL_DEBUG( "Didn't Receive" );
   }
	}
  else
  {
    SERIAL_DEBUG( "No messages" );
  }

  delay( 50 );
}
