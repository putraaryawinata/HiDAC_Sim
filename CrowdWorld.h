#include "CrowdObject.h"
#include "Agent.h"
#include "Wall.h"
#include "Render.h"
#include <vector>
#include <json/value.h>

class CrowdWorld {
 protected:
  std::vector<Agent * > agentList;
  std::vector<CrowdObject * > objectList;
  
 private:
  void createNewObject(const Json::Value& v);
  
 public:
  //build from JSON value
  CrowdWorld();
  CrowdWorld( const Json::Value& w );
  virtual ~CrowdWorld();
  
  //updates each agent with visibility and collision information
  virtual void updateAgents();

  //calcs forces for each agent
  virtual void calcForces();
  
  //applies forces for each agent
  virtual void stepWorld(float deltaT);

  //output functions
  void print();
  void render();
};
