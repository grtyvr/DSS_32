#ifndef as5048_h
#define as5048_h
#define LIBRARY_VERSION 1.0.1

#include <SPI.h>

class AS5048A{

	bool errorFlag;
	byte _cs;
	byte cs;
	byte dout;
	byte din;
	byte clk;
	word position;
	word transaction(word data);

	SPISettings settings;

	public:

	/**
	 *	Constructor
	 */
	AS5048A();
	AS5048A(byte arg_cs);

	/**
	 * Initialiser
	 * Sets up the SPI interface
	 */
	void init();

	/**
	 * Closes the SPI connection
	 */
	void close();

	/*
	 * Read a register from the sensor
	 * Takes the address of the register as a 16 bit word
	 * Returns the value of the register
	 */
	word read(word registerAddress);
	word read(word registerAddress, byte arg_cs);

	/*
	 * Write to a register
	 * Takes the 16-bit  address of the target register and the 16 bit word of data
	 * to be written to that register
	 * Returns the value of the register after the write has been performed. This
	 * is read back from the sensor to ensure a sucessful write.
	 */
	word write(word registerAddress, word data);
	word write(word registerAddress, word data, byte arg_cs);

	/**
	 * Get the rotation of the sensor relative to the zero position.
	 *
	 * @return {int} between -2^13 and 2^13
	 */
	int getRotation();
	int getRotation(byte arg_cs);

	/**
	 * Returns the raw angle directly from the sensor
	 */
	word getRawRotation();
	word getRawRotation(byte arg_cs);

	/**
	 * returns the value of the state register
	 * @return 16 bit word containing flags
	 */
	word getState();
	word getState(byte arg_cs);

	/**
	 * Print the diagnostic register of the sensor
	 */
	void printState();
	void printState(byte arg_cs);

	/**
	 * Returns the value used for Automatic Gain Control (Part of diagnostic
	 * register)
	 */
	byte getGain();
	byte getGain(byte arg_cs);

	/*
	 * Get and clear the error register by reading it
	 */
	word getErrors();
	word getErrors(byte arg_cs);

	/*
	 * Set the zero position
	 */
	void setZeroPosition(word arg_position);

	/*
	 * Returns the current zero position
	 */
	word getZeroPosition();
	word getZeroPosition(byte arg_cs);

	/*
	 * Check if an error has been encountered.
	 */
	bool error();
	bool error(byte arg_cs);

	private:

	byte spiCalcEvenParity(word);
};
#endif
