#include "pn532.hpp"

//IPASS DEMONSTRATION CODE.

int main() {

	// Required pins to use this library with I2C.
	auto scl = hwlib::target::pin_oc( hwlib::target::pins::scl );
	auto sda = hwlib::target::pin_oc( hwlib::target::pins::sda );
	auto rst = hwlib::target::pin_out( hwlib::target::pins::d3 );
	// Optional pin to save some bandwidth on the bus.
	auto irq = hwlib::target::pin_in( hwlib::target::pins::d2 );
	
	// Create the object, "object" can be replaced with any name of your choosing.
	pn532 object = pn532( scl, sda, rst, irq );
	
	// Fill this array with the card uid's you would like to use in your code.
	// If your uid is only 4 bytes, pad it up with zeros as shown in this example.
	// Ofcourse with a little programming magic this could also point to a database,
	// or perhaps a filesystem, however this is not handled in the examples.
	std::array<std::array<uint8_t, 7>, 2> known_uid = {{{0xA4, 0x93, 0x4F, 0x12, 0x00, 0x00, 0x00},
														{0x02, 0x21, 0x0B, 0x21, 0x00, 0x00, 0x00}}};

	// Read the boards hardware and firmware version.
	// Should always be: 
	object.get_firmware_version();
	
	// Read the states of the gpio ports and places them into our array.
	std::array<uint8_t, 3> gpio_states;
	object.read_gpio( gpio_states );
	
	// Waits untill a card gets in range, reads its uid and places that into our array.
	std::array<uint8_t, 7> uid;
	object.get_card_uid( uid );
		
	// A loop to check if the received uid is known to us and does stuff
	// based on whether or not we've added the uid into our array.
	bool card_auth = false;
	for(size_t i = 0; i < known_uid.size(); i++) {
		
		// The below if statement cal also be used with a specific uid
		// replace the "i" with the array index of your choosing.
		// Remember it starts at 0!
		if( uid == known_uid[i] ) {
			
			// We do stuff if we have a known uid.
			hwlib::cout << "Found trusted uid!\n";
			// Set GPIO P72 to HIGH, in my case we light up a blue LED.
			object.write_gpio( 0x00, 0x84 );
			// Set a bool so the code below wont run if we detected a known uid.
			card_auth = true;
			
		}
	}
	// If no known card is found the following code gets run.
	if( !card_auth ) {
		
		hwlib::cout << "No known card detected!\n";
		// Set GPIO P71 to HIGH, in my case we light up a red LED.
		object.write_gpio( 0x00, 0x82 );
		
	}
	
	// This code is only ran when a specific uid is detected.
	// Potential use for different levels of access.
	if( uid == known_uid[1] ) {
			
			// We do stuff if we have a known uid.
			hwlib::cout << "Found keychain uid!\n";
			// loop a little animation over the 4 available gpios on port 3.
			object.write_gpio( 0x81, 0x00 ); hwlib::wait_ms(200); //P30 HIGH
			object.write_gpio( 0x82, 0x00 ); hwlib::wait_ms(200); //P31 HIGH
			object.write_gpio( 0x88, 0x00 ); hwlib::wait_ms(200); //P33 HIGH
			object.write_gpio( 0xA0, 0x00 ); hwlib::wait_ms(200); //P35 HIGH
			object.write_gpio( 0x80, 0x00 ); // all back to LOW.
		}
}