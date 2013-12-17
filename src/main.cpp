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

MAIN_FUNCTION
{

	GpioOutputB7::setOutput(xpcc::Gpio::High);

	//GpioD0::connect(Uart::Rx);
	GpioD1::connect(Uart::Tx);
	Uart::initialize<19200>();


	xpcc::atmega::enableInterrupts();
	XPCC_LOG_INFO << "iUhr 0.1 says hello...." << xpcc::endl;

	GpioOutputD5::setOutput(xpcc::Gpio::High);

	while (1)
	{
		GpioOutputD5::toggle();
		xpcc::delay_ms(1);
	}

	return 0;
}
