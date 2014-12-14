#ifndef IUHR_DISPLAY
#define IUHR_DISPLAY


namespace iuhr
{

template<typename Tlc, typename RowPort, size_t Width>
class
Display
{
private:
	static constexpr size_t
	Height = RowPort::width;

	static uint8_t
	buffers[2][Height][Width*3];

	static uint8_t
	front_buffer;

	static uint8_t
	back_buffer;

	static uint8_t
	row;


public:
	template< class clockSource, uint32_t baudrate>
	static inline void
	initialize()
	{
		RowPort::write(0xff); // turn rows off
		RowPort::setOutput();
		XPCC_LOG_DEBUG<< "RowPort initialized" << xpcc::endl;
		Tlc::template initialize<clockSource, baudrate>();
		XPCC_LOG_DEBUG<< "Tlc5951 initialized" << xpcc::endl;
	}

	static inline void
	setPixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t x, uint8_t y)
	{
		if(x >= Width)  return;
		if(y >= Height) return;
		buffers[back_buffer][y][x]   = red;
		buffers[back_buffer][y][x+1] = green;
		buffers[back_buffer][y][x+2] = blue;
	}

	static inline void
	swapBuffer()
	{
		front_buffer = (front_buffer == 0)? 1 : 0;
		back_buffer  = (back_buffer  == 0)? 1 : 0;
	}

	/// Should only be called from an interrupt
	static inline void
	displayNextRow()
	{
		Tlc::writeGrayscale(buffers[front_buffer][row]);	// write one row to tlc
		RowPort::write(0xff);		// turn off all rows
		Tlc::latch();				// latch in new data
		RowPort::write(~(1<<row));	// turn row back on
		// calculate and remember next row
		++row;
		row = (row > Height)? 0 : row;
	}

};

template<typename Tlc, typename RowPort, size_t Width>
uint8_t
Display<Tlc, RowPort, Width>::buffers[2][Display<Tlc, RowPort, Width>::Height][Width*3];

template<typename Tlc, typename RowPort, size_t Width>
uint8_t
Display<Tlc, RowPort, Width>::front_buffer;

template<typename Tlc, typename RowPort, size_t Width>
uint8_t
Display<Tlc, RowPort, Width>::back_buffer = 1;

template<typename Tlc, typename RowPort, size_t Width>
uint8_t
Display<Tlc, RowPort, Width>::row;

}

#endif	// IUHR_DISPLAY
