#ifndef _GEOSCOPE_CLI_H
#define _GEOSCOPE_CLI_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <Commander.h>
#include <TelnetStream2.h>

bool cliInit(Stream &iostream = Serial, Stream &altstream = TelnetStream2);

extern Commander cli;

bool cli_swap(Commander &cmd = cli);

#endif
