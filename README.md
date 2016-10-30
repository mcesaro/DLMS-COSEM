# DLMS-COSEM

The goal of this project is to create an open reference implementation of a DLMS-COSEM compliant communications stack. 

** IMPORTANT - This is a work in progress and is rapidly changing. ** 

To build:

mkdir bin
cd bin
cmake ..
make

To run:

Open a console.

	socat PTY,link=/tmp/ttyS10 PTY,link=/tmp/ttyS11

Open another console for the Server.

	cd [DLMS-COSEM Directory]/bin/src/Linux
	./Linux SERVER

Open another console for the Client.

	cd [DLMS-COSEM Directory]/bin/src/Linux
	./Linux

To run unit tests.

	cd [DLMS-COSEM Directory]/bin/test/DLMS-COSEM-TEST
	./DLMS-COSEM-TEST

For the STM32 Nucleo F207ZG development board:

	Prerequisites:
	
		GCC for ARM (at least 4.8 or above) installed on a WINDOWS PC.
		STM32 ST-LINK Utility from st.com installed on a WINDOWS PC.

		Set the ARM_TOOLCHAIN_ROOT environment variable to the root of where GCC is installed.

	cd [DLMS-COSEM Directory]/src/STM32-NUCLEO-F207ZG
	make

	The output will be located in [DLMS-COSEM Directory]/src/STM32-NUCLEO-F207ZG/Debug

	You can program the development board using the ST-LINK software from STM.  Select the .bin file from 
	the Debug folder.



