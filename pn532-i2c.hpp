// ==========================================================================
//
// File      : pn532-i2c.hpp
// Part of   : C++ library for controlling a pn532 chip over i2c or spi.
// Copyright : mike.hoogendoorn@student.hu.nl 2019
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// ==========================================================================

// This file contains Doxygen lines.
/// @file

// Multiple inclusion guards.
#ifndef PN532_I2C_HPP
#define PN532_I2C_HPP

// Required include, since hwlib is a dependency.
#include "hwlib.hpp"

// Some defines for convenience and readability.
#define PREAMBLE 0x00
#define START_CODE_1 0x00
#define START_CODE_2 0xFF
#define TFI 0xD4
#define POSTAMBLE 0x00

/// \brief
/// pn532 i2c class
/// \details
/// This class implements all the neccesary functions
/// for controlling an adafruit pn532 breakout board.
/// (presumably also works on the shield version of this board.)
///
/// Other pn532 boards are not tested and/or supported through
/// this library.

class pn532_i2c {
private:

	hwlib::target::pin_oc & scl;
	hwlib::target::pin_oc & sda;
	hwlib::target::pin_out & rst;
	hwlib::target::pin_out & irq;
	const uint8_t addr;
	hwlib::i2c_bus_bit_banged_scl_sda bus;
	
	void pn532_reset();
	void i2c_start();
	void i2c_stop();
	void samconfig();
	void read_status_byte();
	int8_t calculate_dcs ( std::array<uint8_t, 32> & bytes_out, size_t byte_length );

public:

	pn532_i2c( hwlib::target::pin_oc & scl, hwlib::target::pin_oc & sda, hwlib::target::pin_out & rst, hwlib::target::pin_out & irq, const uint8_t & addr = 0x48 );
	
	void get_firmware_version();
	void read_gpio();
	void write_gpio();
	void read_write( std::array<uint8_t, 32> & bytes_out, size_t byte_length );

}; // class pn532_i2c

#endif // PN532_I2C_HPP