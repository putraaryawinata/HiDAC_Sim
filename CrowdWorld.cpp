#include "CrowdWorld.h"

CrowdWorld::CrowdWorld(){
  Render * r = Render::getInstance();
}

//adds the new CrowdObject(s) to the end of the vector
void CrowdWorld::createNewObject(const Json::Value& v){
  std::string s = v["type"].asString();
  std::string fa = "fallen agent";
  std::string a = "attractor";
  std::string w = "wall";
  std::string ag = "agent";
  std::string ob = "obstacle";
  if (s.compare(w) == 0){
    v2f st, en;
    st[0] = v["start"][0u].asDouble();
    st[1] = v["start"][1u].asDouble();
    en[0] = v["end"][0u].asDouble();
    en[1] = v["end"][1u].asDouble();
    
    Wall* w1 = new Wall(st, en);
    Wall* w2 = new Wall(en, st);
    
    objectList.push_back( w1 );
    objectList.push_back( w2 );
    Render * r = Render::getInstance();
    r->drawThis(w1, "wall.mesh");
    return;
  }
  if (s.compare(ag) == 0){
    
    objectList.push_back( new Agent(v) );
  }
  else 
    objectList.push_back( new CrowdObject(v) );

}

CrowdWorld::CrowdWorld( const Json::Value& w ){

  Render * r = Render::getInstance();
  //loading from file
  int numAgents = w["agents"].size();
  for(int i = 0; i < numAgents ; i++ ){
    Agent * a = new Agent(w["agents"][i]);
    agentList.push_back( a );
    r->drawThis(a, a->getMesh());
  }

  int numObjects = w["objects"].size();
  for(int i = 0; i < numObjects ; i++){

    createNewObject( w["objects"][i] );
  }

  //end loading


}

CrowdWorld::~CrowdWorld(){
  Render * r = Render::getInstance();
  r->destroyInstance();
}



//updates each agent with visibility and collision information
void CrowdWorld::updateAgents(){
  for( std::vector<Agent *>::iterator a = agentList.begin();
       a != agentList.end();
       a++ ){
    //contains some inneficiency
    for( std::vector<Agent *>::iterator b = agentList.begin();
	 b != agentList.end();
	 b++ ){
      if (a != b) {
	(*a)->checkVisible(*b);
	(*a)->checkCollide(*b);
      }
    }
    for( std::vector<CrowdObject *>::iterator c = objectList.begin(); 
	 c != objectList.end();
	 c++ ){
      
      (* a)->checkVisible(* c);
      (* a)->checkCollide(* c);

    }

  }
}

//calcs forces for each agent
void CrowdWorld::calcForces(){
  for( std::vector<Agent * >::iterator it = agentList.begin();
       it != agentList.end();
       it++ ){
    (*it)->calculateForces();
  }
}
  
//applies forces for each agent
void CrowdWorld::stepWorld( float deltaT ){
  for( std::vector<Agent *>::iterator it = agentList.begin();
       it != agentList.end();
       it++ ){
    (* it)->applyForces(deltaT);
    (* it)->reset();
  }
}

void CrowdWorld::print(){
  for( std::vector<Agent *>::iterator it = agentList.begin();
       it != agentList.end();
       it++ ){

    (* it)->print();
  }
}

void CrowdWorld::render(){
  Render * r = Render::getInstance();
  //requires a float, but that shouldn't affect anything
  r->update(0.1);

}
