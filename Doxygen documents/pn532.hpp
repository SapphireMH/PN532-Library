// ==========================================================================
//
// File      : pn532.hpp
// Part of   : C++ library for controlling a PN532 chip over I2C or SPI.
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

// ==========================================================================

// Some defines for convenience and readability.
// The following bytes are part of all communication frames
#define PREAMBLE (uint8_t) 0x00
#define START_CODE_1 (uint8_t) 0x00
#define START_CODE_2 (uint8_t) 0xFF
#define TFI (uint8_t) 0xD4
#define POSTAMBLE (uint8_t) 0x00

// ==========================================================================

// The following bytes are for reading or sending acks and nacks.
#define ACK_1 (uint8_t) 0x00
#define ACK_2 (uint8_t) 0xFF
#define NACK_1 (uint8_t) 0xFF
#define NACK_2 (uint8_t) 0x00

// ==========================================================================

// These bytes tell the SPI bus what the following information
// frame will do.

/// \brief
/// Required byte to let the SPI chip know we will read its status.
#define SPI_SR (uint8_t) 0x02

/// \brief
/// Required byte to let the SPI chip know we will write data to it.
#define SPI_DW (uint8_t) 0x01

/// \brief
/// Required byte to let the SPI chip know we will read data from it.
#define SPI_DR (uint8_t) 0x03

// ==========================================================================

// Unique command codes for each function, to be added to a
// communication frame within the corresponding function.

/// \brief
/// Command code used to receive the firmware version.
#define CC_get_firm (uint8_t) 0x02

/// \brief
/// Command code to configure the SAM.
#define CC_samconfig (uint8_t) 0x14

/// \brief
/// Command code to get a cards UID.
#define CC_get_uid (uint8_t) 0x4A

/// \brief
/// Command code to read from the GPIO.
#define CC_read_gpio (uint8_t) 0x0C

/// \brief
/// Command code to write to the GPIO.
#define CC_write_gpio (uint8_t) 0x0E

// ==========================================================================

/// \brief
/// pn532 class supporting both i2c and spi
/// \details
/// This class implements all the neccesary functions
/// for controlling an adafruit pn532 breakout board.
/// (presumably also works on the shield version of this board.)
///
/// Other pn532 boards are not tested and/or supported through
/// this library.

// ==========================================================================

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
	const bool using_i2c;
	const bool irq_present;
	
	//General functions used by other functions.
	void pn532_reset();
	void samconfig();
	void read_status_byte();
	bool read_ack_nack();
	void write( const uint8_t bytes_out[], const size_t & size_out, uint8_t timeout = 5 );
	void read( uint8_t bytes_in[], const size_t & size_in );

public:

	//I2C constructor.
	pn532( hwlib::target::pin_oc scl, hwlib::target::pin_oc sda, hwlib::target::pin_out rst, hwlib::target::pin_in irq = hwlib::target::pins::d0, const bool irq_present = false, const uint8_t & addr = 0x24 );
	
	//SPI constructor.
	pn532( hwlib::target::pin_out sclk, hwlib::target::pin_out mosi, hwlib::target::pin_in miso, hwlib::target::pin_out sel, hwlib::target::pin_out rst, hwlib::target::pin_in irq = hwlib::target::pins::d0, const bool irq_present = false );
	
	//Functions for both I2C and SPI.
	void get_firmware_version( std::array<uint8_t, 4> & firmware );
	void read_gpio( std::array<uint8_t, 3> & gpio_states );
	void write_gpio( uint8_t gpio_p3, uint8_t gpio_p7 );
	void get_card_uid( std::array<uint8_t, 7> & uid );

}; // class pn532.

#endif // PN532_HPP