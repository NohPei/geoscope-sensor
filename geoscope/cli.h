#ifndef _GEOSCOPE_CLI_H
#define _GEOSCOPE_CLI_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <Commander.h>
#include <TelnetStream.h>

bool cliInit(Stream &iostream = Serial, Stream &altstream = TelnetStream);

extern Commander cli;

bool cli_swap(Commander &cmd = cli);
bool backup(Commander &cmd = cli);
bool restore(Commander &cmd = cli);


#endif
