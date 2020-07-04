#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>
#include <LEDAnimation.h>

// US frequency, change to 868 for EU
#define RF95_FREQ 915.0

// The pins for the Feather M0 RFM95
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Using a define to ensure minimal runtime penalty in production
#define DEBUG

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

#define ENABLE_BUZZER
#define BUZZER_PIN 10

#define LED_PIN 6
#define LED_COUNT 20

#define SERVER_ADDRESS 1
#define MY_ADDRESS 2

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Create our reliable datagram manager
RHReliableDatagram manager( rf95, MY_ADDRESS );

// Create the neopixel strip
Adafruit_NeoPixel_ZeroDMA strip( LED_COUNT, LED_PIN, NEO_GRB );

uint32_t idle_frames[4][4] = {
	{ 
		0xFFFFFF, // white	
		0x000000, // off
		0x000000, // off
		0x000000  // off
	},
	{ 
		0x000000, // off
		0xFFFFFF, // white	
		0x000000, // off
		0x000000  // off
	},
	{ 
		0x000000, // off
		0x000000, // off
		0xFFFFFF, // white	
		0x000000  // off
	},
	{ 
		0x000000, // off
		0x000000, // off
		0x000000, // off
		0xFFFFFF  // white	
	}
};

FrameAnimation idle_animation(
	4, //LED Count
	4, // Frame Count
	idle_frames // Frames
);

uint32_t pass_frames[2][1] = {
	{
		0xFF0000 // red
	},
	{
		0x00FF00 // blue
	},
};

FrameAnimation pass_animation(
	1, // LED Count
	2, // Frame Count
	pass_frames // Frames
);

// Used for tracking motion debouncing
bool motion_detected = false;
unsigned long debounce_ends = 0;

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
	// Set the radio module Reset pin to output and start it high
	pinMode( RFM95_RST, OUTPUT );
	digitalWrite( RFM95_RST, HIGH );

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

void led_strip_setup()
{
	strip.begin();
	strip.setBrightness(32);
	strip.show();
}

void builtin_led_setup()
{
	// Set the LED pin to output
	pinMode( LED_BUILTIN, OUTPUT );
	digitalWrite( LED_BUILTIN, LOW );
}

void motion_detected_handler()
{
	// Minimizing intterupt time
	motion_detected = true;
}

void pir_setup()
{
	// Set our PIR input pin as an interrupt
	pinMode( PIR_PIN, INPUT );
	attachInterrupt( digitalPinToInterrupt( PIR_PIN ), motion_detected_handler, RISING );
}

void setup()
{
#ifdef DEBUG
	Serial.begin(115200);
#endif

	led_strip_setup();

	radio_module_setup();

	pir_setup();

	builtin_led_setup();

	SERIAL_DEBUG( "Setup complete" );
}

void detect_new_motion()
{
	if ( motion_detected )
	{
		if ( debounce_ends == 0 )
		{
			SERIAL_DEBUG( "Motion detected!" );

			debounce_ends = millis() + PIR_DEBOUNCE_PERIOD_MS;
			queued_detect_message = true;

			digitalWrite( LED_BUILTIN, HIGH );

			#ifdef ENABLE_BUZZER
				tone( BUZZER_PIN, 1500 );
			#endif
		}
	}
}

void clear_debounce()
{
	if ( debounce_ends != 0 )
	{
		if ( millis() > debounce_ends )
		{
			SERIAL_DEBUG( "Debounce ended" );

			motion_detected = false;
			debounce_ends = 0;

			digitalWrite( LED_BUILTIN, LOW );

			#ifdef ENABLE_BUZZER
				noTone( BUZZER_PIN );
			#endif
		}
	}
}

void send_queued_message()
{
	if ( queued_detect_message )
	{
		SERIAL_DEBUG( "Sending Message" );
		manager.sendtoWait( data, sizeof( data ), SERVER_ADDRESS );
		queued_detect_message = false;
		SERIAL_DEBUG( "Message Sent" );
	}
}

void update_animation()
{
	FrameAnimation *active_animation = &idle_animation;

	if ( debounce_ends != 0 )
	{
		active_animation = &pass_animation;
	}

	*active_animation.drawNext( strip );
}

void loop()
{
	detect_new_motion();

	clear_debounce();

	send_queued_message();

	update_animation();

	delay( 50 );
}
