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

#define PIR_PIN 5
#define PIR_STARTUP_PERIOD_MS 30000 // 30 seconds
#define PIR_DEBOUNCE_PERIOD_MS 2500 // 2.5 seconds

#define SERVER_ADDRESS 1
#define MY_ADDRESS 2

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Create our reliable datagram manager
RHReliableDatagram manager( rf95, MY_ADDRESS );

// Used for tracking motion debouncing
unsigned long last_accepted_motion = 0;

uint8_t data[ RH_RF95_MAX_MESSAGE_LEN ] = "D";

bool queued_detect_message = false;

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
  manager.setRetries( 3 );
  manager.setTimeout( 200 );
}

bool motion_startup_active( unsigned long time )
{
	return time < PIR_STARTUP_PERIOD_MS;
}

bool debounce_active( unsigned long time )
{
  if ( time < PIR_DEBOUNCE_PERIOD_MS )
  {
    return false;
  }
  
	unsigned long ms_since_last = time - last_accepted_motion;
  
	return ms_since_last < PIR_DEBOUNCE_PERIOD_MS;
}

void motion_detected()
{
	unsigned long detected_time = millis();

	if ( motion_startup_active( detected_time ) )
	{
		SERIAL_DEBUG( "Motion before startup ignored." )
		return;
	}

	if ( debounce_active( detected_time ) )
	{
		SERIAL_DEBUG( "Motion during debounce ignored." )
		return;
	}

	// This is a valid detection
	last_accepted_motion = detected_time;

	SERIAL_DEBUG( "Motion detected!" )

	queued_detect_message = true;
}

void pin_setup()
{
	// Set the radio module Reset pin to output and start it high
	pinMode( RFM95_RST, OUTPUT );
	digitalWrite( RFM95_RST, HIGH );

	// Set our PIR input pin as an interrupt
	pinMode( PIR_PIN, INPUT );
	attachInterrupt( digitalPinToInterrupt( PIR_PIN ), motion_detected, RISING );

	// Set the LED pin to output
	pinMode( LED_BUILTIN, OUTPUT );
	digitalWrite( LED_BUILTIN, LOW );
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
	unsigned long current_time = millis();

	if ( debounce_active( current_time ) )
	{
    SERIAL_DEBUG( "LED On" );
		digitalWrite( LED_BUILTIN, HIGH );
	}
	else
	{
    SERIAL_DEBUG( "LED Off" );
		digitalWrite( LED_BUILTIN, LOW );
	}

  if ( queued_detect_message )
  {
    manager.sendtoWait( data, sizeof( data ), SERVER_ADDRESS );
    queued_detect_message = false;

    SERIAL_DEBUG( "Message Sent" );
  }

	delay( 50 );
}
