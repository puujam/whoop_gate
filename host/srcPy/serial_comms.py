"""The main communication process between the host computer and the associated micro."""

import serial
import serial.tools.list_ports
import argparse

# Must match serial baudrate for hub arduino
HUB_SERIAL_BAUDRATE = 115200
HUB_PORT_ID = "Adafruit Feather M0"

def find_port( identification_string ):
    """Searches for a port with a matching identification string.

    :returns: A serial Port object if found. None otherwise."""

    for port in serial.tools.list_ports.comports():
        if identification_string in port[1]:
            return port

    return None

def create_connection( port ):
    """Attempts to establish a connection to a found port.

    :parameter port: A serial Port object to attempt connection to.

    :returns: A Serial object if a connection could be established. None otherwise."""

    try:
        connection = serial.Serial( port, HUB_SERIAL_BAUDRATE, timeout = 0.25, write_timeout = 0.25 )
        
        # Clear out the input buffer, to remove partial messages we may have gotten, which won't make sense to us
        connection.reset_input_buffer()
        return connection
    except Exception as e:
        # Change the exception type once it's established what they could be
        raise

def create_parser():
    """Creates the argparse ArgumentParser instance for this script.

    :returns: An ArgumentParser instance."""

    parser = argparse.ArgumentParser( description = __doc__ )
    parser.add_argument( "--debug", "-d", dest = "debug_mode", action = "store_true", default = False, help = "Enable debug mode for further information" )
    
    return parser

def main_loop( debug_mode ):
    """Daemon control loop. Mostly a state machine.

    :argument debug_mode: Boolean indicating whether debug mode is enabled."""

    connection = None
    sleep_time = 0

    while True:
        if not connection:
            port = find_port( HUB_PORT_ID )
            
            connection = create_connection

def main():
    """Entry point for script. Does initial setup before entering main loop."""

    parser = create_parser()
    args = parser.parse_args()

    main_loop( args.debug_mode )

if __name__ == "__main__":
    main()
