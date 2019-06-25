#include "pn532-i2c.hpp"

int main() {

	auto scl = hwlib::target::pin_oc( hwlib::target::pins::scl );
	auto sda = hwlib::target::pin_oc( hwlib::target::pins::sda );
	auto rst = hwlib::target::pin_out( hwlib::target::pins::d3 );
	auto irq = hwlib::target::pin_out( hwlib::target::pins::d2 );
	
	pn532_i2c pn532 = pn532_i2c( scl, sda, rst, irq );

	pn532.get_firmware_version();

}