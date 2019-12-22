# Web Server using HTTP1.1 Protocol

## Intro
This is a multi-threading web server using HTTP 1.1 protocol, implementation is based on C++ socket API. It can response GET request from different clients concurrently and response 200, 400 or 404 accordingly. HTTP 1.1 has many new additional features as compared with HTTP 1.0, two of them are implemented here: supporting long time connection and pipelined requests. 

## Note
This is a course project of graduate netowrk systems, so if you are taking this course and doing this project, don't check and use codes in this repository.

## Usage
cd to ./src directory and execute 'make' in shell, which will generate an executable program 'httpd'. execute './httpd config.ini' in the shell then the server will start running. The port, file root directory of server and mime types can be specified in config.ini.

There is also a simple client script used to make some test, simply execute 'g++ client.cpp -o client' and './client [SITE DNS] [PORT]' in the shell will let the client send msg to the server.
