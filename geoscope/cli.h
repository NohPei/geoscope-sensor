#ifndef _GEOSCOPE_CLI_H
#define _GEOSCOPE_CLI_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "main.h"
#include <Commander.h>

bool cliInit(Stream &iostream = Serial);

extern Commander cli;


#endif
