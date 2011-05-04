/*
 *  OscManager.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 10/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "constants.h"
#include "common.h"
#include "OscListener.h"
#include "OscSender.h"
#include <string>
#include <cstdarg>
#include <vector>
#include <iostream>

#define TYPE_SPECTRUM 0
#define TYPE_WAVEFORM 1

using namespace std;
using namespace ci;

class OscManager {
	
public:
	
	osc::Listener listener;
	osc::Sender sender;
	string m_send_host;
	int m_send_port, m_receive_port;
    float spectrum[SAMPLE_WINDOW_SIZE];
    float waveform[SAMPLE_WINDOW_SIZE];
    GameState *gs;
	
	OscManager(string send_host, int send_port, int receive_port, GameState *_gs)
	{
		m_send_host = send_host;
		m_send_port = send_port;
		m_receive_port = receive_port;
		sender.setup(send_host, send_port);
		listener.setup(receive_port);
        gs = _gs;
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
	
	void send(string address, int a, float b)
	{
		osc::Message message;
		
        message.addIntArg(a);
        message.addFloatArg(b);
        
		message.setAddress(address);
		message.setRemoteEndpoint(m_send_host, m_send_port);
		sender.sendMessage(message);
	}
	
	void send(string address, int a, float b, float c, float d)
	{
		osc::Message message;
		
        message.addIntArg(a);
        message.addFloatArg(b);
		message.addFloatArg(c);
		message.addFloatArg(d);
        
		message.setAddress(address);
		message.setRemoteEndpoint(m_send_host, m_send_port);
		sender.sendMessage(message);
	}
	
	void send(string address, int a, string b)
	{
		osc::Message message;
		
        message.addIntArg(a);
        message.addStringArg(b);
        
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
	
	void send(string address, vector<int> &iv)
	{
		osc::Bundle bundle;
		

		
		
		string s = "";
		
		vector<int>::iterator it;
		for(it = iv.begin(); it != iv.end(); it++)
		{
			s += (char) *it;
			if(strlen(s.c_str()) == 1920)
			{
				osc::Message message;
				message.setAddress(address);
				message.setRemoteEndpoint(m_send_host, m_send_port);
				message.addStringArg(s);
				bundle.addMessage(message);
				s = "";
			}
		}
		
		sender.sendBundle(bundle);
		
	}
    
    void receive()
    {
        while (listener.hasWaitingMessages()) {
            osc::Message message;
            listener.getNextMessage(&message);

            
            string addr = message.getAddress();
			
			
			if(addr.find("event") != string::npos)
			{
				cout << "OSC: " << addr << endl;
				if(addr.find("match") != string::npos)
				{
					int idMatched = message.getArgAsInt32(0);
					cout << "match registered: " << idMatched << endl;
					gs->matchRegistered = idMatched;
					gs->lastMatchActive = SK_MATCH_ACTIVE_TIME;
				}
				else if(addr.find("intro_done") != string::npos)
				{
					
					*(gs->intro_playing) = false;
					cout << "intro done" << endl;
				}
			}
			else
			{

				int type = 0;
				if(addr.find("spectrum") != string::npos)
					type = TYPE_SPECTRUM;
				else
					type = TYPE_WAVEFORM;
				
				string starget = "";
				
				for (int i = 0; i < message.getNumArgs(); i++) 
				{
					if (message.getArgType(i) == osc::TYPE_FLOAT)
					{
						try {

							if(type == TYPE_SPECTRUM)
								spectrum[i] = message.getArgAsFloat(i);
							if(type == TYPE_WAVEFORM)
								waveform[i] = message.getArgAsFloat(i);
						}
						catch (...) {

						}

					}
					else if (message.getArgType(i) == osc::TYPE_STRING)
					{
						try {
							starget = message.getArgAsString(i);
							
						}
						catch (...) {
						}
						
					}
				}
				
				if(gs->visualsMap->find(starget) != gs->visualsMap->end())
				{
					if(type == TYPE_SPECTRUM)
						memcpy( (*(gs->visualsMap))[starget]->spectrum, spectrum, SAMPLE_WINDOW_SIZE * sizeof(float) );
					else
						memcpy( (*(gs->visualsMap))[starget]->waveform, waveform, SAMPLE_WINDOW_SIZE * sizeof(float) );
				}
			}
            
        }
    }
	
};