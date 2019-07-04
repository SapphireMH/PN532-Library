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
// The following bytes are part of all communication frames.

/// \brief
/// The preamble of a communication frame.
#define PREAMBLE 0x00

/// \brief
/// Part 1 of frame start code. (frame identifier for the PN532.)
#define START_CODE_1 0x00

/// \brief
/// Part 2 of frame start code. (frame identifier for the PN532.)
#define START_CODE_2 0xFF

/// \brief
/// TFI is a byte that shows the direction of frame (0xD4 = arduino to PN532.)
#define TFI 0xD4

/// \brief
/// The postamble of a communication frame.
#define POSTAMBLE 0x00

// ==========================================================================

// The following bytes are for reading or sending acks and nacks.

/// \brief
/// First byte of an ack.
#define ACK_1 0x00

/// \brief
/// Second byte of an ack.
#define ACK_2 0xFF

/// \brief
/// First byte of a nack.
#define NACK_1 0xFF

/// \brief
/// Second byte of a nack.
#define NACK_2 0x00

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
#define CC_get_firm 0x02

/// \brief
/// Command code to configure the SAM.
#define CC_samconfig 0x14

/// \brief
/// Command code to get a cards UID.
#define CC_get_uid 0x4A

/// \brief
/// Command code to read from the GPIO.
#define CC_read_gpio 0x0C

/// \brief
/// Command code to write to the GPIO.
#define CC_write_gpio 0x0E

// ==========================================================================

/// \brief
/// Command code for eeprom data exchange. (Read or Write.)
#define CC_data_exchange 0x40

/// \brief
/// Add-on to CC_data_exchange for reading NFC card eeprom.
#define mifare_read 0x30

/// \brief
/// Add-on to CC_data_exchange for writing NFC card eeprom.
#define mifare_write 0xA0

/// \brief
/// Add-on for mifare write/read to specify which card we target. (Always 0x01.)
#define target_card 0x01

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

class pn532 {
private:

	// I2C required variables.
	hwlib::pin_oc & scl;
	hwlib::pin_oc & sda;
	hwlib::i2c_bus_bit_banged_scl_sda i2c_bus;
	hwlib::target::pin_out rst;
	const uint8_t & addr;
	
	// SPI required variables.
	hwlib::spi_bus_bit_banged_sclk_mosi_miso spi_bus;
	hwlib::pin_out & sel;
	
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
	pn532( hwlib::pin_oc & scl, hwlib::pin_oc & sda, hwlib::target::pin_out rst, hwlib::target::pin_in irq = hwlib::target::pins::d13, const bool irq_present = false, const uint8_t & addr = 0x24 );
	
	//SPI constructor.
	pn532( hwlib::spi_bus_bit_banged_sclk_mosi_miso & spi_bus, hwlib::pin_out & sel, hwlib::target::pin_out rst, hwlib::target::pin_in irq = hwlib::target::pins::d13, const bool irq_present = false );
	
	//Functions for both I2C and SPI.
	void get_firmware_version( std::array<uint8_t, 4> & firmware );
	void read_gpio( std::array<uint8_t, 3> & gpio_states );
	void write_gpio( uint8_t gpio_p3, uint8_t gpio_p7 );
	void get_card_uid( std::array<uint8_t, 7> & uid );
	void read_eeprom_block( const uint8_t blocknr );
	void write_eeprom_block( const uint8_t blocknr, const std::array<uint8_t, 16> & data );
	void read_eeprom_all();

}; // class pn532.

#endif // PN532_HPP
