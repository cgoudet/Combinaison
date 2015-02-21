#include <string>
#include "TFile.h"
#include "RooBinning.h"
#include "TString.h"
#include "TList.h"
#include "TDOMParser.h"
#include "TXMLDocument.h"
#include "TXMLNode.h"
#include "TXMLAttr.h"
#include <vector>
#include "RooWorkspace.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooStats/ModelConfig.h"
#include <sstream>
#include "RooCategory.h"
#include "RooDataSet.h"
#include <map>
#include "RooSimultaneous.h"
#include "RooPlot.h"
#include "RooProdPdf.h"
#include "TH1.h"
#include "TCanvas.h"
#include "RooArgList.h"
#include "RooMoment.h"
#include "RooGaussian.h"
//#include "/afs/in2p3.fr/home/d/delgove/private/Combination/Couplage/CloseCoutSentry.cc"

using namespace RooFit;
using namespace RooStats;
using namespace std;

class Combine{
  
 public:

  Combine(string filename);
  ~Combine();
  void MergeWorkspace();
  void CreateModelConfig();
  void Print();
  void MergeCategory();
  void CreateFinalWorkspace();
  int SplitPOI();
  
 private:
  ModelConfig* CreateModelConfig(const RooWorkspace* wsinput, RooWorkspace* wsout);
  int SplitVarNames( string name, vector<vector<string>> &splittedNames );
  string EditLine( vector< vector< string >> &splittedNames, string combinedVar );
  int FindVariable( string var, vector<string> &varList );
  void GetPOIMinMax(TString poi_list, vector<string> *combined_pois_name, vector<string> *max_pois, vector<string> *min_pois);
  void GetPOI(TString poi_list, vector<string> *pois_name);
  void GetComponents(RooProdPdf *prodpdf, RooArgSet *np);
  string m_combined_name;
  string m_combined_file_name;
  string m_combined_workspace_name;
  string m_combined_data_name;
  vector<string> m_combined_pois_name;
  vector<string> m_cat_name;
  vector<string> m_max_pois;
  vector<string> m_min_pois;
  vector<string> m_pdf_name;
  string m_combined_ModelConfig_name;
  vector<bool> m_containgg;
  vector<string> m_channels_name;
  vector<string> m_files_name;
  vector<string> m_workspaces_name;
  vector<string> m_datas_name;
  vector<bool> m_haslumi;
  vector<string> m_newLumi_name;
  vector< vector<string> > m_pois_name;
  vector<string> m_ModelConfigs_name;
  vector< vector<string> > m_renamemap_new;
  vector< vector<string> > m_renamemap_old;

  std::map<std::string, std::string> m_renamemap_go;

  RooWorkspace *m_temp_w;
  ModelConfig *m_newmc;
  RooDataSet *m_data;
  RooSimultaneous *m_combPdf;
};
