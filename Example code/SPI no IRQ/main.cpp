#include "pn532.hpp"

//SPI example without IRQ.

int main() {

	// Required pins to use this library with SPI.
	auto sclk = hwlib::target::pin_out( hwlib::target::pins::d2 );
	auto mosi = hwlib::target::pin_out( hwlib::target::pins::d3 );
	auto miso = hwlib::target::pin_in( hwlib::target::pins::d4 );
	auto spi_bus = hwlib::spi_bus_bit_banged_sclk_mosi_miso( sclk, mosi, miso );
	auto sel = hwlib::target::pin_out( hwlib::target::pins::d5 );
	auto rst = hwlib::target::pin_out( hwlib::target::pins::d6 );
	
	// Create the object, "object" can be replaced with any name of your choosing.
	pn532 object = pn532( spi_bus, sel, rst );
	
	// Read the boards hardware and firmware version.
	std::array<uint8_t, 4> firmware;
	object.get_firmware_version( firmware );
	
	// Read the states of the gpio ports and places them into our array.
	std::array<uint8_t, 3> gpio_states;
	object.read_gpio( gpio_states );
	
	// Waits untill a card gets in range, reads its uid and places that into our array.
	std::array<uint8_t, 7> uid;
	object.get_card_uid( uid );
	
	// Set GPIO P72 to HIGH, in my case we light up a blue LED.
	object.write_gpio( 0x00, 0x84 );
	// Set GPIO P71 to HIGH, in my case we light up a red LED.
	object.write_gpio( 0x00, 0x82 );
	// loop a little animation over the 4 available gpios on port 3.
	object.write_gpio( 0x81, 0x00 ); hwlib::wait_ms(200); //P30 HIGH
	object.write_gpio( 0x82, 0x00 ); hwlib::wait_ms(200); //P31 HIGH
	object.write_gpio( 0x88, 0x00 ); hwlib::wait_ms(200); //P33 HIGH
	object.write_gpio( 0xA0, 0x00 ); hwlib::wait_ms(200); //P35 HIGH
	object.write_gpio( 0x80, 0x00 ); // all back to LOW.
}