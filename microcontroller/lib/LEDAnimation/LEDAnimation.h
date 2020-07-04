/**
 * A library to define an LED animation.
 * Written by Patrick Harris
 * Licensed under GNU GPLv3
 */
#ifndef LEDAnimation_h
#define LEDAnimation_h

/* For Feather MO this library only functions
 * on pins 5, 6, 12, and MOSI */
#include <Adafruit_NeoPixel_ZeroDMA.h>


class FrameAnimation
{
	/* Access controls are for cowards */
	public:
		uint32_t **frames;
		uint8_t frame_count;
		uint8_t LED_count;

		uint8_t current_frame;

		FrameAnimation( uint8_t LED_count_in, uint8_t frame_count_in, uint32_t frames_in[][] );

		void drawNext( Adafruit_NeoPixel_ZeroDMA strip );

		void serialize( uint16_t out_buffer_size, uint8_t *out_buffer );

		/* Static functions */
		static FrameAnimation deserialize( uint16_t buffer_size, uint8_t *buffer );
		static FrameAnimation createGradientAnimation( uint8_t led_count, uint8_t frame_count, uint8_t color_point_count, uint32_t *color_points );
};

#endif
