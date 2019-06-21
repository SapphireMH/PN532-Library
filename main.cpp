#include "pn532-i2c.hpp"

int main() {

	auto rst = hwlib::target::pin_out( hwlib::target::pins::d3 );
	auto scl = hwlib::target::pin_oc( hwlib::target::pins::scl );
	auto sda = hwlib::target::pin_oc( hwlib::target::pins::sda );
	auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda( scl,sda );
	
	pn532_i2c pn532 = pn532_i2c( i2c_bus, rst );

	pn532.get_firmware_version();

}