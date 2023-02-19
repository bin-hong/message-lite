/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * \file message-log.h
 */
#ifndef _MESSAGE_LOG_H_
#define _MESSAGE_LOG_H_

/**
 * @brief log filter
 */
#define MSL_LOG_FATAL		(1<<0)
#define MSL_LOG_ERROR		(1<<1)
#define MSL_LOG_WARN		(1<<2)
#define MSL_LOG_INFO		(1<<3)
#define MSL_LOG_DEBUG		(1<<4)
void msl_log(int prio, const char* format, ...);

#define debug_switch 
#ifdef debug_switch
    #define debug_printf(format,...)     printf("LINE: [%d]: "format"\n", __LINE__, ##__VA_ARGS__)
#else
    #define debug_printf(format,...)
#endif

#endif  //_MESSAGE_LOG_H_

