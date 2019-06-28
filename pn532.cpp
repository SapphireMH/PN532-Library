// ==========================================================================
//
// File      : pn532.cpp
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

// Include the matching header.
#include "pn532.hpp"

/// \brief
/// Constructor for this class.
/// \details
/// This constructor requires 4 microcontroller pins and
/// optionally an address in case the default is not
/// 0x48, see the "main.cpp" file for reference.
/// The constructor automatically resets the chip and
/// configures it for normal operation mode.

pn532::pn532( hwlib::i2c_bus_bit_banged_scl_sda & i2c_bus, hwlib::target::pin_out & rst, hwlib::target::pin_in & irq, const uint8_t & addr ):
	i2c_bus( i2c_bus ),
	rst( rst ),
	irq( irq ),
	addr( addr )
	{
		using_i2c = true;
		hwlib::wait_ms( 500 );
		pn532_reset();
		//samconfig();
	}

//pn532::pn532( hwlib::spi_bus::spi_transaction & spi_bus ):
//	spi_bus( spi_bus )
//	{
//		using_i2c = false;
//		hwlib::wait_ms( 500 );
//		pn532_reset();
//		samconfig();
//	}

/// \brief
/// Function to reset the PN532 chip.
/// \details
/// This function is only called by the constructor at startup,
/// this is done to ensure the chip is in a known state before
/// continuing. The reset line is active low and a minimum
/// amount of 10 ms must be waited before sending commands.

void pn532::pn532_reset() {

	rst.write(true);
	rst.write(false);
	hwlib::wait_ms(400);
	rst.write(true);
	hwlib::wait_ms(10);

}

/// \brief
/// Function configure the SAM for normal operation
/// \details
/// This function sends out a command to configure the SAM
/// (secure access module.) for normal operation, the first
/// byte is the command code, the second byte is which operating
/// mode we want, the third byte specifies the timeout, we leave
/// it at 0 (No timeout.) because normal operation mode does not
/// make use of it. The final byte specifies if we want to make
/// use of interupts (IRQ pin.) we use this so our programme can
/// wait for feedback on this pin instead of continuously
/// checking for the PN532 to send a READY byte. (0x01)

void pn532::samconfig() {
	
	uint8_t CC = 0x14;
	uint8_t CED[3] = {0x01, 0x00, 0x01};
	uint8_t LEN = 5;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~( TFI + CC + CED[0] + CED[1] + CED[2] ) + 1;
	
	
	uint8_t bytes_out[12] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, CED[0], CED[1], CED[2], DCS, POSTAMBLE};
	uint8_t bytes_in[8];
	
	if(using_i2c) {
		
		// write and read bytes on the i2c bus.
		i2c_bus.i2c_bus::write( addr ).write( bytes_out, 12 );
		read_status_byte();
		i2c_bus.i2c_bus::read( addr ).read( bytes_in, 8 );
		
	}
	else {
		
		//spi_bus.write_and_read(12, bytes_out, bytes_in);
		
	}

}

/// \brief
/// Function to wait for the READY byte.
/// \details
/// This function keeps looping until a READY byte (0x01) is received or 
/// (i2c only!) the IRQ pin goes high. When the READY byte is received we
/// return back to the function that called this one.

void pn532::read_status_byte() {

	bool ready = false;
	while( !ready ) {
		if( using_i2c ) {
		
			ready = !irq.read();
		
		}
		else {
			
			//implement spi
			
		}
	}
	return;	
}

/// \brief
/// Function to read the acknowledge frame.
/// \details
/// This function reads 6 bytes (the size of the ack/nack frame) and
/// compares the response to the ack template, if the received data does
/// not equal the template then the function that called this one needs
/// to be ran again.

bool pn532::read_ack_nack() {

	uint8_t ack_frame[6] = {PREAMBLE, START_CODE_1, START_CODE_2, ACK_1, ACK_2, POSTAMBLE};
	uint8_t bytes_in[7];
	
	if( using_i2c ) {
		
		i2c_bus.i2c_bus::read( addr ).read( bytes_in, 7 );
		
	}
	else {
		
		//implement spi
		
	}
	
	for( size_t i = 0; i < 6; i++ ) {
		
		hwlib::cout << ack_frame[i] << ":" << bytes_in[i] << "\n";
		
		if( bytes_in[i + 1] != ack_frame[i] ) {
			
			return false;
			
		}
		
	}
	return true;
}

void pn532::read_write( uint8_t bytes_out[], size_t & size_out, size_t & size_in ) {

	uint8_t bytes_in[ size_in ];

	if( using_i2c) {
		
		// Write and read bytes on the i2c bus.
		i2c_bus.i2c_bus::write( addr ).write( bytes_out, size_out );
		read_status_byte();
		while( !read_ack_nack() ) {
			
			i2c_bus.i2c_bus::write( addr ).write( bytes_out, size_out );
			
		}
		i2c_bus.i2c_bus::read( addr ).read( bytes_in, size_in );
	}
	else {
		
		//spi_bus.write_and_read(9, bytes_out, bytes_in);
		
	}

	print( bytes_in, size_in );

}

void pn532::print( uint8_t bytes_in[], size_t & size_in ) {

	for( size_t i = 0; i < size_in; i++ ) {
		
		hwlib::cout << bytes_in[i] << " ";
		
	}

}

/// \brief
/// Function to get the boards firmware version.
/// \details
/// This function sends out a command byte (0x02) with a
/// request to receive the firmware version of the board.

void pn532::get_firmware_version() {

	uint8_t CC = 0x02;
	uint8_t LEN = 2;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~(TFI + CC) + 1;
	
	
	uint8_t bytes_out[9] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, DCS, POSTAMBLE};
	size_t size_out = 9;
	size_t size_in = 13;
	
	read_write( bytes_out, size_out, size_in );

}

/// \brief
/// Function to read the boards GPIO pins.
/// \details
/// TODO: add details.

void pn532::read_gpio() {

	uint8_t CC = 0x0C;
	uint8_t LEN = 2;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~(TFI + CC) + 1;
	
	
	uint8_t bytes_out[9] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, DCS, POSTAMBLE};
	uint8_t bytes_in[20];
	
	if( using_i2c) {
		
		// Write and read bytes on the i2c bus.
		i2c_bus.i2c_bus::write( addr ).write( bytes_out, 9 );
		i2c_bus.i2c_bus::read( addr ).read( bytes_in, 20 );
		
	}
	else {
		
		//spi_bus.write_and_read(9, bytes_out, bytes_in);
		
	}

}

/// \brief
/// Function to write to the boards GPIO pins.
/// \details
/// TODO: add details.

void pn532::write_gpio( uint8_t gpio_p3, uint8_t gpio_p7 ) {

	uint8_t CC = 0x0C;
	uint8_t LEN = 4;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~( TFI + CC + gpio_p3 + gpio_p7 ) + 1;
	
	
	uint8_t bytes_out[11] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, gpio_p3, gpio_p7, DCS, POSTAMBLE};
	uint8_t bytes_in[20];

	if( using_i2c ) {

		// Write and read bytes on the i2c bus.
		i2c_bus.i2c_bus::write( addr ).write( bytes_out, 11 );
		i2c_bus.i2c_bus::read( addr ).read( bytes_in, 20 );

	}
	else {
		
		//spi_bus.write_and_read(11, bytes_out, bytes_in);
		
	}

}