#include "hwlib.hpp"

int main(){

   auto sck = hwlib::target::pin_out( hwlib::target::pins::d2 );
   auto mosi = hwlib::target::pin_out( hwlib::target::pins::d3 );
   auto miso = hwlib::target::pin_in( hwlib::target::pins::d5 );
   auto ssel = hwlib::target::pin_out( hwlib::target::pins::d4 );
   
   auto spi_bus = hwlib::spi_bus_bit_banged_sclk_mosi_miso( sck, mosi, miso );
   
   auto spi_transaction = hwlib::spi_bus::spi_transaction(spi_bus, ssel);
   
   const size_t n = 5;
   uint8_t data_out[1];
   data_out[0] = 0x02;
   uint8_t data_in[5];
   
   spi_transaction.write_and_read(n, data_out, data_in);
   
   hwlib::cout << data_out[0] << "\n";
   
   for(size_t i = 0; i < sizeof(data_in); i++){
	   hwlib::cout << data_in[i] << "\n";
   }
   
}