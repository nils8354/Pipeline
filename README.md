**UST-3400 Pipeline Simulator**

*This program simulates a pipepline design for the UST-3400 assembly language*

FILES:
 - pipelineOverview.pdf	: an overview document outlining the project and development process
 - README.md	: description of file contents and program use
 - Makefile	: used for creation of sim executable
 - sim.c	: file containing C-code for simulator
 - ref_asm	: provided reference assembler
 - ref_sim	: provided reference simulator
 - /tests/	: contains suite of test cases

		- class.asm		: provided test case
		- class.mc		: machine code for provided test case
		- controlHazard1.asm	: code that tests only for control (beq) hazards
		- controlHazard1.mc	: machine code for controlHazard1.asm
		- givenNoHazard.asm	: tests given in assignment document
		- givenNoHazard.mc	: machine code for givenNoHazard.asm
		- loadStall.asm		: contains only load stall hazard
		- loadStall.mc		: machine code for loadStall.asm
		- multiHazard1.asm	: contains lw stall, control, and MEMWB/EXMEM data forwarding hazards
		- multiHazard1.mc	: machine code for multiHazard1.asm
		- multiHazard2.asm	: contains beq and WBEND data forwarding hazards
		- multiHazard2.mc	: machine code for multiHazard2.asm
		- multiHazard3.asm	: contains lw stall and MEMWB/EXMEM data forwarding hazards; data forwarding to multiple registers
		- multiHazard3.mc	: machine code for multiHazard3.asm
		- multiHazard4.asm	: contains lw stalls, MEMWB/EXMEM data forwarding; forwarding to multiple registers
		- multiHazard4.mc	: machine code for multiHazard4.asm
		- noHazardTest.asm	: contains all instructions with no hazards
		- noHazardTest.mc	: machine code for noHazardTest.asm

USE:
 - To create sim executable:

		$make


 - To run program:

		$./sim -i inputFileName

 - To execute test files:

		$./sim -i tests/testFileName

 - To make clean (remove temporary files):

		$make clean
