#include <dos.h> 

#define CONSOLE_WIDTH 80		//width of console window
#define VERTICAL_OFFSET 12	//vertical offset from top
#define HORIZONTAL_OFFSET 20		//horizontal offset from left
#define REG_TEXT_SPACE 9	//symbols per register



		//INFO:
		//var - 12. Starting vectors - B8h, 08h



struct VIDEO
{
	unsigned char symb;
	unsigned char attr;
};
//role of bits(7->0):
//blink; bgr red; bgr green; bgr blue; saturated(bold?); font red; font green; font blue
int attr = 0xD6;		//attribute for video mode

void print(int offset, int value);
void getRegisterValue();

void interrupt(*oldHandleB8) (...);
void interrupt(*oldHandleB9) (...);
void interrupt(*oldHandleBA) (...);
void interrupt(*oldHandleBB) (...);
void interrupt(*oldHandleBC) (...);
void interrupt(*oldHandleBD) (...);
void interrupt(*oldHandleBE) (...);
void interrupt(*oldHandleBF) (...);

void interrupt(*oldHandle08) (...);
void interrupt(*oldHandle09) (...);		//keyboard interrupt address
void interrupt(*oldHandle0A) (...);
void interrupt(*oldHandle0B) (...);
void interrupt(*oldHandle0C) (...);
void interrupt(*oldHandle0D) (...);
void interrupt(*oldHandle0E) (...);
void interrupt(*oldHandle0F) (...);

void interrupt newHandleB8(...) { getRegisterValue(); oldHandleB8(); }
void interrupt newHandleB9(...) { getRegisterValue(); oldHandleB9(); }
void interrupt newHandleBA(...) { getRegisterValue(); oldHandleBA(); }
void interrupt newHandleBB(...) { getRegisterValue(); oldHandleBB(); }
void interrupt newHandleBC(...) { getRegisterValue(); oldHandleBC(); }
void interrupt newHandleBD(...) { getRegisterValue(); oldHandleBD(); }
void interrupt newHandleBE(...) { getRegisterValue(); oldHandleBE(); }
void interrupt newHandleBF(...) { getRegisterValue(); oldHandleBF(); }

void interrupt newHandle08(...) { getRegisterValue(); oldHandle08(); }
void interrupt newHandle09(...) { attr++; getRegisterValue(); oldHandle09(); }	//change video mode
void interrupt newHandle0A(...) { getRegisterValue(); oldHandle0A(); }
void interrupt newHandle0B(...) { getRegisterValue(); oldHandle0B(); }
void interrupt newHandle0C(...) { getRegisterValue(); oldHandle0C(); }
void interrupt newHandle0D(...) { getRegisterValue(); oldHandle0D(); }
void interrupt newHandle0E(...) { getRegisterValue(); oldHandle0E(); }
void interrupt newHandle0F(...) { getRegisterValue(); oldHandle0F(); }

void print(int offset, int value)
{
	char temp;

	VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0);
	//moves cursor to position
	screen += VERTICAL_OFFSET * CONSOLE_WIDTH + offset;

	for (int i = 0; i < 8; i++)
	{
		temp = value % 2;
		value /= 2;
		screen->symb = temp + '0';
		screen->attr = attr;
		screen++;
	}
}

void getRegisterValue()
{
	//prints all 6 registers, master in 1st row, slave in 2nd

	print(0 + HORIZONTAL_OFFSET, inp(0x21));

	outp(0x20, 0x0B);
	print(REG_TEXT_SPACE + HORIZONTAL_OFFSET, inp(0x20));

	outp(0x20, 0x0A);
	print(REG_TEXT_SPACE * 2 + HORIZONTAL_OFFSET, inp(0x20));

	print(CONSOLE_WIDTH + HORIZONTAL_OFFSET, inp(0xA1));

	outp(0xA0, 0x0B);
	print(CONSOLE_WIDTH + REG_TEXT_SPACE + HORIZONTAL_OFFSET, inp(0xA0));

	outp(0xA0, 0x0A);
	print(CONSOLE_WIDTH + REG_TEXT_SPACE * 2 + HORIZONTAL_OFFSET, inp(0xA0));
}

void init()
{
	//get old interrupt vectors
	oldHandle08 = getvect(0xB8);
	oldHandle09 = getvect(0xB9);
	oldHandle0A = getvect(0xBA);
	oldHandle0B = getvect(0xBB);
	oldHandle0C = getvect(0xBC);
	oldHandle0D = getvect(0xBD);
	oldHandle0E = getvect(0xBE);
	oldHandle0F = getvect(0xBF);
	
	oldHandle08 = getvect(0x08);
	oldHandle09 = getvect(0x09);
	oldHandle0A = getvect(0x0A);
	oldHandle0B = getvect(0x0B);
	oldHandle0C = getvect(0x0C);
	oldHandle0D = getvect(0x0D);
	oldHandle0E = getvect(0x0E);
	oldHandle0F = getvect(0x0F);
	
	//set custom interrupt vectors
	setvect(0xB8, newHandleB8);
	setvect(0xB9, newHandleB9);
	setvect(0xBA, newHandleBA);
	setvect(0xBB, newHandleBB);
	setvect(0xBC, newHandleBC);
	setvect(0xBD, newHandleBD);
	setvect(0xBE, newHandleBE);
	setvect(0xBF, newHandleBF);
	
	setvect(0x08, newHandle08);
	setvect(0x09, newHandle09);
	setvect(0x0A, newHandle0A);
	setvect(0x0B, newHandle0B);
	setvect(0x0C, newHandle0C);
	setvect(0x0D, newHandle0D);
	setvect(0x0E, newHandle0E);
	setvect(0x0F, newHandle0F);

	// Disable interrupts handling (cli)
	_disable();

	// Master
	outp(0x20, 0x11); // ICW1
	outp(0x21, 0x08); // ICW2
	outp(0x21, 0x04); // ICW3
	outp(0x21, 0x01); // ICW4

	// Slave
	outp(0xA0, 0x11); // ICW1
	outp(0xA1, 0x70); // ICW2
	outp(0xA1, 0x02); // ICW3
	outp(0xA1, 0x01); // ICW4

	// Enable interrupt handling (sti)
	_enable();
}

int main()
{
	unsigned far* fp;

	init();

	FP_SEG(fp) = _psp;
	FP_OFF(fp) = 0x2c;
	_dos_freemem(*fp);

	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);
	return 0;
}