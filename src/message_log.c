 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <fcntl.h>      /* for open() */
#include "message_log.h"

/**
 * @brief local print buffer max size which print  use
 */
#define PRINT_STRING_MAX	(256)
/**
 * @brief        print mode 
 *			0:None
 *			1:Terminal output, to /dev/kmsg
 *			2:syslog, this is normal way
 *			3:printf, only use debug
 */
#define FORCE_PRINT_MODE	(3)	/* 1:Terminal 2:syslog 3:printf */



#if FORCE_PRINT_MODE==2
#define MSL_LOG_DISABLE	(MSL_LOG_INFO | MSL_LOG_DEBUG)
#elif FORCE_PRINT_MODE==3
#define MSL_LOG_DISABLE	(MSL_LOG_DEBUG)
#else
#define MSL_LOG_DISABLE	( MSL_LOG_DEBUG)
#endif


/**
 * @internal
 * @brief <print log.>
 * @param[in] ubBuffOut <log string.>
 * @param[in] loglevel <log level [fatal,error,warn,info,debug]>
 *
 */
static void lvprintlog( const char* ubBuffOut, int loglevel )
{

	#if FORCE_PRINT_MODE==2
	uint32_t	auwErrorType =  (MSL_LOG_FATAL==loglevel)?LOG_ALERT:\
							(MSL_LOG_ERROR==loglevel)?LOG_ERR:\
							(MSL_LOG_WARN ==loglevel)?LOG_WARNING:\
							(MSL_LOG_INFO ==loglevel)?LOG_INFO:\
							(MSL_LOG_DEBUG ==loglevel)?LOG_DEBUG:LOG_DEBUG;
	#endif

	#if FORCE_PRINT_MODE==1
	int aswFD = open( "/dev/kmsg", O_WRONLY|O_NOCTTY|O_CLOEXEC );
	if ( aswFD >= 0 )
	{
		write( aswFD, ubBuffOut, strlen(ubBuffOut) );
		close( aswFD );
	}
	#elif FORCE_PRINT_MODE==2

	syslog(auwErrorType, "%s",ubBuffOut);

	#elif FORCE_PRINT_MODE==3
	printf("%s\n",ubBuffOut);
	fflush(stdout);
	#endif

	return;
}

void msl_log(int prio, const char* format, ...)
{
	char ubBuff[PRINT_STRING_MAX];
	va_list ap;

	if( MSL_LOG_DISABLE & prio)
		return;

	va_start(ap, format);
	vsnprintf(ubBuff,PRINT_STRING_MAX, format, ap);
	va_end(ap);

	lvprintlog( ubBuff , MSL_LOG_FATAL ); 

	return;
}
