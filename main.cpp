#include "Agent.h"
#include "CrowdObject.h"
#include "Wall.h"
#include "CrowdWorld.h"
#include <stdlib.h>
#include <fstream> 
#include <istream>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

using namespace std;


Json::Value readJsonFromFile(const char * file);
void writeJsonToFile(Json::Value v, const char * file);

Json::Value readJsonFromFile(const char * file){
  Json::Reader reader;
  Json::Value root;
  std::ifstream data_file;
  data_file.open( file);
  
  if (!data_file.is_open()) {
    std::cout << "Could not open file: " << file << std::endl;
    exit(1);
  }
  
  std::string the_str ( std::istreambuf_iterator<char>( data_file ),
			(std::istreambuf_iterator<char>() ) );
  
  data_file.close();

  bool success = reader.parse(the_str , root );

  if( ! success ){
    std::cout << "bad json data!\n";
    std::cout << reader.getFormatedErrorMessages();
    exit(1);
  } 

  return root;

}

int mysleep( unsigned long millis ){
  struct timespec req={0};
  time_t sec = (int) (millis / 1000 );
  millis = millis - (sec * 1000);
  req.tv_sec = sec;
  req.tv_nsec = millis * 1000000L;
  return nanosleep( &req, NULL);
}

int main( int argc, char ** argv){
  Json::Value data;
  if( argc == 2){
    data = readJsonFromFile( argv[1] );
  }else {
    data = readJsonFromFile( "data/test.json" );
  }
  std::srand(0);
  int steps = data["steps"].asInt();
  float deltat = data["timeslice"].asDouble();
  Agent * a = new Agent(data["agents"][0u]);
  Render * r = Render::getInstance();
  CrowdObject * cos;
  cos = twoWalls(data["walls"][0u]);
  CrowdWorld c(data);
  while(! r->isInitialized() ){

  }
  for( int i = 0 ; i < steps ; i++){
    c.updateAgents();
    c.calcForces();
    c.stepWorld(deltat);
    c.print();
    r->update(deltat);
    mysleep( 10 );
  }

  return 0;

}
