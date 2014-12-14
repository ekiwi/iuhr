#ifndef TLC_HPP
#define TLC_HPP
#include <xpcc/processing.hpp>
#include <xpcc/ui/led/tables.hpp>

class Tlc5951
{
	typedef ::xpcc::atmega::SpiMaster Spi;
	typedef GpioOutputB2 Gslat;
	typedef GpioOutputB1 Xblnk;


	static xpcc::accessor::Flash<uint16_t> lut;


	/// Function Data enums

	enum class
	RangeRed : uint8_t
	{
		Lower  = (0<<0),	/// constant current control    0% -  66.7%
		Higher = (1<<0),	/// constant current control 33.3% - 100.0%
	};

	enum class
	RangeGreen : uint8_t
	{
		Lower  = (0<<1),	/// constant current control    0% -  66.7%
		Higher = (1<<1),	/// constant current control 33.3% - 100.0%
	};

	enum class
	RangeBlue : uint8_t
	{
		Lower  = (0<<2),	/// constant current control    0% -  66.7%
		Higher = (1<<2),	/// constant current control 33.3% - 100.0%
	};

	enum class
	AutoDisplayRepeatMode : uint8_t
	{
		Disable = (0<<3),	/// each output driver is turned on and off once after XBLINK goes high
		Enable  = (1<<3),	/// cycle
	};

	enum class
	DisplayTimingResetMode : uint8_t
	{
		Disable = (0<<4),	/// XBLINK is needed to reset GS counter
		Enable  = (1<<4),	/// GS counter is reset to 0 at GSLAT rising edge for a GS data write
	};

	enum class
	GrayscaleCounterMode : uint8_t
	{
		Bit12 = 0,					///
		Bit10 = (1<<5),				///
		Bit8  = (1<<5) | (1<<6),	///
	};


public:

	static inline void initialize()
	{
		// Init Pins
		Gslat::setOutput(xpcc::Gpio::Low);
		Xblnk::setOutput(xpcc::Gpio::Low);
		// init SPI
		GpioOutputB3::connect(Spi::Mosi);
		GpioInputB4::connect(Spi::Miso);
		GpioOutputB5::connect(Spi::Sck);
		Spi::initialize<xpcc::avr::SystemClock, 1250000>();
		Spi::setDataMode(Spi::DataMode::Mode0);
		XPCC_LOG_DEBUG<< "Spi initialized" << xpcc::endl;
		// initialize Control
		writeControl();
		XPCC_LOG_DEBUG<< "Tlc Control initialized" << xpcc::endl;
		// initalize Channels to zero
		writeGrayscale();
		latch();
		XPCC_LOG_DEBUG<< "Tlc Grayscale initialized" << xpcc::endl;
	}

	static inline void writeGrayscale(uint8_t* data = NULL)
	{
		Gslat::reset();

		if(data) {
			uint8_t* ptr = data + (24-1); // point to last byte of 24 Channels
			uint8_t shared;
			bool shared_high = true;
			for(uint8_t ii = 0; ii < 24; ++ii){
				uint16_t buffer = lut[*ptr];
				if(shared_high) { // we share a high nibble
					Spi::transferBlocking((buffer >> 4) & 0xff);
					shared = (buffer & 0x0f) << 4;
				} else {
					shared |= (buffer >> 8) & 0x0f;
					Spi::transferBlocking(shared);
					Spi::transferBlocking(buffer & 0xff);
				}
				shared_high = !shared_high;
				--ptr;
			}
		} else { // when called with a NULL pointer => clear all channels
			for(uint8_t ii = 0; ii < 36; ++ii) {
				Spi::transferBlocking(0x00);
			}
		}
	}

	/// for Grayscale Data
	static inline void latch()
	{
		xpcc::atomic::Lock lock;
		Xblnk::reset();
		// latch in data
		Gslat::set();
		//xpcc::delay_us(1);
		__asm__ __volatile__("nop");
		__asm__ __volatile__("nop");
		Gslat::reset();
		Xblnk::set();
	}

	static inline void writeControl()
	{
		static constexpr uint8_t function =
						static_cast<uint8_t>(RangeRed::Higher) |
						static_cast<uint8_t>(RangeGreen::Higher) |
						static_cast<uint8_t>(RangeBlue::Higher) |
						static_cast<uint8_t>(AutoDisplayRepeatMode::Enable) |
						static_cast<uint8_t>(DisplayTimingResetMode::Disable) |
						static_cast<uint8_t>(GrayscaleCounterMode::Bit12);
		// We really do not want to kill our 500mA rated power supply...
		static constexpr uint8_t GlobalBrightnessBlue  = 0x4f;
		static constexpr uint8_t GlobalBrightnessGreen = 0x4f;
		static constexpr uint8_t GlobalBrightnessRed   = 0x4f;

		Gslat::set();

		// bit 287-281 padding
		// bit 280-216 read and reserved
		// bit 215-199 user data
		// => 89 blank bits
		// => 11 bytes and 1 bit

		for(uint8_t ii = 0; ii < 10; ++ii) {
			Spi::transferBlocking(0);
		}

		//XPCC_LOG_DEBUG << "Wrote empty bytes" << xpcc::endl;
		// 7 bit function control (+ MSB 0, see above)
		Spi::transferBlocking(function & 0x7f);		// bit 199-192
		// Global Brightness: 3 bytes
		Spi::transferBlocking(GlobalBrightnessBlue);	// bit 191-184
		Spi::transferBlocking(GlobalBrightnessGreen);	// bit 183-176
		Spi::transferBlocking(GlobalBrightnessRed);		// bit 175-168
		// bit 167-0: 24 Channel 7 bit dot correction => 7 bytes per channel
		// 21 bytes for Dot Correction
		for(uint8_t ii = 0; ii < 21; ++ii) {
			Spi::transferBlocking(0xff);
		}

		// in order to be sure not to display bullshit disable outputs before
		// latching in data (most of the times Xblnk will already be low, but to be sure....)
		Xblnk::reset();

		// latch in data
		Gslat::reset();
		xpcc::delayMicroseconds(1);
		Gslat::set();
		xpcc::delayMicroseconds(1);
		Gslat::reset();	// default "position" for Gslat is low
	}


};

xpcc::accessor::Flash<uint16_t>
Tlc5951::lut(xpcc::ui::table12_256);


#endif // TLC_HPP
