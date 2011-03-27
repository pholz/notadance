//
//  FlickrGen.h
//  Skels
//
//  Created by Peter Holzkorn on 27/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#include "cinder/URL.h"
#include "cinder/Xml.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"


#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <time.h>

#define APIKEY "60ecb5dc0fc829a46edc4eca1d45685b"

#define SK_ALLPICS 0
#define SK_USERPICS 1

using namespace std;
using namespace ci;

class FlickrGen
{
public:
    FlickrGen(string _name, int _mode)
    {
        username = _name;
        mode = _mode;
        init();
    }
    
    ~FlickrGen()
    {
        rand = 0;
        delete rand;
    }
    
    void init()
    {
        time_t seconds;
        
        seconds = time (NULL);
        
        rand = new Rand((int) seconds);
        
        if(mode == SK_USERPICS)
        {
        
            Url userurl = Url( string("http://api.flickr.com/services/rest/?method=flickr.people.findByUsername&api_key=")+APIKEY+string("&username=") + username );
            
            const XmlTree xml( loadUrl( userurl ) );
            
            cout << xml.getValue();
            
            for( XmlTree::ConstIter item = xml.begin( "rsp/user" ); item != xml.end(); ++item ) {
                userid = item->getAttributeValue<string>( "nsid" );
            }
            
            const XmlTree photos( loadUrl( Url( string("http://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=")+APIKEY+string("&user_id=")+userid+string("&extras=views") ) ) );
            
            
            int i = 0;
            for( XmlTree::ConstIter item = photos.begin( "rsp/photos/photo" ); item != photos.end() && i < 10; ++item, ++i ) {
                
                string farmid = item->getAttributeValue<string>("farm");
                string serverid = item->getAttributeValue<string>("server");
                string photoid = item->getAttributeValue<string>("id");
                string secret = item->getAttributeValue<string>("secret");
                int views = item->getAttributeValue<int>("views");
                
                string photourl = "http://farm"+farmid+".static.flickr.com/"+serverid+"/"+photoid+"_"+secret+"_b.jpg";
                
                photourls.push_back( Url(photourl) );
                
                createTextureFromURL();
            }
        
        }
        else
        {
        
            int ryear = rand->nextInt(2005, 2009);
            int rmonth = rand->nextInt(1,12);
            int rday = rand->nextInt(1,20);
            
            stringstream ss, ss2;
            ss << ryear << "-" << rmonth << "-" << rday;
            ss2 << ryear << "-" << rmonth << "-" << (rday+10);
            
            cout << ss.str() << " - " << ss2.str() << endl;
            
            const XmlTree photos( loadUrl( Url( 
                string("http://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=")+APIKEY+string("&min_upload_date='")+ss.str()+string("%2000:00:00'")+
                    string("&max_upload_date='")+ss2.str()+string("%2000:00:00'&extras=views&per_page=500") 
                                               ) ) );
            
            int i = 0;
            for( XmlTree::ConstIter item = photos.begin( "rsp/photos/photo" ); item != photos.end() && i < 10; ++item ) {
                
                string farmid = item->getAttributeValue<string>("farm");
                string serverid = item->getAttributeValue<string>("server");
                string photoid = item->getAttributeValue<string>("id");
                string secret = item->getAttributeValue<string>("secret");
                int views = item->getAttributeValue<int>("views");
                
                if(views <= 1)
                {
                    string photourl = "http://farm"+farmid+".static.flickr.com/"+serverid+"/"+photoid+"_"+secret+"_b.jpg";
                    
                    photourls.push_back( Url(photourl) );
                    
                    createTextureFromURL();
                    
                    ++i;
                }
            }
        }
        
    }
    
    void createTextureFromURL() 
    {
        gl::Texture tex = gl::Texture( loadImage( loadUrl( photourls.back() ) ) );
        cout << "Loaded:" << photourls.back().str() << endl;
        photourls.pop_back();
        textures.push_back( tex );
    }
  
    string username, userid;
    vector<Url> photourls;
    vector<gl::Texture> textures;
    Rand *rand;
    int mode;
};
