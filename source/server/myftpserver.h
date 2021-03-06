/*
 * myftpserver.h
 * @Author:     StrayWarrior
 * @Date:       2015-12-02
 * @Email:      i@straywarrior.com
 * Copyright (C) 2015 StrayWarrior
 *
 */

#ifndef _MYFTP_SERVER_H_
#define _MYFTP_SERVER_H_ value

/*
 * My header
 */
#include "../common/common.h"

/*
 * Server Constants Definition
 */
#define DEFAULT_IPV4_ADDR   INADDR_ANY    // Selected IP by System
#define DEFAULT_PORT        21
#define DEFAULT_DATA_PORT   20
#define RETRY_TIME          1             // Retry after 1 minutes.
#define EOL                 "\r\n"

#define TRANSMODE_S         0
#define TRANSMODE_B         1
#define TRANSCODE_C         2

/*
 * The config structure
 */
typedef struct myftpserver_t{
    unsigned int port;      // In host sequence
    unsigned int ipv4addr;  // In network sequence
    unsigned int max_conns;
    //TODO: add ipv6 support?
    bool allow_anonymous;
    char default_dir[MAX_PATH_LEN];
}myftpserver_t;

/*
 * Define reply codes
 */
#define REPCODE_110 "110 Restart marker reply.\r\n"
#define REPCODE_120 "120 Try again in " ## RETRY_TIME ##" minutes.\r\n"
#define REPCODE_125 "125 Data connection already open; transfer starting.\r\n"
#define REPCODE_150 "150 File status okay; about to open data connection.\r\n"

#define REPCODE_200 "200 Command okay.\r\n"
#define REPCODE_202 "202 Command not implemented, superfluous at this site.\r\n"
#define REPCODE_211 "211 System status, or system help reply.\r\n"
#define REPCODE_212 "212 Directory status.\r\n"
#define REPCODE_213 "213 File status.\r\n"
#define REPCODE_213_SIZE "213 %lld\r\n"
#define REPCODE_214 "214 Help message.\r\n"
#define REPCODE_215 "215 %s system type.\r\n"
#define REPCODE_220 "220 Service ready for new user.\r\n"
#define REPCODE_221 "221 Service closing control connection.\r\n"
#define REPCODE_225 "225 Data connection open; no transfer in progress.\r\n"
#define REPCODE_226 "226 Closing data connection.\r\n"
#define REPCODE_227 "227 Entering Passive Mode (%s,%s,%s,%s,%s,%s).\r\n"
#define REPCODE_230 "230 User logged in, proceed.\r\n"
#define REPCODE_250 "250 Requested file action okay, completed.\r\n"
#define REPCODE_257 "257 \"%s\" created.\r\n"

#define REPCODE_331 "331 User name okay, need password.\r\n"
#define REPCODE_332 "332 Need account for login.\r\n"
#define REPCODE_350 "350 Requested file action pending further information.\r\n"

#define REPCODE_421 "421 Service not available, closing control connection.\r\n"
#define REPCODE_425 "425 Can't open data connection.\r\n"
#define REPCODE_426 "426 Connection closed; transfer aborted.\r\n"
#define REPCODE_450 "450 Requested file action not taken.\r\n"
#define REPCODE_451 "451 Requested action aborted: local error in processing.\r\n"
#define REPCODE_452 "452 Requested action not taken.\r\n"

#define REPCODE_500 "500 Syntax error, command unrecognized.\r\n"
#define REPCODE_501 "501 Syntax error in parameters or arguments.\r\n"
#define REPCODE_502 "502 Command not implemented.\r\n"
#define REPCODE_503 "503 Bad sequence of commands.\r\n"
#define REPCODE_504 "504 Command not implemented for that parameter.\r\n"
#define REPCODE_530 "530 Not logged in.\r\n"
#define REPCODE_532 "532 Need account for storing files.\r\n"
#define REPCODE_550 "550 Requested action not taken.\r\n"
#define REPCODE_551 "551 Requested action aborted: page type unknown.\r\n"
#define REPCODE_552 "552 Requested file action aborted.\r\n"
#define REPCODE_553 "553 Requested action not taken.\r\n"

#define SERVER_LOG_FATAL    MYFTP_LOG_FATAL
#define SERVER_LOG_ERROR    MYFTP_LOG_ERROR
#define SERVER_LOG_WARNING  MYFTP_LOG_WARNING
#define SERVER_LOG_INFO     MYFTP_LOG_INFO
#define SERVER_LOG_DEBUG    MYFTP_LOG_DEBUG
#define server_log(level, ...) myftp_log(level, __VA_ARGS__)

/*
 * Main Functions Definition
 */
int start_server(myftpserver_t * server_t);

#endif
