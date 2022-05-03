#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR – not implemented in this project */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

int signExtend(int num){
        // convert a 16-bit number into a 32-bit integer
        if (num & (1<<15) ) {
                num -= (1<<16);
        }
        return num;
}

typedef struct IFIDstruct{ // Fetch Decode struct
	int instr;
	int pcplus1;
} IFIDType;

typedef struct IDEXstruct{ // Decode Execute struct
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
} IDEXType;

typedef struct EXMEMstruct{ // Execute Memory struct
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
} EXMEMType;

typedef struct MEMWBstruct{ // Memory Writeback struct
	int instr;
	int writedata;
} MEMWBType;

typedef struct WBENDstruct{ // Writeback End struct
	int instr;
	int writedata;
} WBENDType;

typedef struct statestruct{
	int pc;
	int instrmem[NUMMEMORY];
	int datamem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles;       /* Number of cycles run so far */
	int fetched;     /* Total number of instructions fetched */
	int retired;      /* Total number of completed instructions */
	int branches;  /* Total number of branches executed */
	int mispreds;  /* Number of branch mispredictions*/
} statetype;


int field0(int instruction){
	return( (instruction>>19) & 0x7);
}

int field1(int instruction){
	return( (instruction>>16) & 0x7);
}

int field2(int instruction){
	return(instruction & 0xFFFF);
}

int opcode(int instruction){
	return(instruction>>22);
}

void printInstruction(int instr){
	char opcodeString[10];
	if (opcode(instr) == ADD) {
		strcpy(opcodeString, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodeString, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodeString, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodeString, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodeString, "beq");
	} else if (opcode(instr) == JALR) {
		strcpy(opcodeString, "jalr");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodeString, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodeString, "noop");
	} else {
		strcpy(opcodeString, "data");
	}
	if(opcode(instr) == ADD || opcode(instr) == NAND){
                printf("%s %d %d %d\n", opcodeString, field2(instr), field0(instr), field1(instr));
        }
        else if(0 == strcmp(opcodeString, "data")){
                printf("%s %d\n", opcodeString, signExtend(field2(instr)));
        }
        else{
                printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
                                signExtend(field2(instr)));
        }
}

void printstate(statetype *stateptr){
	int i;
	printf("\n@@@\nstate before cycle %d starts\n", stateptr->cycles);
	printf("\tpc %d\n", stateptr->pc);

	printf("\tdata memory:\n");
		for (i=0; i<stateptr->numMemory; i++) {
			printf("\t\tdataMem[ %d ] %d\n", i, stateptr->datamem[i]);
		}
	printf("\tregisters:\n");
		for (i=0; i<NUMREGS; i++) {
			printf("\t\treg[ %d ] %d\n", i, stateptr->reg[i]);
		}
	printf("\tIFID:\n");
		printf("\t\tinstruction ");
		printInstruction(stateptr->IFID.instr);
		printf("\t\tpcPlus1 %d\n", stateptr->IFID.pcplus1);
	printf("\tIDEX:\n");
		printf("\t\tinstruction ");
		printInstruction(stateptr->IDEX.instr);
		printf("\t\tpcPlus1 %d\n", stateptr->IDEX.pcplus1);
		printf("\t\treadRegA %d\n", stateptr->IDEX.readregA);
		printf("\t\treadRegB %d\n", stateptr->IDEX.readregB);
		printf("\t\toffset %d\n", stateptr->IDEX.offset);
	printf("\tEXMEM:\n");
		printf("\t\tinstruction ");
		printInstruction(stateptr->EXMEM.instr);
		printf("\t\tbranchTarget %d\n", stateptr->EXMEM.branchtarget);
		printf("\t\taluResult %d\n", stateptr->EXMEM.aluresult);
		printf("\t\treadReg %d\n", stateptr->EXMEM.readreg);
	printf("\tMEMWB:\n");
		printf("\t\tinstruction ");
		printInstruction(stateptr->MEMWB.instr);
		printf("\t\twriteData %d\n", stateptr->MEMWB.writedata);
	printf("\tWBEND:\n");
		printf("\t\tinstruction ");
		printInstruction(stateptr->WBEND.instr);
		printf("\t\twriteData %d\n", stateptr->WBEND.writedata);
}

void print_stats(statetype* state){
	printf("total of %d cycles executed\n", state->cycles);
	printf("total of %d instructions fetched\n", state->fetched);
	printf("total of %d instructions retired\n", state->retired);
	printf("total of %d branches executed\n", state->branches);
	printf("total of %d branch mispredictions\n", state->mispreds);
}

int regData(statetype* state,int reg){
	int destReg;
	int regAHold   = state->IDEX.readregA;
	int regBHold   = state->IDEX.readregB;
	int idex       = state->IDEX.instr;
        int exmem      = state->EXMEM.instr;
        int memwb      = state->MEMWB.instr;
        int wbend      = state->WBEND.instr;

	//WBEND
	if(opcode(wbend)==LW){
                destReg = field0(wbend);
                if(destReg==field0(idex)){
                        regAHold = state->WBEND.writedata;
                }
                if(destReg==field1(idex)){
                        regBHold = state->WBEND.writedata;
                }
        }
	if(opcode(wbend)==ADD||opcode(wbend)==NAND){
                destReg = field2(wbend);
                if(destReg==field0(idex)){
                        regAHold = state->WBEND.writedata;
                }
                if(destReg==field1(idex)){
                        regBHold = state->WBEND.writedata;
                }
        }

	//MEMWB
	if(opcode(memwb)==LW){
                destReg = field0(memwb);
                if(destReg==field0(idex)){
                        regAHold = state->MEMWB.writedata;
                }
                if(destReg==field1(idex)){
                        regBHold = state->MEMWB.writedata;
                }
        }
	if(opcode(memwb)==ADD||opcode(memwb)==NAND){
                destReg = field2(memwb);
                if(destReg==field0(idex)){
                        regAHold = state->MEMWB.writedata;
                }
                if(destReg==field1(idex)){
                        regBHold = state->MEMWB.writedata;
                }
        }

	//EXMEM
	if(opcode(exmem)==ADD||opcode(exmem)==NAND){
                destReg = field2(exmem);
                if(destReg==field0(idex)){
                        regAHold = state->EXMEM.aluresult;
                }
                if(destReg==field1(idex)){
                        regBHold = state->EXMEM.aluresult;
                }

        }

	if(reg==1){return regAHold;}
	if(reg==2){return regBHold;}
}
// Stall if pipeline needs to wait for instruction 
void stall(statetype* state,statetype* newstate,int regAHold,int regBHold){
	newstate->IFID.instr   = state->IFID.instr;
	newstate->IFID.pcplus1 = state->IFID.pcplus1;

	newstate->IDEX.instr    = state->IDEX.instr;
	newstate->IDEX.pcplus1  = state->IDEX.pcplus1;
	newstate->IDEX.readregA = regAHold;
	newstate->IDEX.readregB = regBHold;
	newstate->IDEX.offset   = state->IDEX.offset;

	newstate->EXMEM.instr = NOOPINSTRUCTION;
	newstate->EXMEM.branchtarget = 0;
	newstate->EXMEM.aluresult = 0;
	newstate->EXMEM.readreg = 0;

	//maintains position in instrmem
	newstate->pc = state->pc;

}

// Cleans pipeline of data
void flush(statetype* newstate){
	newstate->IFID.instr    = NOOPINSTRUCTION;
        newstate->IFID.pcplus1  = 0;

        newstate->IDEX.instr    = NOOPINSTRUCTION;
        newstate->IDEX.pcplus1  = 0;
        newstate->IDEX.readregA = 0;
        newstate->IDEX.readregB = 0;
        newstate->IDEX.offset   = 0;

	newstate->EXMEM.instr         = NOOPINSTRUCTION;
        newstate->EXMEM.branchtarget  = 0;
        newstate->EXMEM.aluresult     = 0;
        newstate->EXMEM.readreg       = 0;
}

// Pipeline Fetch
void instrFetch(statetype* state, statetype* newstate){
        newstate->IFID.instr = state->instrmem[state->pc];
	newstate->pc = state->pc+1;
        newstate->IFID.pcplus1 = state->pc+1;
        newstate->fetched = state->fetched+1;
}

// Pipeline Decode
void instrDecode(statetype* state, statetype* newstate){
	newstate->IDEX.instr    = state->IFID.instr;
        newstate->IDEX.pcplus1  = state->IFID.pcplus1;
       	newstate->IDEX.readregA = state->reg[field0(state->IFID.instr)];
       	newstate->IDEX.readregB = state->reg[field1(state->IFID.instr)];
	newstate->IDEX.offset   = signExtend(field2(state->IFID.instr));
}

// Pipeline execute stage
int exec(statetype* state, statetype* newstate){
	newstate->EXMEM.instr = state->IDEX.instr;
	newstate->EXMEM.branchtarget = state->IDEX.pcplus1 + state->IDEX.offset;

	int op = opcode(state->IDEX.instr);
	int regAHold = state->IDEX.readregA;
	int regBHold = state->IDEX.readregB;
	int offsetHold = state->IDEX.offset;
	int retired = state->retired;

	int idex  = state->IDEX.instr;
	int exmem = state->EXMEM.instr;
	int memwb = state->MEMWB.instr;
	int wbend = state->WBEND.instr;

	regAHold = regData(state,1);
	regBHold = regData(state,2);

	//STALL CHECK
	if(opcode(exmem)==LW){
	int lwReg = field0(exmem);
        	if(lwReg==field0(idex) || lwReg==field1(idex)){
                	stall(state,newstate,regAHold,regBHold);
			newstate->fetched = state->fetched;
			retired = state->retired-1;
			regAHold = 0;
			regBHold = 0;
			offsetHold = 0;
			newstate->EXMEM.branchtarget = 0;
        	}
	}

	newstate->EXMEM.readreg      = regAHold;

        if(op==ADD){
                //aluresult = rA+rB
		newstate->EXMEM.aluresult = regAHold + regBHold;
	}else if(op==NAND){
                //aluresult = ~(rA&rB)
		newstate->EXMEM.aluresult = ~(regAHold & regBHold);
	}else if(op==LW){
                //aluresult = rB+offset
		newstate->EXMEM.aluresult = regBHold + offsetHold;
	}else if(op==SW){
		//aluresult = rB+offset
		newstate->EXMEM.aluresult = regBHold + offsetHold;
	}else if(op==BEQ){
		//aluresult = rA-rB
		newstate->EXMEM.aluresult = regAHold - regBHold;

	}else if(op==JALR){

	}else if(op==NOOP){
		newstate->EXMEM.aluresult = 0;
		newstate->EXMEM.branchtarget = state->IDEX.pcplus1 + offsetHold;
		newstate->EXMEM.readreg = regAHold;
	}
	return retired;
}

// Pipeline Memory 
void memory(statetype* state, statetype* newstate,int retired){
	int op = opcode(state->EXMEM.instr);

	newstate->MEMWB.instr = state->EXMEM.instr;
	newstate->retired = retired+1;
	newstate->MEMWB.writedata = 0;

	if(op==ADD || op==NAND){
		//writedata = aluresult = rA+rb OR ~(rA&rB)
		newstate->MEMWB.writedata = state->EXMEM.aluresult;
	}else if(op==LW){
		//writedata = datamem[aluresult] = datamem[rB+offset]
		newstate->MEMWB.writedata = state->datamem[state->EXMEM.aluresult];
	}else if(op==SW){
		//writedata = readreg = content of rA
		newstate->datamem[state->EXMEM.aluresult] = state->EXMEM.readreg;
		newstate->MEMWB.writedata = 0;
	}else if(op==BEQ){
		newstate->branches = state->branches+1;

		if(opcode(state->IDEX.instr)==HALT){
			newstate->fetched = state->fetched-1;
		}

		//if(branch is taken)
		if(state->EXMEM.aluresult==0){
			//writedata = aluresult = rB+offset
			newstate->MEMWB.writedata = 0;

			//branch to target
			newstate->pc = state->EXMEM.branchtarget;

			//increment mispred since branch is taken
			newstate->mispreds = state->mispreds+1;

			if(opcode(state->IDEX.instr)==HALT){
                        	newstate->fetched = state->fetched-1;
                	}else if(opcode(state->IFID.instr)==HALT){
				newstate->fetched = state->fetched;
			}

			//flush pipeline
			flush(newstate);

			//adjust retired instructions to account for branch taken
			newstate->retired = state->retired-2;
		}
	}else if(op==NOOP){
		newstate->MEMWB.writedata=0;
	}
}

// Pipeline Writeback
void writeBack(statetype* state, statetype* newstate){
	int op = opcode(state->MEMWB.instr);
	newstate->WBEND.instr = state->MEMWB.instr;
	newstate->WBEND.writedata = state->MEMWB.writedata;

	if(op==ADD || op==NAND){
		//reg[destReg==field2/offset] = writedata = rA+rB OR ~(rA&rB)
		newstate->reg[field2(state->MEMWB.instr)] = state->MEMWB.writedata;
	}else if(op==LW){
		//reg[destReg==field0] = writedata = datamem[rB+offset]
		newstate->reg[field0(state->MEMWB.instr)] = state->MEMWB.writedata;
	}
}

// Initalizes values for pipeline and runs each stage
void run(statetype* state, statetype* newstate){

	state->pc = 0;

	for(int i=0;i<NUMREGS; i++){
		state->reg[i] = 0;
	}

	state->IFID.instr    = NOOPINSTRUCTION;
	state->IFID.pcplus1  = 0;

	state->IDEX.instr    = NOOPINSTRUCTION;
	state->IDEX.pcplus1  = 0;
	state->IDEX.readregA = 0;
	state->IDEX.readregB = 0;
	state->IDEX.offset   = 0;

	state->EXMEM.instr        = NOOPINSTRUCTION;
	state->EXMEM.branchtarget = 0;
	state->EXMEM.aluresult    = 0;
	state->EXMEM.readreg      = 0;

	state->MEMWB.instr     = NOOPINSTRUCTION;
	state->MEMWB.writedata = 0;

	state->WBEND.instr     = NOOPINSTRUCTION;
	state->WBEND.writedata = 0;

	state->cycles   = 0;
	state->fetched  = -3;
	state->retired  = -3;
	state->branches = 0;
	state->mispreds = 0;



	// Primary loop
	while(1){
		printstate(state);

		/* check for halt */
		if(HALT == opcode(state->MEMWB.instr)) {
			printf("machine halted\n");
			print_stats(state);
			break;
		}

		*newstate = *state;
		newstate->cycles++;

		/*------------------ IF stage ----------------- */
		instrFetch(state,newstate);
		/*------------------ ID stage ----------------- */
		instrDecode(state,newstate);
		/*------------------ EX stage ----------------- */
		int retired = exec(state,newstate);
		/*------------------ MEM stage ----------------- */
		memory(state,newstate,retired);
		/*------------------ WB stage ----------------- */
		writeBack(state,newstate);

		*state = *newstate; 	/* this is the last statement before the end of the loop.  
					It marks the end of the cycle and updates the current
					state with the values calculated in this cycle
					– AKA “Clock Tick”. */
	}
}

int main(int argc, char** argv){

	/** Get command line arguments **/
	char* fname;

	opterr = 0;

	int cin = 0;

	while((cin = getopt(argc, argv, "i:")) != -1){
		switch(cin)
		{
			case 'i':
				fname=(char*)malloc(strlen(optarg));
				fname[0] = '\0';

				strncpy(fname, optarg, strlen(optarg)+1);
				break;
			case '?':
				if(optopt == 'i'){
					printf("Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt)){
					printf("Unknown option `-%c'.\n", optopt);
				}
				else{
					printf("Unknown option character `\\x%x'.\n", optopt);
					return 1;
				}
				break;
			default:
				abort();
		}
	}

	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fname, strerror(errno));
		return -1;
	}

	/* count the number of lines by counting newline characters */
	int line_count = 0;
	int c;
	while (EOF != (c=getc(fp))) {
		if ( c == '\n' ){
			line_count++;
		}
	}
	// reset fp to the beginning of the file
	rewind(fp);

	statetype* state = (statetype*)malloc(sizeof(statetype));
	statetype* newstate = (statetype*)malloc(sizeof(statetype));

	state->numMemory = line_count;

	char line[256];

	int i = 0;
	while (fgets(line, sizeof(line), fp)) {
		/* note that fgets doesn't strip the terminating \n, checking its
		   presence would allow to handle lines longer that sizeof(line) */
		state->instrmem[i] = atoi(line);
		state->datamem[i]  = atoi(line);
		i++;
	}
	fclose(fp);

	/** Run the simulation **/
	run(state,newstate);

	free(state);
	free(newstate);
	free(fname);

}
