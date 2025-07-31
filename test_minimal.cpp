#include "Agent.h"
#include "CrowdObject.h"
#include <json/reader.h>
#include <json/value.h>
#include <iostream>
#include <fstream>

using namespace std;

Json::Value readJsonFromFile(const char * file){
  Json::Reader reader;
  Json::Value root;
  std::ifstream data_file;
  data_file.open( file);
  std::string the_str ( std::istreambuf_iterator<char>( data_file ),
			(std::istreambuf_iterator<char>() ) );

  bool success = reader.parse(the_str , root );

  if( ! success ){
    std::cout << "bad json data!\n";
    exit(1);
  } 

  return root;
}

int main(){
  cout << "Starting minimal test..." << endl;
  
  try {
    cout << "Loading JSON..." << endl;
    Json::Value data = readJsonFromFile("data/test.json");
    cout << "JSON loaded successfully" << endl;
    
    cout << "Creating Agent..." << endl;
    Agent * a = new Agent(data["agents"][0u]);
    cout << "Agent created successfully" << endl;
    
    cout << "Deleting Agent..." << endl;
    delete a;
    cout << "Agent deleted successfully" << endl;
    
  } catch (const std::exception& e) {
    cout << "Exception caught: " << e.what() << endl;
  }
  
  cout << "Test completed" << endl;
  return 0;
}
