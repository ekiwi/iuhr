// coding: utf-8
#include <xpcc/architecture.hpp>

// Serial debug
#include <xpcc/io/iodevice_wrapper.hpp>
typedef xpcc::atmega::Uart0 Uart;
xpcc::IODeviceWrapper<Uart> logger;

#include <xpcc/debug/logger.hpp>
xpcc::log::Logger xpcc::log::debug(logger);
xpcc::log::Logger xpcc::log::info(logger);
xpcc::log::Logger xpcc::log::warning(logger);
xpcc::log::Logger xpcc::log::error(logger);


// Settings
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG
static constexpr size_t Digits   = 5;
static constexpr size_t Segments = 7;


// TLC5951
#include "xpcc/tlc.hpp"
#include "iuhr/display.hpp"


using namespace xpcc::atmega;
static constexpr size_t Rows = Digits + 1;	// One additional Row for the dots
typedef Tlc5951 Tlc;	// TODO: hand Spi and Pins to Tlc
typedef GpioPort<GpioOutputC0, Rows> RowPort;
typedef iuhr::Display<Tlc, RowPort, Segments> Display;


MAIN_FUNCTION
{

	// keep on
	GpioOutputB7::setOutput(xpcc::Gpio::High);


	// Trigger for Logic Analyzer
	GpioOutputD5::setOutput(xpcc::Gpio::Low);
	xpcc::delayMicroseconds(100);
	GpioOutputD5::set();			// Trigger!!
	xpcc::delayMicroseconds(100);


	//GpioD0::connect(Uart::Rx);
	GpioD1::connect(Uart::Tx);
	Uart::initialize<xpcc::avr::SystemClock, 19200>();

	xpcc::atmega::enableInterrupts();
	XPCC_LOG_INFO << "iUhr 0.1 says hello...." << xpcc::endl;

	Display::initialize<xpcc::avr::SystemClock, 1250000>();
	Display::setPixel(0xff, 0, 0);
	Display::setPixel(0xff, 0, 1);
	Display::setPixel(0xff, 0, 2);
//	Display::swapBuffer();

	// square wave at D5
	//TCCR0A = (1<<COM0B1) | (1<<WGM00) | (1<<WGM01);
	//TCCR0B = (1<<CS01) | (1<<CS00) | (1<<WGM02);


	while (1)
	{
		GpioOutputD5::toggle();
		Display::displayNextRow();
		xpcc::delayMicroseconds(100);
	}

	return 0;
}
