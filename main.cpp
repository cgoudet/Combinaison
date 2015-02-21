#include "Combine.h"

int main() {
  //  string filename = "/sps/atlas/d/delgove/private/Combination/comb_raw/ManagerWorkspace/comb_test.xml";
  //string filename = "/afs/in2p3.fr/home/d/delgove/private/Combination/Workspace/ManagerWorkspace/comb_inv.xml";
  //string filename = "comb_ind4.xml";
  string filename = "comb_test2.xml";
  Combine combine_cl(filename);
  combine_cl.SplitPOI();
  // combine_cl.Print();
  // combine_cl.MergeWorkspace();
  // combine_cl.CreateModelConfig();
  // combine_cl.MergeCategory();
  // combine_cl.CreateFinalWorkspace();

  return 0;
}
