/*
 * Compile the loader with:
 *   target_link_options(<loader name> PRIVATE -Ttext=0x80068000)
 *
 * All Crash games' executables are about 0x51000 bytes long and loaded at
 * 0x80010000, so relocating the loader to 0x80068000 should work.
 *
 * The path passed to launchExecutable() should have no "cdrom:" prefix (i.e.
 * "\FILENAME.EXE;1"). ResetGraph() and CdInit() must be called beforehand.
 */
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxcd.h>
#include <psxpad.h>

struct ExeHeader {
	uint8_t magic[16];
	struct EXEC param;
	char _padding[1972];
};

extern const uint8_t __text_start[], _end[];

bool launchExecutable(const char *path, int maxAttempts) {
	CdlFILE file;
	ExeHeader header;

	if (!CdSearchFile(&file, path)) {
		printf("Failed to open %s (error %d)\n", path, CdIsoError());
		return false;
	}

	// Read the header (first sector) to determine where the executable should
	// be loaded in RAM.
	CdControl(CdlSetloc, &(file.pos), nullptr);
	CdReadRetry(
		sizeof(header) / 2048,
		reinterpret_cast<uint32_t*>(&header),
		CdlModeSpeed, maxAttempts
	);
	if (CdReadSync(false, nullptr) < 0)
		return false;
	
	// Validate the header by checking for the magic string and making sure the
	// body is not overlapping the loader.
	if (memcmp(header.magic, "PS-X EXE", 8))
		return false;

	if (!(
		(header.param.t_addr >= reinterpret_cast<uint32_t>(_end)) ||
		((header.param.t_addr + header.param.t_size) <= reinterpret_cast<uint32_t>(__text_start))
	)) {
		printf(
			"Memory areas overlap: %08x-%08x (exe), %08x-%08x (loader)\n",
			header.param.t_addr, header.param.t_addr + header.param.t_size,
			__text_start, _end
		);
		return false;
	}

	// Read the body of the executable directly into the target address.
	CdIntToPos(CdPosToInt(&(file.pos)) + sizeof(header) / 2048, &(file.pos));
	CdControl(CdlSetloc, &(file.pos), nullptr);
	CdReadRetry(
		header.param.t_size / 2048,
		reinterpret_cast<uint32_t *>(header.param.t_addr),
		CdlModeSpeed, maxAttempts
	);
	if (CdReadSync(false, nullptr) < 0)
		return false;

	// Blank the screen and uninitialize everything to make sure the executable
	// can do its own initialization.
	CdSync(false, nullptr);
	DrawSync(false);
	SetDispMask(false);
	StopPAD();
	ChangeClearPAD(1);
	ChangeClearRCnt(3, 1);
	StopCallback();
	FlushCache();

	printf("Executing...\n");
	Exec(&header.param, 0, 0);

	RestartCallback();
	return true;
}

// Define display/draw environments for double buffering
DISPENV disp[2];
DRAWENV draw[2];
int db;

#define OTLEN 8         // Ordering table length (recommended to set as a define
// so it can be changed easily)

// Define variables for controllers
u_long ot[2][OTLEN];    // Ordering table length
char pribuff[2][32768]; // Primitive buffer
char* nextpri;          // Next primitive pointer

// Pad stuff (omit when using PSn00bSDK)
#define PAD_SELECT      1
#define PAD_L3          2
#define PAD_R3          4
#define PAD_START       8
#define PAD_UP          16
#define PAD_RIGHT       32
#define PAD_DOWN        64
#define PAD_LEFT        128
#define PAD_L2          256
#define PAD_R2          512
#define PAD_L1          1024
#define PAD_R1          2048
#define PAD_TRIANGLE    4096
#define PAD_CIRCLE      8192
#define PAD_CROSS       16384
#define PAD_SQUARE      32768

// pad buffer arrays
u_char padbuff[2][34];

// Init function
void init(void)
{
	// This not only resets the GPU but it also installs the library's
	// ISR subsystem to the kernel
	ResetGraph(0);
	CdInit();

	// Define display environments, first on top and second on bottom
	SetDefDispEnv(&disp[0], 0, 0, 320, 240);
	SetDefDispEnv(&disp[1], 0, 240, 320, 240);

	// Define drawing environments, first on bottom and second on top
	SetDefDrawEnv(&draw[0], 0, 240, 320, 240);
	SetDefDrawEnv(&draw[1], 0, 0, 320, 240);

	// Set and enable clear color
	setRGB0(&draw[0], 0, 0, 0);
	setRGB0(&draw[1], 0, 0, 0);
	draw[0].isbg = 1;
	draw[1].isbg = 1;

	// Clear double buffer counter
	db = 0;
	nextpri = pribuff[0];           // Set initial primitive pointer address

	// Apply the GPU environments
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);

	// Load test font
	FntLoad(960, 0);

	// Open up a test font text stream of 100 characters
	FntOpen(0, 8, 320, 224, 0, 200);

	// Initialize the pads
	InitPAD(padbuff[0], 34, padbuff[1], 34);

	// Begin polling
	StartPAD();

	// To avoid VSync Timeout error, may not be defined in PsyQ
	ChangeClearPAD(1);
}

// Display function
void display(void)
{
	// Flip buffer index
	db = !db;
	nextpri = pribuff[db];      // Reset next primitive pointer

	// Wait for all drawing to complete
	DrawSync(0);

	// Wait for vertical sync to cap the logic to 60fps (or 50 in PAL mode)
	// and prevent screen tearing
	VSync(0);

	// Switch pages	
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);

	// Enable display output, ResetGraph() disables it by default
	SetDispMask(1);

}

int main( int argc, const char *argv[] )
{
	const char* filename = "none";
	int executing = 0;

	init();
	
	while( 1 )
	{
		if (executing == 1) {
			executing = 2;
			if (launchExecutable(filename, 5) == true)
				executing = 3; // executed
			else
				executing = 4; // failed
		}

		FntPrint(-1, "Welcome to Sestain's bootleg loader\n\n");

		FntPrint(-1, "Selected: %s\n", filename);

		FntPrint(-1, "Press start to boot game\n");

		PADTYPE* pad = (PADTYPE*)padbuff[0];

		// Only parse inputs when a controller is connected
		if (pad->stat == 0)
		{
			// Only parse when a digital pad, 
			// dual-analog and dual-shock is connected
			if ((pad->type == 0x4) ||
				(pad->type == 0x5) ||
				(pad->type == 0x7))
			{
				if (!(pad->btn & PAD_CROSS))
				{
					filename = "\\LOADS\\CRASH1.EXE;1";
				}
				if (!(pad->btn & PAD_CIRCLE))
				{
					filename = "\\LOADS\\CRASH2.EXE;1";
				}
				if (!(pad->btn & PAD_TRIANGLE))
				{
					filename = "\\LOADS\\CRASH3.EXE;1";
				}
				if (!(pad->btn & PAD_SQUARE))
				{
					filename = "\\LOADS\\BASH.EXE;1";
				}
				//if (!(pad->btn & PAD_SELECT))
				//{
				//	filename = "\\LOADS\\CTR.EXE;1";
				//}

				if (!(pad->btn & PAD_START)) {
					if (!(filename == "none") && !(executing == 2)) {
						executing = 1;
					}
				}
			}
		}
		
		if (executing > 0)
			FntPrint(-1, "Executing: %s\n", filename);

		// Draw the last created text stream
		FntFlush(-1);

		// Update display
		display();
	}
	
	return( 0 );
	
} /* main */