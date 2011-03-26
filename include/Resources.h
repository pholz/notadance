#pragma once
#include "cinder/CinderResources.h"

#define RES_CONFIG              CINDER_RESOURCE( ../resources/, configIR.xml, 128, TEXT )
#define RES_WORLD               CINDER_RESOURCE( ../resources/, world.xml, 130, TEXT )
#define RES_MEM_FRAG			CINDER_RESOURCE(../resources/, mem_frag.glsl, 131, GLSL)
#define RES_PASS_VERT			CINDER_RESOURCE(../resources/, pass_vert.glsl, 132, GLSL)
#define RES_EXPIRE_FRAG			CINDER_RESOURCE(../resources/, expire_frag.glsl, 133, GLSL)