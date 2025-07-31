#include "CrowdObject.h"

CrowdObject::CrowdObject( ){

}

CrowdObject::CrowdObject( const Json::Value& c){
  // Initialize members first
  norm[0] = 0.0;
  norm[1] = 0.0;
  pos[0] = 0.0;
  pos[1] = 0.0;
  myType = AGENT; // default
  
  if(c.isNull() || !c.isObject()) {
    return; // Exit early if invalid JSON
  }
  
  std::string s = c["type"].asString();
  
  if (s == "fallen agent")
    myType = FALLEN_AGENT; 
  else if (s == "attractor")
    myType = ATTRACTOR;
  else if (s == "wall")
    myType = WALL;
  else if (s == "agent")
    myType = AGENT;
  else if (s == "obstacle")
    myType = OBSTACLE;

  // Manually set array values instead of using v2fFromJson
  if(c.isMember("norm") && c["norm"].isArray() && c["norm"].size() >= 2){
    norm[0] = c["norm"][0u].asDouble();
    norm[1] = c["norm"][1u].asDouble();
  }
  
  if(c.isMember("pos") && c["pos"].isArray() && c["pos"].size() >= 2){
    pos[0] = c["pos"][0u].asDouble();
    pos[1] = c["pos"][1u].asDouble();
  }
}

//all methods of CrowdObject will be overridden. 
bool CrowdObject::isVisible( v2f pos, v2f dir, float vislength, float viswidth){
  std::cout << "crap!\n";
  return false; 
}


void CrowdObject::getNorm( v2f ret ){
  v2fCopy( norm, ret );
  return;
}


float CrowdObject::getDistance( v2f otherPos ){
  v2f v;
  getDirection( otherPos, v);
  std::cout << "BAD THINGS!\n";
  return v2fLen( v );
}


void CrowdObject::getDirection( v2f otherPos, v2f res ){
  v2fSub(pos, otherPos, res);
  return;
}


int CrowdObject::getType(){
  return myType;
}


void CrowdObject::getVelocity(v2f ret){
  std::cout << "Crap!\n";
  v2fMult( ret, 0.0, ret );
}
