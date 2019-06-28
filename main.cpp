#include "pn532.hpp"

int main() {

	auto scl = hwlib::target::pin_oc( hwlib::target::pins::scl );
	auto sda = hwlib::target::pin_oc( hwlib::target::pins::sda );
	auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda( scl, sda );
	auto rst = hwlib::target::pin_out( hwlib::target::pins::d3 );
	auto irq = hwlib::target::pin_in( hwlib::target::pins::d2 );
	
	pn532 object = pn532( i2c_bus, rst, irq );

	//object.get_firmware_version();
	object.write_gpio( 0x00, 0x86 );

}