/*
 *  OscManager.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 10/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "OscListener.h"
#include "OscSender.h"
#include <string>
#include <cstdarg>

using namespace std;
using namespace ci;

class OscManager {
	
public:
	
	osc::Listener listener;
	osc::Sender sender;
	string m_send_host;
	int m_send_port, m_receive_port;
	
	OscManager(string send_host, int send_port, int receive_port)
	{
		m_send_host = send_host;
		m_send_port = send_port;
		m_receive_port = receive_port;
		sender.setup(send_host, send_port);
		listener.setup(receive_port);
	}
	
	void send(string address, float value)
	{
		osc::Message message;
		message.addFloatArg(value);
		message.setAddress(address);
		message.setRemoteEndpoint(m_send_host, m_send_port);
		sender.sendMessage(message);
	}
    
    void send(string address, int a, int b)
	{
		osc::Message message;

        message.addIntArg(a);
        message.addIntArg(b);
        
		message.setAddress(address);
		message.setRemoteEndpoint(m_send_host, m_send_port);
		sender.sendMessage(message);
	}
	
	void send(string address, string value)
	{
		osc::Message message;
		message.addStringArg(value);
		message.setAddress(address);
		message.setRemoteEndpoint(m_send_host, m_send_port);
		sender.sendMessage(message);
	}
	
};