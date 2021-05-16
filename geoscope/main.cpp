// 
// 
// 

#include "main.h"

#include<time.h>

char timebuf[CHAR_BUF_SIZE];

char* timestamp() {
	time_t now = time(nullptr);
	strftime(timebuf, CHAR_BUF_SIZE, "%d%b%Y-%T ", localtime(&now));
	return timebuf;
}


