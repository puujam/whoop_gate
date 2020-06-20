#include <LEDAnimation.h>

FrameAnimation::FrameAnimation( uint8_t LED_count_in, uint8_t frame_count_in, uint32_t **frames_in )
{
	LED_count = LED_count_in;
	frame_count = frame_count_in;
	frames = frames_in;

	current_frame = 0;
}

void FrameAnimation::drawNext( Adafruit_NeoPixel_ZeroDMA strip )
{
	for( uint16_t current_LED = 0; current_LED < strip.numPixels(); current_LED++ )
	{
		strip.setPixelColor( current_LED, frames[ current_frame ][ current_LED % LED_count ] );
	}

	strip.show()
	current_frame++;
}

void write_out( uint8_t *out_buffer, uint16_t *out_buffer_location, void *data, size_t data_size )
{
	memcpy( &out_buffer[ *out_buffer_location ], data, data_size );
	*out_buffer_location += data_size;
}

void FrameAnimation::serialize( uint16_t out_buffer_size, uint8_t *out_buffer, uint16_t *out_written_size )
{
	uint16_t current_byte = 0;

	write_out( out_buffer, &current_byte, LED_count, sizeof( LED_count ) );
	write_out( out_buffer, &current_byte, frame_count, sizeof( frame_count ) );

	for( uint8_t current_frame = 0; current_frame < frame_count; current_frame++ )
	{
		for( uint8_t current_LED = 0; current_LED < LED_count; current_LED++ )
		{
			write_out( out_buffer, &current_byte, frames[ current_frame ][ current_LED ], sizeof( uint32_t ) );
		}
	}

	*out_written_size = current_byte;
}
