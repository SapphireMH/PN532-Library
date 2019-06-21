#include "pn532-i2c.hpp"

pn532_i2c::pn532_i2c( hwlib::i2c_bus_bit_banged_scl_sda & bus, const uint8_t & addr ):
	bus( bus ),
	addr ( addr )
	{
		hwlib::wait_ms(500);
	}

int8_t pn532_i2c::calculate_dcs( std::array<uint8_t, 64> & bytes_out, size_t byte_length ) {

	uint8_t temp = 0x00;

	temp += TFI;

	for( size_t i = 0; i < sizeof(bytes_out); i++ ) {
		temp += bytes_out[i];
	}

	return temp * -1;

}

void pn532_i2c::get_firmware_version() {

	std::array<uint8_t, 64> bytes_out = {0x02};
	read_write(bytes_out, 1);

}

void pn532_i2c::wait_for_ready() {

	bool ready = false;
	while( !ready ) {
		bus.write_start();
		if( bus.read_byte() == 0x01 ) {
			ready = true;
		}
		bus.write_stop();
	}

}

void pn532_i2c::read_write( std::array<uint8_t, 64> & bytes_out, size_t byte_length ) {
	
	//define required variables.
	uint8_t LEN = byte_length + 1;
	int8_t LCS = LEN * -1;
	int8_t DCS = calculate_dcs( bytes_out, byte_length );
	
	//write the required bytes to the i2c bus.
	bus.write_start();
	bus.i2c_bus::write( addr );
	bus.i2c_bus::write( PREAMBLE );
	bus.i2c_bus::write( START_CODE_1);
	bus.i2c_bus::write( START_CODE_2 );
	bus.i2c_bus::write( LEN );
	bus.i2c_bus::write( LCS );
	bus.i2c_bus::write( TFI );
	
	//write our byte array.
	for( size_t i = 0; i < byte_length; i++ ) {
		bus.i2c_bus::write( bytes_out[0] );
	}
	
	bus.i2c_bus::write( DCS );
	bus.i2c_bus::write( POSTAMBLE );
	bus.write_stop();
	
	wait_for_ready();
	
	// add read.

}