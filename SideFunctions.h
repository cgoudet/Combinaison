#ifndef SIDEFUNCTIONS_H
#define SIDEFUNCTIONS_H

#include <string>
#include <vector>

int SplitVarNames( string name, vector<string> &splittedNames );
int MergedVarNames( string name, vector<string> &mergedNames );
int FindVariable( string var, vector<string> &varList );
#endif
