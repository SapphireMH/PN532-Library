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
/// Constructor for this class using I2C.
/// \details
/// This constructor requires 3 microcontroller pins: scl, sda & reset.
/// Its recommended but not required to also pass a 4th pin, IRQ to reduce
/// the amount of traffic on the i2c bus. An address can also be passed
/// in case the default is not 0x48, see the examples for reference.
/// The constructor automatically resets the chip and
/// configures it for normal operation mode.

pn532::pn532( hwlib::target::pin_oc scl, hwlib::target::pin_oc sda, hwlib::target::pin_out rst, hwlib::target::pin_in irq, const bool irq_present, const uint8_t & addr ):
	scl( scl ),
	sda( sda ),
	i2c_bus ( hwlib::i2c_bus_bit_banged_scl_sda( scl, sda ) ),
	rst( rst ),
	addr( addr  ),
	sclk( hwlib::target::pins::d0 ),
	mosi( hwlib::target::pins::d0 ),
	miso( hwlib::target::pins::d0 ),
	sel( hwlib::target::pins::d0 ),
	spi_bus( hwlib::spi_bus_bit_banged_sclk_mosi_miso( sclk, mosi, miso) ),
	irq( irq ),
	using_i2c( true ),
	irq_present( irq_present )
	{
		pn532_reset();
		samconfig();
		// Initialise all usable GPIO ports to default LOW.
		write_gpio( 0x94, 0x80 );
	}

/// \brief
/// Constructor for this class using SPI.
/// \details
/// This constructor requires 4 microcontroller pins: sclk, mosi, miso & sel.
/// Its recommended but not required to also pass a 5th pin, IRQ to reduce
/// the amount of traffic on the SPI bus. The constructor automatically resets
/// the chip and configures it for normal operation mode.

pn532::pn532( hwlib::target::pin_out sclk, hwlib::target::pin_out mosi, hwlib::target::pin_in miso, hwlib::target::pin_out sel, hwlib::target::pin_in irq, const bool irq_present ):
	scl( hwlib::target::pins::d0 ),
	sda( hwlib::target::pins::d0 ),
	i2c_bus ( hwlib::i2c_bus_bit_banged_scl_sda( scl, sda ) ),
	rst( hwlib::target::pins::d0 ),
	addr( 0 ),
	sclk( sclk ),
	mosi( mosi ),
	miso( miso ),
	sel( sel ),
	spi_bus( hwlib::spi_bus_bit_banged_sclk_mosi_miso( sclk, mosi, miso) ),
	irq( irq ),
	using_i2c( false ),
	irq_present( irq_present )
	{
		pn532_reset();
		samconfig();
		// Initialise all usable GPIO ports to default LOW.
		write_gpio( 0x94, 0x00 );
	}

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
	
	const size_t size_out = 12;
	const size_t size_in = 9;
	const uint8_t bytes_out[ size_out ] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, CED[0], CED[1], CED[2], DCS, POSTAMBLE};
	uint8_t bytes_in[ size_in ];
	
	write( bytes_out, size_out );
	read( bytes_in, size_in );

}

/// \brief
/// Function to wait for the READY byte.
/// \details
/// This function keeps looping until a READY byte (0x01) is received or 
/// the IRQ pin goes high. When the READY byte is received we
/// return back to the function that called this one.

void pn532::read_status_byte() {

	uint8_t ready = 0x00;
	while( ready != 0x01 ) {
		
		if( irq_present ) {
			
			ready = !irq.read();
			
		}
		else {
		
			if( using_i2c ) {
				
				i2c_bus.i2c_bus::read( addr ).read( ready );
				
			}
			else {
				
				hwlib::spi_bus::spi_transaction spi_transaction = spi_bus.transaction( sel );
				spi_transaction.write( SPI_SR );
				ready = spi_transaction.read_byte();
				
			}
		}
	}
}

/// \brief
/// Function to read the acknowledge frame.
/// \details
/// This function reads 7 bytes (the size of the ack/nack frame + the ready
/// byte, which we ignore.) and compares the response to the ack template,
/// if the received data does not equal the template then the command will
/// resend untill we timeout (default 5 tries.).

bool pn532::read_ack_nack() {

	uint8_t ack_frame[6] = {PREAMBLE, START_CODE_1, START_CODE_2, ACK_1, ACK_2, POSTAMBLE};
	uint8_t bytes_in[7];
	
	if( using_i2c ) {
		
		i2c_bus.i2c_bus::read( addr ).read( bytes_in, 7 );
		
	}
	else {
		
		hwlib::spi_bus::spi_transaction spi_transaction = spi_bus.transaction( sel );
		spi_transaction.write( SPI_DR );
		spi_transaction.read( 7, bytes_in );
		
	}
	
	for( size_t i = 0; i < 6; i++ ) {
		
		if( bytes_in[i + 1] != ack_frame[i] ) {
			
			return false;
			
		}
		
	}
	return true;
}

/// \brief
/// Function to read and write data to/from the pn532
/// \details
/// This function writes bytes_out[] to the pn532, the amount of bytes
/// written is decided by the variable size_out and the amount of bytes
/// read is decided by the variable size_in.
///
/// This function contains a retry loop for when the pn532
/// does not acknowledge the received command, the timeout
/// is 5 retries.

void pn532::write( const uint8_t bytes_out[], const size_t & size_out, uint8_t timeout ) {
	
	if( using_i2c ) {
		
		// Write and read bytes on the i2c bus.
		i2c_bus.i2c_bus::write( addr ).write( bytes_out, size_out );
		read_status_byte();
		while( !read_ack_nack() ) {
			
			i2c_bus.i2c_bus::write( addr ).write( bytes_out, size_out );
			read_status_byte();
			timeout -= 1;
			if( timeout <= 0 ) {
				return;
			}
			
		}
	}
	else {
		
		hwlib::spi_bus::spi_transaction spi_transaction = spi_bus.transaction( sel );
		spi_transaction.write( SPI_DW );
		spi_transaction.write( size_out, bytes_out );
		read_status_byte();
		while( !read_ack_nack() ) {
			
			spi_transaction.write( SPI_DW );
			spi_transaction.write( size_out, bytes_out );
			read_status_byte();
			timeout -= 1;
			if( timeout <= 0 ) {
				return;
			}
		}
	}

}

void pn532::read( uint8_t bytes_in[], const size_t & size_in ) {

	read_status_byte();
	if( using_i2c) {
		
		i2c_bus.i2c_bus::read( addr ).read( bytes_in, size_in );
		
	}
	else {
		
		hwlib::spi_bus::spi_transaction spi_transaction = spi_bus.transaction( sel );
		spi_transaction.write( SPI_DR );
		spi_transaction.read( size_in, bytes_in );
		
	}
}

/// \brief
/// Function to get the boards firmware version.
/// \details
/// This function sends out a command byte (0x02) with a
/// request to receive the firmware version of the board.
/// This function reads 4 bytes, these bytes get printed to
/// console and returned as std::array so that they can be used
/// for other functionality.
///
/// The first byte is the IC version, probably 0x32.
/// The second byte is the firmware version, probably 0x01.
/// The 3th byte is the firmware revision.
/// the 4th byte "support" tells which card types this chip supports
/// 1 = ISO/IEC 14443 TypeA, 2 = ISO/IEC 14443 TypeB,
/// 3 = SO18092, any higher number then the previous 3 means a
/// combination, for example: 7 means that all 3 are supported.

void pn532::get_firmware_version( std::array<uint8_t, 4> & firmware ) {

	uint8_t CC = 0x02;
	uint8_t LEN = 2;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~(TFI + CC) + 1;
	
	const size_t size_out = 9;
	const size_t size_in = 13;
	const uint8_t bytes_out[ size_out ] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, DCS, POSTAMBLE};
	uint8_t bytes_in[ size_in ];
	
	write( bytes_out, size_out );
	read( bytes_in, size_in );
	
	hwlib::cout << hwlib::hex << "PN532 firmware version: " << bytes_in[9] << " firmware revision: " << bytes_in[10] << "\n";
	hwlib::cout << hwlib::hex << "PN532 IC version: " << bytes_in[8] << " Supporting: " << bytes_in[11] << "\n\n";
	
	for( size_t i = 8; i < 12; i++ ) {
		
		//firmware[ i - 8 ] = bytes_in[i];

	}
}

/// \brief
/// Function to read the boards GPIO pins.
/// \details
/// WARNING: All gpio pins remember their last state, resetting the pn532 or the
/// arduino does not change this, therefor this library sets all usable GPIO's
/// to LOW at startup to make sure they are in a known state.
///
/// This function returns the states of the 3 gpio ports in the formats shown below.
/// GPIO port 7 can only be used with the i2c protocol since these ports are
/// shared with the spi bus, therefor the second byte will not contain any usable data
/// when using SPI. GPIO port 3 numbers 32 and 34 are reserved and will therefor always
/// read as HIGH. The I0I1 byte will always read as 00000001 for I2c and 00000010 for SPI.
///
/// gpio port 3 format:
/// 0, 0, P35, P34, P33, P32, P31, P30
///
/// (I2C ONLY!) gpio port 7 format:
/// 0, 0, 0, 0, 0, P72, P71, 0
///
/// I0I1 (interface select jumpers.) format:
/// 0, 0, 0, 0, 0, 0, SEL0, SEL1

void pn532::read_gpio( std::array<uint8_t, 3> & gpio_states ) {

	uint8_t CC = 0x0C;
	uint8_t LEN = 2;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~(TFI + CC) + 1;
	
	const size_t size_out = 9;
	const size_t size_in = 12;
	const uint8_t bytes_out[ size_out ] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, DCS, POSTAMBLE};
	uint8_t bytes_in[ size_in ];
	
	write( bytes_out, size_out );
	read( bytes_in, size_in );
	
	hwlib::cout << "GPIO states:\n";
	hwlib::cout << "P3: " << bytes_in[8] << "\nP7: " << bytes_in[9] << "\n";
	
	if( bytes_in[10] == 1 ) {
		hwlib::cout << "SEl0 ON / SEL1 OFF\n\n"; 
	}
	else {
		hwlib::cout << "SEl0 OFF / SEL1 ON\n\n";
	}
	
	for( size_t i = 8; i < 11; i++ ) {
		
//		gpio_states[ i - 8 ] = bytes_in[i];
		
	}
}

/// \brief
/// Function to write to the boards GPIO pins.
/// \details
/// WARNING: All gpio pins are HIGH by default, keep this in mind!
///
/// This function allows you to turn the extra gpio of the pn532 to high or low.
/// GPIO port 7 can only be used with the i2c protocol since these ports are
/// shared with the spi bus. GPIO port 3 numbers 32 and 34 are reserved and should
/// be high at all times. The arguments are required to be either in decimal or
/// hexadecimal format.
///
/// for gpio_p3 use the following byte format:
/// EN, NU, P35, P34, P33, P32, P31, P30
///
/// (I2C ONLY!) For gpio_p7 use the following format:
/// EN, NU, NU, NU, NU, P72, P71, 0
///
/// EN is enable, set this bit high to use this port.
/// NU means not used, the value on this bit does not matter.
/// each P number corresponds with a physical port on the board.

void pn532::write_gpio( uint8_t gpio_p3, uint8_t gpio_p7 ) {

	// Safety check, gpio port 7 is shared with the spi bus, so when using spi, set port 7 to dont touch.
	if( !using_i2c ) {
		gpio_p7 = 0x00;
	}
	
	// Safety check, p32 and p34 are reserved and must always be high (1).
	gpio_p3 = gpio_p3 | 0x14;
	
	uint8_t CC = 0x0E;
	uint8_t LEN = 4;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~( TFI + CC + gpio_p3 + gpio_p7 ) + 1;
	
	const size_t size_out = 11;
	const size_t size_in = 9;
	const uint8_t bytes_out[ size_out ] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, gpio_p3, gpio_p7, DCS, POSTAMBLE};
	uint8_t bytes_in[ size_in ];
	
	write( bytes_out, size_out );
	read( bytes_in, size_in );

}

/// \brief
/// Function to read the boards GPIO pins.
/// \details
/// This function waits for a card to enter the the pn532's range and then reads its uid.
/// 4 or 7 byte uid cards are supported, however 4 byte uid's are padded with 3 0x00's at
/// its end. The uid gets printed to cout and can then be used in other functions. for
/// authentication or triggering other actions using the uid, an example of this is
/// available, see the main.cpp in the implementation folder.

void pn532::get_card_uid( std::array<uint8_t, 7> & uid ) {

	uint8_t CC = 0x4A;
	uint8_t MaxTg = 0x01;
	uint8_t BrTy = 0x00;
	uint8_t LEN = 4;
	uint8_t LCS = ~LEN + 1;
	uint8_t DCS = ~(TFI + CC + MaxTg + BrTy) + 1;
	
	const size_t size_out = 11;
	const size_t size_in = 22;
	const uint8_t bytes_out[ size_out ] = {PREAMBLE, START_CODE_1, START_CODE_2, LEN, LCS, TFI, CC, MaxTg, BrTy, DCS, POSTAMBLE};
	uint8_t bytes_in[ size_in ];
		
	write( bytes_out, size_out );
	hwlib::cout << "Waiting for NFC card.\n";
	read_status_byte();
	hwlib::cout << "NFC card found!\n";
	read( bytes_in, size_in );
	
	hwlib::cout << "Length of card UID: " << bytes_in[13] << "\n";
	hwlib::cout << "UID:";
	if( bytes_in[13] == 4 ) { // Check if the UID length is the most common 4 bytes.
		
		for( size_t i = 14; i < 18; i++ ) {
			
			uid[ i - 14 ] = bytes_in[i];
			hwlib::cout << hwlib::hex << " " << bytes_in[i];
			
		}
		hwlib::cout << "\n";
		// Pad the 4 byte uid with zeros.
		uid[4] = 0x00; uid[5] = 0x00; uid[6] = 0x00;
	}
	else { // Assume a size of 7 bytes.
		
		for( size_t i = 14; i < 21; i++ ) {
			
			uid[ i - 14 ] = bytes_in[i];
			hwlib::cout << hwlib::hex << " " << bytes_in[i];
			
		}
		hwlib::cout << "\n\n";
	}
}