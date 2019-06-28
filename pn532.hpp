// ==========================================================================
//
// File      : pn532.hpp
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
#ifndef PN532_HPP
#define PN532_HPP

// Required include, since hwlib is a dependency.
#include "hwlib.hpp"

// Some defines for convenience and readability.
#define PREAMBLE 0x00
#define START_CODE_1 0x00
#define START_CODE_2 0xFF
#define TFI 0xD4
#define POSTAMBLE 0x00
#define ACK_1 0x00
#define ACK_2 0xFF
#define NACK_1 0xFF
#define NACK_2 0x00

/// \brief
/// pn532 class supporting both i2c and spi
/// \details
/// This class implements all the neccesary functions
/// for controlling an adafruit pn532 breakout board.
/// (presumably also works on the shield version of this board.)
///
/// Other pn532 boards are not tested and/or supported through
/// this library.

class pn532 {
private:

	//I2C required variables.
	hwlib::i2c_bus_bit_banged_scl_sda & i2c_bus;
	hwlib::target::pin_out & rst;
	hwlib::target::pin_in & irq;
	const uint8_t & addr;
	bool using_i2c;
	
	//SPI required variables.
	//hwlib::spi_bus::spi_transaction & spi_bus;
	
	//General functions used by other functions.
	void pn532_reset();
	void samconfig();
	void read_status_byte();
	void print( uint8_t bytes_in[], size_t & size_in );
	bool read_ack_nack();
	void read_write( uint8_t bytes_out[], size_t & size_out, size_t & size_in );

public:

	//I2C constructor.
	pn532( hwlib::i2c_bus_bit_banged_scl_sda & i2c_bus, hwlib::target::pin_out & rst, hwlib::target::pin_in & irq, const uint8_t & addr = 0x48 );
	
	//SPI constructor.
	//pn532( hwlib::spi_bus::spi_transaction & spi_bus );
	
	//Functions for both I2C and SPI.
	void get_firmware_version();
	void read_gpio();
	void write_gpio( uint8_t gpio_p3, uint8_t gpio_p7 );

}; // class pn532.

#endif // PN532_HPP