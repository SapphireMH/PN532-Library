// ==========================================================================
//
// File      : pn532-i2c.cpp
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
#include "pn532-i2c.hpp"

/// \brief
/// Constructor for this class.
/// \details
/// This constructor requires 4 microcontroller pins and
/// optionally an address in case the default is not
/// 0x48, see the "main.cpp" file for reference.
/// The constructor automatically resets the chip and
/// configures it for normal operation mode.

pn532_i2c::pn532_i2c( hwlib::target::pin_oc & scl, hwlib::target::pin_oc & sda, hwlib::target::pin_out & rst, hwlib::target::pin_out & irq, const uint8_t & addr ):
	scl( scl ),
	sda( sda ),
	rst( rst ),
	irq( irq ),
	addr( addr ),
	bus( hwlib::i2c_bus_bit_banged_scl_sda( scl, sda ) )
	{
		hwlib::wait_ms( 500 );
		pn532_reset();
		samconfig();
	}

/// \brief
/// Function to reset the PN532 chip.
/// \details
/// This function is only called by the constructor at startup,
/// this is done to ensure the chip is in a known state before
/// continuing. The reset line is active low and a minimum
/// amount of 10 ms must be waited before sending commands.

void pn532_i2c::pn532_reset() {

	rst.write(true);
	rst.write(false);
	hwlib::wait_ms(400);
	rst.write(true);
	hwlib::wait_ms(10);

}

/// \brief
/// Function to send an i2c start condition.
/// \details
/// This function sends out an i2c start condition, there is one
/// already available in hwlib, however it is not used here since
/// we want more control over when it is and is not called.
	
void pn532_i2c::i2c_start() {

       sda.write( 0 ); sda.flush();
       hwlib::wait_us( 1 );
       scl.write( 0 ); scl.flush();
       hwlib::wait_us( 1 );

    }

/// \brief
/// Function to send an i2c stop condition.
/// \details
/// This function sends out an i2c stop condition, there is one
/// already available in hwlib, however it is not used here since
/// we want more control over when it is and is not called.

void pn532_i2c::i2c_stop() {

	scl.write( 0 ); scl.flush();
	hwlib::wait_us( 1 );
	sda.write( 0 ); sda.flush();
	hwlib::wait_us( 1 );
	scl.write( 1 ); scl.flush();
	hwlib::wait_us( 1 );
	sda.write( 1 ); sda.flush();
	hwlib::wait_us( 1 );
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

void pn532_i2c::samconfig() {

	std::array<uint8_t, 32> bytes_out = {0x14, 0x01, 0x00, 0x01};
	read_write(bytes_out, 4);

}

/// \brief
/// Function to wait for the READY byte.
/// \details
/// This function keeps looping until a READY byte is received.
/// (0x01) For each cycle that the PN532 is not ready yet (receiving 0x00.)
/// we send out an i2c start, check the READY byte and, if not ready, send
/// an i2c stop command. When the READY byte is received we return back
/// to the function that called this one.

void pn532_i2c::read_status_byte() {

	bool ready = false;
	uint8_t byte_ready = 0x00;
	while( !ready ) {
		i2c_start();
		bus.i2c_bus::write( addr + 1 );
		byte_ready = bus.i2c_primitives::read_byte();
		if( byte_ready == 0x01 ) {
			return;
		}
		i2c_stop();
	}
}

/// \brief
/// Function to calculate the data byte checksum.
/// \details
/// This function calculates the required checksum bite for the
/// communication frame send by the read_write function.
/// The rule for calculation is: "TFI + D1 + ... + Dn + DCS= 0x00"
/// D1 & Dn meaning every databyte between TFI and DCS.
/// DCS being the checksum byte generated through this function.

int8_t pn532_i2c::calculate_dcs( std::array<uint8_t, 32> & bytes_out, size_t byte_length ) {

	uint8_t temp = 0x00;

	temp += TFI;

	for( size_t i = 0; i < byte_length; i++ ) {
		temp += bytes_out[i];
	}

	return ~temp + 1;

}

/// \brief
/// Function to get the boards firmware version.
/// \details
/// This function sends out a command byte (0x02) with a
/// request to receive the firmware version of the board.

void pn532_i2c::get_firmware_version() {

	std::array<uint8_t, 32> bytes_out = {0x02};
	read_write( bytes_out, 1 );

}

/// \brief
/// Function to read the boards GPIO pins.
/// \details
/// TODO: add details.

void pn532_i2c::read_gpio() {

	std::array<uint8_t, 32> bytes_out = {0x0C};
	read_write( bytes_out, 1 );

}

/// \brief
/// Function to write to the boards GPIO pins.
/// \details
/// TODO: add details.

void pn532_i2c::write_gpio( uint8_t gpio_p3, uint8_t gpio_p7 ) {

	std::array<uint8_t, 32> bytes_out = {0x0E, gpio_p3, gpio_p7};
	read_write( bytes_out, 3 );

}

/// \brief
/// Function to write and read communication frames.
/// \details
/// This function receives data bytes from other functions and
/// combines these into a full frame before sending it out to
/// the PN532 over i2c or spi. it implements or uses other functions
/// for all the neccesary checks and requirements.
///
/// LCS (Length checksum) rule is: LEN + LCS = 0x00.

void pn532_i2c::read_write( std::array<uint8_t, 32> & bytes_out, size_t byte_length ) {
	
	// Define required variables.
	uint8_t LEN = byte_length + 1;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = calculate_dcs( bytes_out, byte_length );
	
	// Write the required bytes to the i2c bus.
	i2c_start();
	bus.i2c_bus::write( addr );
	bus.i2c_bus::write( PREAMBLE );
	bus.i2c_bus::write( START_CODE_1 );
	bus.i2c_bus::write( START_CODE_2 );
	bus.i2c_bus::write( LEN );
	bus.i2c_bus::write( LCS );
	bus.i2c_bus::write( TFI );
	
	//write our byte array.
	for( size_t i = 0; i < byte_length; i++ ) {
		bus.i2c_bus::write( bytes_out[0] );
	}
	
	// Write the other required bytes.
	bus.i2c_bus::write( DCS );
	bus.i2c_bus::write( POSTAMBLE );
	
	// Free the i2c bus.
	i2c_stop();
	
	// Debug print. REMOVE THIS LATER.
	hwlib::cout << "hellow world";
	
	// Ready state required before continuing.
	// This function also takes care of sending
	// the i2c start condition.
	read_status_byte();
	
	// Create buffer for received data bytes.
	uint8_t bytes_in[20];
	
	// Read bytes on the i2c bus.
	bus.i2c_primitives::read(true, bytes_in, 20);
	
	// Free the i2c bus.
	i2c_stop();
	
	// Print the received bytes.
	for(size_t i = 0; i < 20; i++){

		hwlib::cout << bytes_in[i] << "\n";

	}
}