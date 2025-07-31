#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <math.h> 
#include <json/value.h>
#include <iostream>

#define MY_EPSILON 0.0005

typedef float v2f[2];

void v2fCopy( v2f src, v2f dst );


float v2fDot( v2f v1, v2f v2);


float v2fLen( v2f v );


void v2fNormalize( v2f v, v2f res );


float v2fCross( v2f v1, v2f v2);


void v2fTangent( v2f v, v2f ret );

void v2fNegate( v2f v);

// Additional vector operations for ORCA
float v2fLenSq( v2f v );
void v2fCross2D( v2f v, v2f result );


void v2fSub( v2f v1, v2f v2, v2f res );


void v2fAdd( v2f v1, v2f v2, v2f res );
void v2fAdd( v2f v1, v2f v2, float t, v2f res);

void v2fPrint( v2f p);
void v2fPrint( const char * label, v2f p);
void v2fMult( v2f v, float f, v2f res );

void v2fFromJson( Json::Value v, v2f res);

float ptToLineDist( v2f pos, v2f start, v2f dir, float len );

void v2fEpsilon( v2f v );

#endif

