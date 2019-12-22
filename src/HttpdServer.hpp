#ifndef HTTPDSERVER_HPP
#define HTTPDSERVER_HPP

#include <iostream>
#include <sysexits.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <netdb.h>
#include <netinet/in.h>
#include <csignal>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include <string>
#include <ctime>
#include <unistd.h>/* gethostname */

#include "inih/INIReader.h"
#include "logger.hpp"

#define LOGQUEUE_LEN 10

using namespace std;

class HttpdServer {
public:
	HttpdServer(INIReader& t_config);
	void launch();

protected:
	INIReader& config;
	string port;
	string doc_root;
    string mime_dir;

	int get_file_size(const char* filepath);
	int badreq_check(char* req_msg);
	int validate_file(string abs_path);
	int connectClose_check(char* key_vals);
	string readfrommime(string value);
	string build_400_badreq_headers();
	string build_404_notfound_headers();
	string build_200_ok_headers(string filepath);
	int handle_request(const char* req_msg, int client_sock);
	void responseThread(int new_sockfd);
};

#endif // HTTPDSERVER_HPP
