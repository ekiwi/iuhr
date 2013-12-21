// coding: utf-8
#include <xpcc/architecture.hpp>
using namespace xpcc::atmega;


// Serial debug
#include <xpcc/io/iodevice_wrapper.hpp>
typedef Uart0 Uart;
xpcc::IODeviceWrapper<Uart> logger;

#include <xpcc/debug/logger.hpp>
xpcc::log::Logger xpcc::log::debug(logger);
xpcc::log::Logger xpcc::log::info(logger);
xpcc::log::Logger xpcc::log::warning(logger);
xpcc::log::Logger xpcc::log::error(logger);

#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

// TLC5951
#include "tlc.hpp"

uint8_t buffer[24];

typedef GpioPort<GpioOutputC0, 6> RowPort;

MAIN_FUNCTION
{

	GpioOutputB7::setOutput(xpcc::Gpio::High);

	//GpioD0::connect(Uart::Rx);
	GpioD1::connect(Uart::Tx);
	Uart::initialize<19200>();

	xpcc::atmega::enableInterrupts();
	XPCC_LOG_INFO << "iUhr 0.1 says hello...." << xpcc::endl;

	RowPort::write(0xfe); // turn rows off except for digit 0 by default
	RowPort::setOutput();
	XPCC_LOG_DEBUG<< "RowPort initialized" << xpcc::endl;

	Tlc5951::initialize();
	XPCC_LOG_DEBUG<< "Tlc5951 initialized" << xpcc::endl;

	GpioOutputD5::setOutput(xpcc::Gpio::Low);

	// square wave at D5
	//TCCR0A = (1<<COM0B1) | (1<<WGM00) | (1<<WGM01);
	//TCCR0B = (1<<CS01) | (1<<CS00) | (1<<WGM02);


	const uint8_t x = 2;
	buffer[x] = 0xff;
	buffer[x+3] = 0xff;
	buffer[x+6] = 0xff;
	buffer[x+9] = 0xff;
	buffer[x+12] = 0xff;
	buffer[x+15] = 0xff;
	buffer[x+19] = 0xff;
	buffer[x+21] = 0xff;


	//Tlc5951::writeGrayscale(buffer);
	//Tlc5951::latch();


	XPCC_LOG_DEBUG<< "Set B0 to 0xff" << xpcc::endl;



	uint8_t row = 0;
	while (1)
	{
		//RowPort::write(~(1<<row));
		//++row;
		//row = (row > 4)? 0 : row;

		GpioOutputD5::toggle();

		xpcc::delay_us(100);
	}

	return 0;
}
