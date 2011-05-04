#include "common.h"

string dec3(float f)
{
	stringstream ss;
	
	ss << (math<float>::floor(f * 1000.0f) / 1000.0f);
	
	string s = ss.str();
	
	while(strlen(s.c_str()) < 8)
	{
		stringstream ss2;
		ss2 << " " << s;
		s = ss2.str();
	}
	
	return s;
}

string str(Vec3f v, string prefix, string spacer)
{
	stringstream ss;
	ss << prefix << spacer << dec3(v.x) << "\t\t" << dec3(v.y) << "\t\t\t\t" << dec3(v.z);
	return ss.str();
}