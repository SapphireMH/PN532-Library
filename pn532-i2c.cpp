#include "pn532-i2c.hpp"

pn532_i2c::pn532_i2c( hwlib::i2c_bus_bit_banged_scl_sda & bus, hwlib::target::pin_out & rst, const uint8_t & addr ):
	bus( bus ),
	rst ( rst ),
	addr ( addr )
	{
//		rst.write(true);
//		rst.write(false);
		hwlib::wait_ms(500);
//		rst.write(true);
		hwlib::wait_ms(10);
	}

int8_t pn532_i2c::calculate_dcs( std::array<uint8_t, 32> & bytes_out, size_t byte_length ) {

	uint8_t temp = 0x00;

	temp += TFI;

	for( size_t i = 0; i < sizeof(bytes_out); i++ ) {
		temp += bytes_out[i];
	}

	return ~temp + 1;

}

void pn532_i2c::get_firmware_version() {

	std::array<uint8_t, 32> bytes_out = {0x02};
	read_write(bytes_out, 1);

}

void pn532_i2c::read_write( std::array<uint8_t, 32> & bytes_out, size_t byte_length ) {
	
	//define required variables.
	//uint8_t LEN = byte_length + 1;
	//uint8_t LCS = ~LEN + 1;
	//uint8_t DCS = calculate_dcs( bytes_out, byte_length );
	
	//write the required bytes to the i2c bus.
	bus.write_start();
	//bus.i2c_bus::write( addr );
//	bus.i2c_bus::write( PREAMBLE );
//	bus.i2c_bus::write( START_CODE_1 );
//	bus.i2c_bus::write( START_CODE_2 );
	//bus.i2c_bus::write( LEN );
//	bus.i2c_bus::write( LCS );
//	bus.i2c_bus::write( TFI );
	
	//write our byte array.
//	for( size_t i = 0; i < byte_length; i++ ) {
//		bus.i2c_bus::write( bytes_out[0] );
//	}
//	
//	bus.i2c_bus::write( DCS );
	bus.i2c_bus::write( POSTAMBLE );
	bus.write_stop();
	
	hwlib::cout << "hellow world";

	bus.write_start();
	bus.i2c_bus::write( addr + 1 );
	uint8_t bytes_in[20];
	bus.i2c_primitives::read(true, bytes_in, 20);
	bus.write_stop();
	
	for(size_t i = 0; i < 20; i++){
		hwlib::cout << bytes_in[i] << "\n";
	}
	
	// add read.

}