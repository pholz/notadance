#include "FlickrGen.h"
#include <sstream>

FlickrGen::FlickrGen(string _name, int _mode, app::App *_app)
{
    username = _name;
    mode = _mode;
    app = _app;
    init();
}

FlickrGen::~FlickrGen()
{
    rand = 0;
    delete rand;
}

void FlickrGen::init()
{
    time_t seconds;
    
    seconds = time (NULL);
    
    rand = new Rand((int) seconds);
    
    if(mode == SK_USERPICS)
    {
		string pics[] = { "pic1.png", "pic2.png", "pic3.png"};
		Surface surfs[3];
		
		string path = "/Users/holz/Documents/maxpat/media/tmp/";
		
		for(int i = 0; i < 3; i++)
		{
			Surface surf = loadImage(app->getResourcePath(pics[i]));
			stringstream ss; ss << path << pics[i];
			writeImage(ss.str(), surf);
			surfs[i] = surf;
		}
		
		
		for(int i = 0; i < 10; i++)
		{
			textures.push_back(gl::Texture(surfs[i%3]));
			names.push_back(pics[i%3]);
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

void FlickrGen::createTextureFromURL() 
{
    gl::Texture tex = gl::Texture( loadImage( loadUrl( photourls.back() ) ) );
    cout << "Loaded:" << photourls.back().str() << endl;
    photourls.pop_back();
    textures.push_back( tex );
}