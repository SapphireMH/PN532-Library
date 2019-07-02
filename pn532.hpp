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
#define SPI_SR (uint8_t) 0x02
#define SPI_DW (uint8_t) 0x01
#define SPI_DR (uint8_t) 0x03

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

	// I2C required variables.
	hwlib::target::pin_oc scl;
	hwlib::target::pin_oc sda;
	hwlib::i2c_bus_bit_banged_scl_sda i2c_bus;
	hwlib::target::pin_out rst;
	const uint8_t & addr;
	
	// SPI required variables.
	hwlib::target::pin_out sclk;
	hwlib::target::pin_out mosi;
	hwlib::target::pin_in miso;
	hwlib::target::pin_out sel;
	hwlib::spi_bus_bit_banged_sclk_mosi_miso spi_bus;
	
	// General variables.
	hwlib::target::pin_in irq;
	bool using_i2c;
	bool irq_present;
	
	//General functions used by other functions.
	void pn532_reset();
	void samconfig();
	void read_status_byte();
	bool read_ack_nack();
	void write( uint8_t bytes_out[], size_t & size_out, uint8_t timeout = 5 );
	void read( uint8_t bytes_in[], size_t & size_in );

public:

	//I2C constructor.
	pn532( hwlib::target::pin_oc scl, hwlib::target::pin_oc sda, hwlib::target::pin_out rst, hwlib::target::pin_in irq = hwlib::target::pins::d0, const uint8_t & addr = 0x48 );
	
	//SPI constructor.
	pn532( hwlib::target::pin_out sclk, hwlib::target::pin_out mosi, hwlib::target::pin_in miso, hwlib::target::pin_out sel, hwlib::target::pin_in irq = hwlib::target::pins::d0 );
	
	//Functions for both I2C and SPI.
	void get_firmware_version();
	void read_gpio( std::array<uint8_t, 3> & gpio_states );
	void write_gpio( uint8_t gpio_p3, uint8_t gpio_p7 );
	void get_card_uid( std::array<uint8_t, 7> & uid );

}; // class pn532.

#endif // PN532_HPP