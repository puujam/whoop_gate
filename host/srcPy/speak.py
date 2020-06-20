"""Plays Text-to-Speech of whatever string is provided as a parameter.

Uses Google Text-to-Speech by default, which requires internet access. If access is unavailable, falls back on pyttsx3 for local tts creation."""

def gtts_speak( to_speak ):
    """Attempts to speak the given string with Google Text-to-Speech and pygame.

    :parameter to_speak: The string to attempt to speak.

    :returns: A boolean indicating whether the string was successfully played."""

    try:
        # get the mp3 formatted data from Google TTS
        # (this requires internet access)
        import gtts
        tts = gtts.gTTS( to_speak )
        
        # write the data to a buffer
        from io import BytesIO
        audio_buffer = BytesIO()
        tts.write_to_fp( audio_buffer )
        
        # reset our location in the buffer
        audio_buffer.seek(0)

        # play data from the buffer with pygame
        import pygame
        pygame.mixer.init()
        pygame.mixer.music.load( audio_buffer )
        pygame.mixer.music.play()

        while pygame.mixer.music.get_busy():
            pygame.time.Clock().tick(10)

        return True
    except gtts.tts.gTTSError:
        return False

def pyttsx3_speak( to_speak ):
    """Attempts to speak the given string with pyttysx3.

    :returns: Always true for now."""

    import pyttsx3
    engine = pyttsx3.init()
    engine.say( to_speak )
    engine.runAndWait()

    # I don't know what exceptions to expect here, and it's been working fine so whatever
    return True

def create_parser():
    """Creates the argparse.ArgumentParser instance for this script. Separating into a function like this allows for easy documentation with Sphinx."""

    import argparse

    parser = argparse.ArgumentParser( description = __doc__ )
    parser.add_argument( "to_speak", help = "The text to be spoken. Only English is supported." )
    parser.add_argument( "--debug", "-d", dest = "debug_mode", action = "store_true", default = False, help = "Enable debug mode for additional information." )

    return parser

def main():
    """Main function. Includes argument parsing."""

    parser = create_parser()
    args = parser.parse_args()

    if args.debug_mode:
        import time
        start_time = time.time_ns()
    
    if not gtts_speak( args.to_speak ):
        print( "Failed to use gTTS, falling back on pyttsx3." )
        pyttsx3_speak( args.to_speak )

    if args.debug_mode:
        end_time = time.time_ns()
        elapsed_time = end_time - start_time
        print( "Took {} ns".format( elapsed_time ) )

if __name__ == "__main__":
    main()
