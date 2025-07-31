#include "Wall.h"

Wall::Wall(){
  myType = WALL;
  
}

Wall::Wall(v2f s, v2f e ){
  myType = WALL;
  v2f w;
  v2fCopy(s, start);
  v2fCopy(e, end);
  v2fSub( end, start , w);
  v2fTangent( w, norm );
  //not positive this vector needs to be normalized
  v2fNormalize( norm, norm );


}


//two lines in start + direction format along with their lengths
bool testLineIntersection( v2f p1, v2f d1, float l1, v2f p2, v2f d2, float l2){

  v2fNormalize( d1, d1);
  v2fNormalize( d2, d2);

  v2f delta;
  v2fSub( p2, p1, delta);
  float s, t;

  //parallel case
  if( v2fCross( d1, d2) == 0 ){
    return false;
  }

  s = v2fCross(delta, d2)/ v2fCross(d1, d2);
  t = v2fCross(delta, d1) / v2fCross(d1, d2);
  
  if(s <= l1 && s >= 0.0 && t <=l2 && t >= 0.0){
    return true;
  }

  return false;
}

//checking the intersection of a line and a rectangle
bool Wall::isVisible( v2f pos, v2f dir, float vislength, float viswidth){
  
  //check to see if either the start or end lies within viswidth of the line
  //this test is repeated in Agent.cpp
  if( ptToLineDist( start, pos, dir, vislength ) <= viswidth ){
    return true;
  }


  //otherwise, treat the rectangle as two lines. The wall is then visible if
  //its segment intersects any of those lines
  v2f walldir;
  v2fSub( end, start, walldir );
  float walllen = v2fLen( walldir );
  v2f tan;
  v2fTangent( dir, tan );
  v2f start1, start2;
  v2fAdd(pos, tan , viswidth , start1);
  v2fAdd(pos, tan, -1*viswidth, start2);
  return 
    testLineIntersection( pos , dir , vislength , start , walldir , walllen ) 
    || 
    testLineIntersection( pos , dir , vislength , start , walldir , walllen);
}

void Wall::getNorm( v2f ret){
  v2fCopy( norm, ret );

}

//find the closest distance from our line to the point
float Wall::getDistance( v2f pos ){
  v2f v;
  getDirection( pos, v);
  return v2fLen(v);

}

//similar, but in vector form
void Wall::getDirection( v2f pos, v2f res ){
  v2f dir, pmins;
  v2f closept;
  v2fMult( dir, 0.0, dir );
  v2fMult( pmins, 0.0, pmins);
  v2fMult( closept, 0.0, closept);
  //get wall direction
  v2fSub( end, start, dir);
  float len = v2fLen(dir);
  v2fNormalize( dir, dir );

  //find the point along the line closest
  v2fSub( pos, start, pmins);
  float t = v2fDot( dir, pmins );

  //if not part of the line segment, the start or the end is closest
  if(t <= 0){
    v2fSub( pos, start, res);
    return;
  }
  if(t > len){
    v2fSub( pos, end, res);
    return;
  }

  //otherwise, use t
  v2fAdd( start, dir, t, closept);

  v2fSub( pos, closept, res );
  v2fEpsilon(res);
  return;
}

int Wall::getType(){
  return myType;
}

void Wall::getVelocity( v2f ret ){
  v2fMult( ret, 0.0, ret );
}

Wall * twoWalls( Json::Value w){
  v2f st, en;
  st[0] = w["start"][0u].asDouble();
  st[1] = w["start"][1u].asDouble();
  en[0] = w["end"][0u].asDouble();
  en[1] = w["end"][1u].asDouble();
  Wall * ws = new Wall[2];
  ws[0] = Wall(st, en);
  ws[1] = Wall(en, st);

  return ws;
}
