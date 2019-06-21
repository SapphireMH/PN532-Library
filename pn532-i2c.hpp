#ifndef PN532_I2C_HPP
#define PN532_I2C_HPP

#include "hwlib.hpp"

#define PREAMBLE 0x00
#define START_CODE_1 0x00
#define START_CODE_2 0xFF
#define TFI 0xD4
#define POSTAMBLE 0x00

class pn532_i2c {
private:

	hwlib::i2c_bus_bit_banged_scl_sda & bus;
	const uint8_t addr;

public:

	pn532_i2c( hwlib::i2c_bus_bit_banged_scl_sda & bus, const uint8_t & addr = 0x48 );

	void get_firmware_version();
	int8_t calculate_dcs ( std::array<uint8_t, 64> & bytes_out, size_t byte_length );
	void wait_for_ready();
	void read_write( std::array<uint8_t, 64> & bytes_out, size_t byte_length );

};

#endif // PN532_I2C_HPP