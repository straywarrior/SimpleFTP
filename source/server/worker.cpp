/*
 * worker.cpp
 * @Author:     StrayWarrior
 * @Date:       2015-12-02
 * @Email:      i@straywarrior.com
 * Copyright (C) 2015 StrayWarrior
 *
 */

#include "worker.h"
#include "connection.h"
#include "fileoperation.h"

static int get_data_conn_parm(char * arg_buf, unsigned int * data_v4addr, unsigned int * data_port){
    char addr_buf[16];
    addr_buf[0] = '\0';
    int sec_count = 0;
    char * p_first = arg_buf;
    char * p_second = arg_buf;
    char sec[4];
    while (sec_count < 4 && p_second && *p_second != '\0'){
        if (*p_second == ','){
            strncpy(sec, p_first, p_second - p_first);
            sec[p_second - p_first] = '\0';
            strcat(addr_buf, sec);
            sec_count++;
            server_log(SERVER_LOG_DEBUG, "IP Section: %s ", sec);
            if (sec_count != 4){
                strcat(addr_buf, ".");
            }
            p_first = ++p_second;
        }else{
            ++p_second;
        }
    }
    if (sec_count != 4){
        return -1;
    }
    server_log(SERVER_LOG_DEBUG, "\nData Connection IP parsed: %s \n", addr_buf);
    
    int port = 0;
    // The High-8bit
    p_second = strstr(p_first, ",");
    strncpy(sec, p_first, p_second - p_first);
    sec[p_second - p_first] = '\0';
    server_log(SERVER_LOG_DEBUG, "Port Section: %s ", sec);
    port = atoi(sec) * 256;
    // The Low-8bit
    p_first = ++p_second;
    strcpy(sec, p_first);
    server_log(SERVER_LOG_DEBUG, "Port Section: %s ", sec);
    port += atoi(sec);
    
    *data_v4addr = (unsigned int)inet_addr(addr_buf);
    *data_port = port;
    return 0;
}

static void get_system_str(char * buf){
#if defined( __APPLE__)
    strcpy(buf, "OSX");
#elif defined(__linux__)
    strcpy(buf, "LINUX");
#endif
}

static int set_mode(myftpserver_worker_t * worker_t, char * arg_buf){
    if (strlen(arg_buf) != 1){
        return -1;
    }
    char mode = arg_buf[0];
    switch(mode){
        case 'S':
            worker_t->mode = TRANSMODE_S;
            break;
        case 'B':
            return -2;
        case 'C':
            return -2;
        default:
            return -1;
    }
    return 0;
}

myftpserver_worker_t::~myftpserver_worker_t(){
    this->server = nullptr;    
}

int worker_run(myftpserver_worker_t * worker_t) {
    int ctl_conn = worker_t->connection;
    send_msg(ctl_conn, REPCODE_220, strlen(REPCODE_220));

    // TODO: Read username and user-dir from config dir
    
    strcpy(worker_t->rootdir, worker_t->server->default_dir);
    strcpy(worker_t->reladir, "/");
    
    bool conn_close = false;
    bool user_login = false;
    while(!conn_close){
        char arg_buf[MAX_READ_BUF + 1] = {0};
        FTPCMD cur_cmd = read_command(ctl_conn, arg_buf);
        server_log(SERVER_LOG_DEBUG, "Command %d from connection %d.\n", static_cast<int>(cur_cmd), ctl_conn);;

        if (cur_cmd == FTPCMD::ERROR || cur_cmd == FTPCMD::QUIT){
            send_msg(ctl_conn, REPCODE_221, strlen(REPCODE_221));
            conn_close = true;
            continue;
        }
        if (cur_cmd == FTPCMD::UNIMPL){
            send_msg(ctl_conn, REPCODE_502, strlen(REPCODE_502));
            continue;
        }
        if (cur_cmd == FTPCMD::UNKNOWN){
            send_msg(ctl_conn, REPCODE_503, strlen(REPCODE_503));
            continue;
        }
        if (!user_login){
            switch (cur_cmd){
               case FTPCMD::USER:
                    if (strlen(arg_buf) == 0){
                        send_msg(ctl_conn, REPCODE_501, strlen(REPCODE_501));
                        break;
                    }
                    if (strcmp(arg_buf, "anonymous") == 0){
                        // Not in RFC 959. Suggested by RFC 1635
                    }
                    strcpy(worker_t->username, arg_buf);
                    send_msg(ctl_conn, REPCODE_331, strlen(REPCODE_331));
                    break;
                case FTPCMD::PASS:
                    // TODO: Add Password-check after adding config-read
                    if (strlen(worker_t->username) > 0){
                        user_login = true;
                        send_msg(ctl_conn, REPCODE_230, strlen(REPCODE_230));
                    }
                    break;
                default:
                    send_msg(ctl_conn, REPCODE_530, strlen(REPCODE_530));
                    break;
            }
        }else{
            int data_conn;
            switch (cur_cmd){
                case FTPCMD::CWD:
                    // TODO: Add change working dir command.
                    if (change_dir(worker_t, arg_buf) < 0){
                        send_msg(ctl_conn, REPCODE_550, strlen(REPCODE_550));
                    };
                    server_log(SERVER_LOG_DEBUG, "Dir changed to %s for connection %d.\n", worker_t->reladir, ctl_conn);
                    send_msg(ctl_conn, REPCODE_250, strlen(REPCODE_250));
                    break;
                case FTPCMD::CDUP:
                    if (change_dir(worker_t, "..") < 0){
                        send_msg(ctl_conn, REPCODE_550, strlen(REPCODE_550));
                    };
                    server_log(SERVER_LOG_DEBUG, "Dir changed to %s for connection %d.\n", worker_t->reladir, ctl_conn);
                    send_msg(ctl_conn, REPCODE_250, strlen(REPCODE_250));
                    break;
                case FTPCMD::PORT:
                    // TODO: RFC 959 minimum
                    if (get_data_conn_parm(arg_buf, &(worker_t->data_v4addr), &(worker_t->data_port)) < 0){
                        send_msg(ctl_conn, REPCODE_501, strlen(REPCODE_501));
                    }else{
                        send_msg(ctl_conn, REPCODE_200, strlen(REPCODE_200));
                    }
                    break;
                case FTPCMD::TYPE:
                    // TODO: RFC 959 minimum
                    send_msg(ctl_conn, REPCODE_200, strlen(REPCODE_200));
                    break;
                case FTPCMD::STRU:
                    // TODO: RFC 959 minimum
                    break;
                case FTPCMD::MODE:
                    // TODO: RFC 959 minimum
                    {
                        int set_mode_status = set_mode(worker_t, arg_buf);
                        switch (set_mode_status){
                            case 0:
                                send_msg(ctl_conn, REPCODE_200);
                                break;
                            case -1:
                                send_msg(ctl_conn, REPCODE_501);
                                break;
                            case -2:
                            default:
                                send_msg(ctl_conn, REPCODE_504);
                                break;
                        }
                    }
                    break;
                case FTPCMD::RETR:
                    data_conn = open_data_connection(ctl_conn, worker_t->data_v4addr, worker_t->data_port);
                    worker_t->data_conn = data_conn;
                    retrieve_file(worker_t, arg_buf);
                    close_data_connection(ctl_conn, data_conn);
                    break;
                case FTPCMD::STOR:
                    data_conn = open_data_connection(ctl_conn, worker_t->data_v4addr, worker_t->data_port);
                    worker_t->data_conn = data_conn;
                    store_file(worker_t, arg_buf);
                    close_data_connection(ctl_conn, data_conn);
                    break;
                case FTPCMD::PWD:
                    {
                        char cur_dir[MAX_PATH_LEN];
                        get_cur_path(worker_t, cur_dir);
                        //strcat(cur_dir, EOL);
                        char send_buf[MAX_PATH_LEN];
                        prepare_msg(send_buf, REPCODE_257, cur_dir);
                        server_log(SERVER_LOG_DEBUG, "PWD reply: %s length: %zu \n", send_buf, strlen(send_buf));
                        send_msg(ctl_conn, send_buf);
                        break;
                    }
                case FTPCMD::LIST:
                    data_conn = open_data_connection(ctl_conn, worker_t->data_v4addr, worker_t->data_port);
                    worker_t->data_conn = data_conn;
                    list_dir(worker_t);
                    close_data_connection(ctl_conn, data_conn);
                    break;
                case FTPCMD::SIZE:
                    // TODO: Needed by Chrome ftp. What the hell... RFC 3659
                    {
                        char send_buf[32];
                        prepare_msg(send_buf, REPCODE_213_SIZE, (long long)250);
                        send_msg(ctl_conn, send_buf);
                        break;
                    }
                case FTPCMD::HELP:
                    send_help(ctl_conn);
                    break;
                case FTPCMD::NOOP:
                    send_msg(ctl_conn, REPCODE_200);
                    break;
                case FTPCMD::SYST:
                    // Sent by FTP built-in Linux
                    {
                        char buf[10];
                        get_system_str(buf);
                        char send_buf[100];
                        prepare_msg(send_buf, REPCODE_215, buf);
                        send_msg(ctl_conn, send_buf);
                        break;
                    }
                default:
                    send_msg(ctl_conn, REPCODE_502, strlen(REPCODE_502));
                    break;
            }
        }
    }
    delete worker_t;
    return 0;
}

