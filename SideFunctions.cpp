#include <iostream>
#include "Combine.h"

int SplitVarNames( string name, vector<string> &splittedNames ) {

  splittedNames.clear();
  
  string var;
  //Find the * in the name to identify the splitted variables and remove the name from allVar
  while ( name.size() ) {
    int index = name.find_first_of("*");
    if ( index == -1 ) {
      splittedNames.push_back( name );
      name = "";
    }
    else {
      var=name.substr( 0, name.find_first_of("*") );
      splittedNames.push_back( var );
      name = name.substr( index+1 );
    }//end else
  }//end while
  

  return 0;
}


int FindVariable( string var, vector<string> &varList ){

  for ( unsigned int i = 0; i < varList.size(); i++ ) {
    if ( var == varList[i] ) return i;
  }

  return -1;
}


int MergedVarNames( string name, vector<string> &mergedNames ) {

  mergedNames.clear();
  
  string var;
  //Find the = in the name to identify the splitted variables and remove the name from allVar
  while ( name.size() ) {
    int index = name.find_first_of("=");
    if ( index == -1 ) {
      mergedNames.push_back( name );
      name = "";
    }
    else {
      var=name.substr( 0, name.find_first_of("=") );
      mergedNames.push_back( var );
      name = name.substr( index+1 );
    }//end else
  }//end while
  

  return 0;
}
