/******************************************************************************************
Project: UCSD CSE291C Course Project: Web Server for TritonHTTP

Author:
1. Hou Wang

handTCPClient:
This module handles incoming connection, and message parsing and framing.
Handle error in data format level.

Data in valid format will be passed to responser for content verification and response
*******************************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include "httpd.h"
#include "handleTCPClient.hpp"
#include "framer.hpp"
#include "parser.hpp"
#include "responder.hpp"
#include "diewithmessage.hpp"

using namespace std;

void *HandleTCPClient(void *args){
      ThreadArgs *arg = static_cast<ThreadArgs *>(args);
      int servSock = arg->servSocket;
      string doc_root = arg->doc_root;
      unsigned int clntLen; /*Length of client address*/
      int clntSock; /*set client socket fd*/
      sockaddr_in echoClntAddr; /*Client IP address*/

      while(true){
  		/* Set the size of the in-out parameter */
  		clntLen = sizeof(echoClntAddr);

      /* clntSock is connected to a client! */
  		if ((clntSock = accept(servSock, (sockaddr *) &echoClntAddr, &clntLen)) < 0){
  			DiewithMessage("accept() failed");
      }

      /* set socket receive timeout */
      timeval *timeOutVal = new timeval;
      timeOutVal->tv_sec = 5;
      timeOutVal->tv_usec = 0;
      if(setsockopt(clntSock, SOL_SOCKET, SO_RCVTIMEO, static_cast<void *>(timeOutVal), sizeof(timeOutVal)) < 0){
        DiewithMessage("Called setsockopt(): socket option set failed"); /*socket creation failed*/
      }

      string addr(inet_ntoa(echoClntAddr.sin_addr));
  		cerr << "Handling client " + addr << '\n';

      /* Start Request Handling Process */
  		HandleReq(clntSock, doc_root);
    }
}

void HandleReq(int clntSock, string doc_root){
  /* P1: Framing and Parsing incoming bytes, parse into a request data structure */
  /* Handle timeout error */
  char buffer[BUFSIZE];
  memset(buffer, 0, sizeof(buffer));
  Framer framer;
  Parser parser;
  Responder responder(clntSock, doc_root);
  ssize_t numBytesRcvd;

  if((numBytesRcvd = recv(clntSock, buffer, BUFSIZE, 0)) < 0){
    /* Insert handle timeout error response */
    responder.sendResponse(CLIENT_ERROR);
    close(clntSock);
    return;
  }

  while(!parser.isTerminated()){
    /* if connection:closed not detect */
    while(numBytesRcvd > 0){
      string input(buffer, numBytesRcvd);
      framer.append(input);

      while(framer.hasMessage()){
          string msg;
          msg = framer.topMessage();
          framer.popMessage();
          if(!parser.parse(msg)){
            /* Insert handle invalid header error response */
            responder.sendResponse(CLIENT_ERROR);
            close(clntSock);
            return;
          }
      }

      memset(buffer, 0, sizeof(buffer));
      if((numBytesRcvd = recv(clntSock, buffer, BUFSIZE, 0)) < 0){
        /* Insert Handle timeout error response*/
        responder.sendResponse(CLIENT_ERROR);
        close(clntSock);
        return;
      }
    }

    /* Produce response based on header provided*/
    int status = responder.verifyandAppendReq(parser.getReqHeader());
    responder.sendResponse(status);
  }

  /* Connection: close detected, close socket and return */
  close(clntSock);
  return;
}
