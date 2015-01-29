#include "RooStats/HistFactory/FlexibleInterpVar.h"
#include "RooCustomizer.h"
#include "Combine.h"
#include "CloseCoutSentry.h"
#include "SideFunctions.h"

using namespace RooFit;
using namespace RooStats;
using namespace std;

Combine::~Combine(){
}

Combine::Combine(string filename){
    
  TDOMParser xmlparser;
  //Check if the xml file is ok 
  xmlparser.ParseFile( filename.c_str() );
  TXMLDocument* xmldoc = xmlparser.GetXMLDocument();
  TXMLNode *root_node  = xmldoc->GetRootNode();
  TXMLNode *next_node = root_node->GetChildren();
  TString attr_name;
  TString attr_value;
  while(next_node!=0) {
    bool iscombined = false;
    TString node_name = next_node->GetNodeName();
    if( node_name == "Channel") {      
      //Read the line starting by Channel
      TList *list = next_node->GetAttributes();
      if(list!=0) {
	TIterator *it = list->MakeIterator();
	iscombined = false;
	
	for (TXMLAttr * atr = (TXMLAttr*)it->Next();atr!=0;atr= (TXMLAttr*)it->Next()){
	  attr_name=atr->GetName();
	  attr_value=atr->GetValue();
	  if(attr_name=="IsCombined" && attr_value == "true") {
	    iscombined = true;
	  }
	}
	it->Reset();
	if(iscombined) {
	  // If the line described the combined workspace, fill the name attribute
	  for (TXMLAttr * atr = (TXMLAttr*)it->Next();atr!=0;atr= (TXMLAttr*)it->Next()){
	    attr_name=atr->GetName();
	    attr_value=atr->GetValue();
	    if(attr_name=="Name") {
	      m_combined_name = attr_value;
	    }
	  }
	}
	else {
	  //Else, fill the attribute list of used channels with the current channel 
	  for (TXMLAttr * atr = (TXMLAttr*)it->Next();atr!=0;atr= (TXMLAttr*)it->Next()){
	    attr_name=atr->GetName();
	    attr_value=atr->GetValue();
	    if(attr_name=="Name") {
	      m_channels_name.push_back(attr_value.Data());
	    }
	  }
	}
      }//End channel attibutes not empty

      TXMLNode *channel_node = next_node->GetChildren();
      TString children_node_name = channel_node->GetNodeName();      
      if(iscombined) {
	//Fill remaining information for the combined workspace
	while(channel_node!=0) {
	  children_node_name = channel_node->GetNodeName(); 
	  TList *list_2 = channel_node->GetAttributes();
	  if(list_2!=0) {
	    TIterator *it_2 = list_2->MakeIterator();
	    for (TXMLAttr * atr = (TXMLAttr*)it_2->Next();atr!=0;atr= (TXMLAttr*)it_2->Next()){
	      attr_name=atr->GetName();
	      attr_value=atr->GetValue();
	      if(attr_name == "Name") {
		if(children_node_name=="File")        m_combined_file_name=attr_value;
		if(children_node_name=="Workspace")   m_combined_workspace_name=attr_value;
		if(children_node_name=="ModelConfig") m_combined_ModelConfig_name=attr_value;
		if(children_node_name=="ModelData")   m_combined_data_name=attr_value;
		if(children_node_name=="ModelPOI")    GetPOIMinMax(attr_value,&m_combined_pois_name,&m_max_pois,&m_min_pois);
	      }
	    }
	  }
	  channel_node = channel_node->GetNextNode();
	}
      }//End isCombined

      else{ //Fill information about the current channel
	while(channel_node!=0) {
	  children_node_name = channel_node->GetNodeName(); 
	  TList *list_2 = channel_node->GetAttributes();
	  if(list_2!=0) {
	    TIterator *it_2 = list_2->MakeIterator();
	    for (TXMLAttr * atr = (TXMLAttr*)it_2->Next();atr!=0;atr= (TXMLAttr*)it_2->Next()){
	      attr_name=atr->GetName();
	      attr_value=atr->GetValue();
	      if(attr_name == "Name") {
		if(children_node_name=="File")	      m_files_name.push_back(attr_value.Data());
		if(children_node_name=="Workspace")   m_workspaces_name.push_back(attr_value.Data());
		if(children_node_name=="ModelConfig") m_ModelConfigs_name.push_back(attr_value.Data());
		if(children_node_name=="ModelData")   m_datas_name.push_back(attr_value.Data());
		if(children_node_name=="ModelPOI") {
		  vector<string> temp;
		  GetPOI(attr_value,&temp);
		  m_pois_name.push_back(temp);		  
		}
	      }//End attr_name ==Name
	    }
	  }

	  //Fill the rename map component
	  if(children_node_name=="RenameMap"){
	    vector<string> new_temp;
	    vector<string> old_temp;
	    TXMLNode * rename_node =channel_node->GetChildren();
	    TString rename_node_name;
	    while(rename_node!=0) {
	      rename_node_name = rename_node->GetNodeName();
	      if( rename_node_name == "Syst") {
		TList *list_3 = rename_node->GetAttributes();
		if(list_3!=0) {
		  TIterator *it_3 = list_3->MakeIterator();
		  for (TXMLAttr * atr = (TXMLAttr*)it_3->Next();atr!=0;atr= (TXMLAttr*)it_3->Next()){
		    attr_name=atr->GetName();
		    attr_value=atr->GetValue();
		    if(attr_name=="OldName")  old_temp.push_back(attr_value.Data());
		    if(attr_name=="NewName")  new_temp.push_back(attr_value.Data());
		  }
		}
	      }
	      rename_node = rename_node->GetNextNode();
	    }
	    m_renamemap_old.push_back(old_temp);
	    m_renamemap_new.push_back(new_temp);
	  }// End rename map

	  channel_node = channel_node->GetNextNode();
	}// End filling a channel
      }//End if not combined
    }//End nodeName = channel
    next_node = next_node->GetNextNode();
  }//end loopo other all nodes
 
}

void Combine::GetPOIMinMax(TString poi_list, vector<string> *combined_pois_name, vector<string> *max_pois, vector<string> *min_pois){
  TObjArray * list_poi = poi_list.Tokenize(",[-]");
  for(int i=0; i<list_poi->GetEntriesFast()-2; i=i+3){
    TObjString *string = (TObjString*)list_poi->At(i);
    TString temp =  string->GetString();
    combined_pois_name->push_back(temp.Data());
    string = (TObjString*)list_poi->At(i+1);
    temp =  string->GetString();
    min_pois->push_back(temp.Data());
    string = (TObjString*)list_poi->At(i+2);
    temp =  string->GetString();
    max_pois->push_back(temp.Data());
  }  
}

void Combine::GetPOI(TString poi_list, vector<string> *pois_name){
  TObjArray * list_poi = poi_list.Tokenize(",[-]");
  for(int i=0; i<list_poi->GetEntriesFast(); i++){
    TObjString *string = (TObjString*)list_poi->At(i);
    TString temp =  string->GetString();
    pois_name->push_back(temp.Data());
  }  
}

void Combine::Print(){
  cout << "The combined channel : " << m_combined_name << endl;
  cout << " In the file : " << m_combined_file_name << endl;
  cout << " Combined workspace : " << m_combined_workspace_name << endl;
  cout << " Model : " << m_combined_ModelConfig_name << endl;
  cout << " DataSet : " << m_combined_data_name << endl;
  cout << " POI : " << endl;
  for(int i =0; i< (int) m_combined_pois_name.size(); i++){
    cout << "       " << m_combined_pois_name.at(i) << " with range : [" << m_min_pois.at(i) << "," << m_max_pois.at(i) << "]" << endl;
  }
  cout << endl;
  
  cout << "Different channels : " << endl;
  for(int i =0; i< (int) m_channels_name.size(); i++){
    cout << " Channel : " << m_channels_name.at(i) << endl;
    cout << "  In the file : " << m_files_name.at(i) << endl;
    cout << "  Combined workspace : " << m_workspaces_name.at(i) << endl;
    cout << "  Model : " << m_ModelConfigs_name.at(i) << endl;
    cout << "  DataSet : " << m_datas_name.at(i) << endl;
    cout << "  POI : " << endl;
    for(int j =0; j < (int) m_pois_name.at(i).size(); j++){
      TString temp=m_pois_name.at(i).at(j);
      if( !temp.Contains("dummy")) {
	cout << "        " << m_pois_name.at(i).at(j) << " ---> " << m_combined_pois_name.at(j) << endl;
      }
    }
    cout << "  Rename Map : " << endl;
    for(int j =0; j < (int ) m_renamemap_new.at(i).size(); j++){
      cout << "        " << m_renamemap_old.at(i).at(j) << " ---> " << m_renamemap_new.at(i).at(j) << endl;      
    }
  }
}

//=============================
void Combine::MergeWorkspace(){
  
  cout << "Begin Merge Workspace" << endl;
  cout << endl;
  RooWorkspace* temp_w = new RooWorkspace("temp","temp");
  for(int i=0; i < (int) m_files_name.size();i++) {
    cout << "Workspace in " << m_files_name.at(i) << " : begin  " << endl;
    TFile *f = TFile::Open(m_files_name.at(i).c_str());
    stringstream old_str;
    stringstream new_str;
    stringstream data_new;
    stringstream data_old;

    RooWorkspace* old_w = (RooWorkspace*)f->Get(m_workspaces_name.at(i).c_str());
    if(old_w==0) cout <<  m_workspaces_name.at(i).c_str() << " is not a workspace"<< endl;

    RooArgSet avar = old_w->allVars();

    ModelConfig *mc = (ModelConfig*) old_w->obj(m_ModelConfigs_name.at(i).c_str());
    if(mc==0) cout <<  m_ModelConfigs_name.at(i).c_str() << " is not a ModelConfig"<< endl;

    RooArgSet obs = *mc->GetObservables();
    RooArgSet gobs = *mc->GetGlobalObservables();
    TIterator* gobs_itr = gobs.createIterator();
   
    RooArgSet func = old_w->allFunctions();

    //Rename all functions adding the name of the workspace they belong to 
    TIterator* func_itr = func.createIterator();
    for( RooAbsArg* v = (RooAbsArg*)func_itr->Next(); v!=0; v = (RooAbsArg*)func_itr->Next() ){
      v->SetName( (string(v->GetName())+"_"+m_channels_name.at(i)).c_str());
    }
    
    RooArgSet pdf = old_w->allPdfs();
    RooArgSet pdf2 = old_w->allPdfs(); //used for renaming the gaussian pdf which are not associated to a np
    TIterator* pdf_itr = pdf.createIterator();

    RooAbsPdf *mc_pdf = mc->GetPdf();
    m_pdf_name.push_back(string(mc_pdf->GetName()));
    
    // Rename the non gaussian pdf of the workspace
    for( RooAbsPdf* p = (RooAbsPdf*)pdf_itr->Next(); p!=0; p = (RooAbsPdf*)pdf_itr->Next() ){
      TString className = p->ClassName() ;
      if(className!="RooGaussian"){
	p->SetName( (string(p->GetName())+"_"+m_channels_name.at(i)).c_str());
	pdf2.remove(*p);
      }
    }	  

    // special treatment of the Lumi
    if(old_w->var("Lumi")){
      cout << "Workspace contains Lumi" << endl;
      TString newlumi_name;
      TString newpdf_name;
      TString newlumig_name;
      
      //Rename the luminosity depending wether or not it appear in renamemap
      bool test_rename = false;
      for (int j=0; j < (int) m_renamemap_old.at(i).size();j++){
	if ( TString(m_renamemap_old.at(i).at(j))!="Lumi") continue;
	newlumi_name=m_renamemap_new.at(i).at(j);
	newpdf_name= m_renamemap_new.at(i).at(j)+"Constrain";
	newlumig_name = m_renamemap_new.at(i).at(j) + "_In";
	test_rename = true;
      }
      if(!test_rename){
	newlumi_name = "Lumi"+m_channels_name.at(i);
	newpdf_name= "LumiConstrain"+m_channels_name.at(i);
	newlumig_name = "Lumi_In"+m_channels_name.at(i);
      }     
    
      RooSimultaneous *combPdf = ( RooSimultaneous *)old_w->pdf( mc_pdf->GetName());    
      if( combPdf==0){
	cout << mc_pdf->GetName() << " not in the independent workspace : " << m_workspaces_name.at(i).c_str() << endl;
      }

      TString combpdf_name = combPdf->GetName();	
      RooCategory *tmpcat ;
      RooArgSet tmp_obs = *mc->GetObservables();
      TIterator *it_obs_tmp = tmp_obs.createIterator();
      //Look for the rooCategory within the observables
      for ( RooRealVar* v = (RooRealVar*)it_obs_tmp->Next(); v!=0; v = (RooRealVar*)it_obs_tmp->Next() ){
	if(old_w->cat(v->GetName())!=0)tmpcat = old_w->cat(v->GetName());
	else cout << v->GetName() << " is not a category " << endl;
      }  

      RooRealVar *lumi = old_w->var("Lumi");
      RooGaussian* lumic = (RooGaussian*) old_w->pdf("lumiConstraint"); 
      if(lumic==0) cout << "lumiConstraint not in the workspace" << endl;

      double sigma_lumi  = lumic->RooAbsReal::sigma(*lumi)->getVal();
      double mean_lumi= lumic->RooAbsReal::mean(*lumi)->getVal();

      if( fabs(sigma_lumi-1)>0.001 && mean_lumi==1){
	cout << "Transform in lognormal" << endl;
	combPdf->SetName( (string(combpdf_name.Data())+"_Obsolete").c_str());
	avar.remove(*lumi);
	pdf2.remove(*lumic);
	lumi->SetName("obsolete_lumi");
	lumic->SetName("obsolete_lumi_pdf");
	RooArgSet * param_gauss = lumic->getVariables();
	if(!param_gauss->remove(*param_gauss->find(lumi->GetName()))) cout << "problem lumi is not a variable of the gaussian constrain" << endl;
	if(param_gauss->getSize()!=1) cout << "problem there is an additionnal variable for the gaussian constrain" << endl;
	
	RooRealVar *lumi_gobs = old_w->var(param_gauss->first()->GetName());
	if(lumi_gobs==0) cout << param_gauss->first()->GetName() << "not in the old workspace " << endl; 

	lumi_gobs->SetName("obsolete_lumi_In");
	avar.remove(*lumi_gobs);
	std::string edit = TString::Format("Gaussian::%s(%s[-5,5],%s[0,-5,5],1)",newpdf_name.Data(), newlumi_name.Data(),newlumig_name.Data()).Data();
	old_w->factory(edit.c_str());
	old_w->var(newlumig_name.Data())->setConstant(1);
	vector<double> vals_up, vals_down;
	vals_up.push_back(1+sigma_lumi);
	vals_down.push_back((1-sigma_lumi));
	RooArgList lumiList(*old_w->var(newlumi_name.Data()));
	TString tag = TString::Format("lumi_fiv_%f", sigma_lumi);
	tag.ReplaceAll("0", "").ReplaceAll(".", "");
	RooStats::HistFactory::FlexibleInterpVar* lumi_fiv = new RooStats::HistFactory::FlexibleInterpVar(tag.Data(), tag.Data(), lumiList, 1., vals_down, vals_up);
	RooCustomizer cust(*combPdf,""/*m_channels_name.at(i).c_str()*/);
	cust.replaceArg(*lumi, *lumi_fiv);
	RooSimultaneous *combPdf_newlumi = dynamic_cast<RooSimultaneous *>(cust.build());
	RooArgSet gobs_1 =*mc->GetGlobalObservables();
	RooArgSet np_1 = *mc->GetNuisanceParameters();
	gobs_1.remove(*lumi_gobs);
	np_1.remove(*lumi);
	if(sigma_lumi>0.001) { 
	  gobs_1.add(*old_w->var(newlumig_name.Data()));
	  np_1.add(*old_w->var(newlumi_name.Data()));
	}
	else {
	  old_w->var(newlumi_name.Data())->setConstant(1);
	  cout << " sigma = " << sigma_lumi << ", not included (not the true lumi) " << endl;
	}
	mc->SetNuisanceParameters(np_1);
	mc->SetGlobalObservables(gobs_1);
	// change the pdf in the roosimultaneous 
	//cout << combpdf_name.Data() << endl;	
	
	RooSimultaneous *combPdf_newlumi_final = new RooSimultaneous(combpdf_name.Data(),combpdf_name.Data(),*tmpcat);;
	for(int k=0;k<tmpcat->numTypes();k++){
	  tmpcat->setIndex(k);
	  RooProdPdf *pdf_ind = (RooProdPdf *)combPdf_newlumi->getPdf(tmpcat->getLabel());
	  TString newname_ind = pdf_ind->GetName();	
	  pdf_ind->SetName((string(pdf_ind->GetName())+"obsolete").c_str());
	  RooArgSet *pdf_comp = new RooArgSet();
	  GetComponents(pdf_ind,pdf_comp);   
	  TIterator *pdf_itr1 = pdf_comp->createIterator();
	  RooAbsPdf *newlum_pdf = old_w->pdf(newpdf_name.Data());
	  //cout << combcat->getLabel() << endl;
	  for ( RooAbsPdf* v = (RooAbsPdf*)pdf_itr1->Next(); v!=0; v = (RooAbsPdf*)pdf_itr1->Next() ){
	    if(TString(v->GetName())==TString("obsolete_lumi_pdf")){
	      pdf_comp->remove(*v);
	      pdf_comp->add(*newlum_pdf);
	    }
	  }	 
	  RooProdPdf *npdf = new RooProdPdf(newname_ind.Data(),newname_ind.Data(),*pdf_comp);
	  // pdf_comp->Print();
	  combPdf_newlumi_final->addPdf(*npdf,tmpcat->getLabel()); 
	}
	mc->SetPdf(*combPdf_newlumi_final);
	mc_pdf = combPdf_newlumi_final;
      }  
    }//End if lumi exists in workspace


    // loop over the rename map, 
    //remove the rename parameter of the argset which contains all the var, 
    //rename the linked global observable and the gaussian pdf if find
    bool cat_newname=false;
    for (int j=0; j< (int) m_renamemap_old.at(i).size();j++){
      // Rename the category (not sure it is useful)
      if(old_w->cat(m_renamemap_old.at(i).at(j).c_str())!=0) {
	RooCategory* cat_temp =(RooCategory*)old_w->cat(m_renamemap_old.at(i).at(j).c_str());
	cat_temp->SetName(m_renamemap_new.at(i).at(j).c_str());
	data_new << m_renamemap_new.at(i).at(j).c_str() ;
	data_old << m_renamemap_old.at(i).at(j).c_str() ;
	cat_newname=true;
	m_cat_name.push_back(m_renamemap_new.at(i).at(j));
      }
      // Rename the variable
      else if( old_w->var(m_renamemap_old.at(i).at(j).c_str())!=0) {	
	avar.remove(*old_w->var(m_renamemap_old.at(i).at(j).c_str()));
	RooRealVar *np_temp = old_w->var(m_renamemap_old.at(i).at(j).c_str());
	np_temp->SetName(m_renamemap_new.at(i).at(j).c_str());

	// dealts with the pdf : search if Gaussian Constrain
	pdf_itr->Reset();
	TString pdf_name_order;
	TString pdf_name;
	TString className;
	int count_pdf=0;
	for( RooAbsPdf* p = (RooAbsPdf*)pdf_itr->Next(); p!=0; p = (RooAbsPdf*)pdf_itr->Next() ){
	  // new fix
	  TString np_name_raw = m_renamemap_old.at(i).at(j).c_str();
	  if( np_name_raw.BeginsWith("nuis_")){
	    np_name_raw.ReplaceAll("nuis_","");
	  }
	  // end new fix
	  className = p->ClassName() ;
	  pdf_name = p->GetName();
	  if(className=="RooGaussian" && pdf_name.Contains(np_name_raw)){
	    count_pdf++;  
	    if(count_pdf==1) {	      
	      pdf_name_order=pdf_name;
	    }
	    else if(count_pdf>1){
	      if(pdf_name.Length()<pdf_name_order.Length()) {
		pdf_name_order=pdf_name;
	      }
	    }
	  }	  
	}
	pdf_itr->Reset();
	// Check if there is not two possibilities for the pdf 
	if(count_pdf>0) {
	  int count_check_pdf=0;
	  for( RooAbsPdf* p = (RooAbsPdf*)pdf_itr->Next(); p!=0; p = (RooAbsPdf*)pdf_itr->Next() ){
	    pdf_name = p->GetName();
	    className = p->ClassName() ;
	    if(className=="RooGaussian" && pdf_name.Contains(m_renamemap_old.at(i).at(j).c_str())){
	      if(pdf_name.Length()==pdf_name_order.Length()) {
		count_check_pdf++;
	      }
	    }
	  }
	  if(count_check_pdf>1){
	    cout << "problem more than one matching" << endl;
	    return;
	  }
	  RooAbsPdf *p1 = old_w->pdf(pdf_name_order.Data());
	  p1->SetName((m_renamemap_new.at(i).at(j)+"Constrain").c_str());
	  pdf2.remove(*p1);
	}
	// dealts with the global obs
	gobs_itr->Reset();
	int count_go =0;
	TString go_name;
	TString go_name_order;
	for ( RooRealVar* v = (RooRealVar*)gobs_itr->Next(); v!=0; v = (RooRealVar*)gobs_itr->Next() ){
	  TString np_name_raw = m_renamemap_old.at(i).at(j).c_str();
	  if( np_name_raw.BeginsWith("nuis_")){
	    np_name_raw.ReplaceAll("nuis_","");
	  }
	  go_name = v->GetName();
	  if(go_name.Contains(np_name_raw)){
	    count_go++;	    
	    if(count_go==1) {
	      go_name_order=v->GetName();
	    }
	    else if(count_go>1){
	      if(go_name.Length()<go_name_order.Length()) {
		go_name_order=go_name;
	      }
	    }
	  }
	}
	// Check if there is not two possibilities for the global obs
	gobs_itr->Reset();
	if(count_go>0) {
	  int count_check=0;
	  for ( RooRealVar* v = (RooRealVar*)gobs_itr->Next(); v!=0; v = (RooRealVar*)gobs_itr->Next() ){
	    go_name = v->GetName();
	    if(go_name.Contains(m_renamemap_old.at(i).at(j).c_str())){
	      if(go_name.Length()==go_name_order.Length()) {
		count_check++;
	      }
	    }
	  }
	  if(count_check>1){
	    cout << "problem more than one matching" << endl;
	    return;
	  }
	  avar.remove(*old_w->var(go_name_order.Data()));
	  RooRealVar *gob_temp = old_w->var(go_name_order.Data());
	  gob_temp->SetName((m_renamemap_new.at(i).at(j)+"_In").c_str());
	  m_renamemap_go[ m_renamemap_new.at(i).at(j) + "_In_"+m_channels_name.at(i)]= go_name_order.Data();
	}
      }
      else {
	if(TString(m_renamemap_old.at(i).at(j))!="Lumi"){
	  cout << m_renamemap_old.at(i).at(j).c_str() << " not a variable in : " << m_files_name.at(i) << endl;
	  cout << m_renamemap_old.at(i).at(j).c_str() << " not included" << endl;
	}	
      }
    }
    //rename the remaining pdf 
    TIterator* pdf2_itr = pdf2.createIterator();
    
    // Rename the non gaussian pdf of the workspace
    for( RooAbsPdf* p = (RooAbsPdf*)pdf2_itr->Next(); p!=0; p = (RooAbsPdf*)pdf2_itr->Next() ){
      cout << "Be careful : gaussian pdf : " << p->GetName() << " remains uncorrelated with others workspace" << endl;
      p->SetName( (string(p->GetName())+"_"+m_channels_name.at(i)).c_str());     
    }	  
    
    //Take the name of the category if it is not renamed
    if(!cat_newname) {
      RooArgSet tmp_obs = *mc->GetObservables();
      TIterator *it_obs_tmp = tmp_obs.createIterator();
      for ( RooRealVar* v = (RooRealVar*)it_obs_tmp->Next(); v!=0; v = (RooRealVar*)it_obs_tmp->Next() ){
	if(old_w->cat(v->GetName())!=0){
	  m_cat_name.push_back(string(v->GetName()));
	}
      }
    }
    // Rename the poi 
    bool hasgg = false;
    for(int j=0; j < (int) m_pois_name.at(i).size();j++){
      if( m_pois_name.at(i).at(j)!="dummy") {
	if(old_w->var(m_pois_name.at(i).at(j).c_str())!=0){
	  if(m_combined_pois_name.at(j)=="mu_BR_gamgam"){
	    hasgg=true;
	  }
	  avar.remove(*old_w->var(m_pois_name.at(i).at(j).c_str()));
	  old_str << m_pois_name.at(i).at(j) << ",";
	  new_str << m_combined_pois_name.at(j) << ",";
	}
	else {
	  cout << m_pois_name.at(i).at(j) << " not a variable in : " << m_files_name.at(i) << endl;
	  cout << m_pois_name.at(i).at(j) << " not included" << endl;	
	}
      }
    }
    m_containgg.push_back(hasgg);
    
    // Dealt with the observable
    TIterator* obs_itr = obs.createIterator();
    for ( RooRealVar* v = (RooRealVar*)obs_itr->Next(); v!=0; v = (RooRealVar*)obs_itr->Next() ){
      if(old_w->var(v->GetName())!=0 && string(v->GetName())!="weightVar"){
	avar.remove(*old_w->var(v->GetName()));
      }
    }
    // add the name of the channel to the other variable
    TIterator* avar_itr = avar.createIterator();
    for ( RooRealVar* v = (RooRealVar*)avar_itr->Next(); v!=0; v = (RooRealVar*)avar_itr->Next() ){
      v->SetName((string(v->GetName())+"_"+m_channels_name.at(i)).c_str());
    }
    if(old_str.str().size()>10000 || new_str.str().size()>10000) {
      cout << "Increase the char * capacity the string has too many characters" << endl; 
    }
    
    // Rename the dataset
    RooAbsData *data =  old_w->data(m_datas_name.at(i).c_str());
    if(data==0){
      cout << m_datas_name.at(i).c_str() << " : dataset not in the old workspace" << endl;
    }
    data->SetName((m_channels_name.at(i)+"_data").c_str());
  
    // import all the components in the new workspace
   
    cout << "Import in the new workspace" << endl;
    CloseCoutSentry sentry(1);
    temp_w->import(*mc_pdf,RenameVariable(old_str.str().c_str(),new_str.str().c_str()),RecycleConflictNodes(),Silence());
    temp_w->import(*old_w->data((m_channels_name.at(i)+"_data").c_str()),RenameVariable(data_old.str().c_str(),data_new.str().c_str()));
    CloseCoutSentry sentry1(0);
    f->Close();
    cout << "Workspace in " << m_files_name.at(i) << " : done " << endl;
    cout << endl;
  }
  m_temp_w = temp_w;
  cout << "End Merge Workspace" << endl;
  cout << endl;
  cout << "Writing to file: " << "temp.root" << endl;
  temp_w->writeToFile("temp.root", true);
  cout << "Writing done to file: " << "temp.root" << endl;
  cout << endl;
}

void Combine::CreateModelConfig(){

  cout << "Create temporary Model Config : " << endl;
  
  ModelConfig * newmc = new ModelConfig("ModelConfigtemp",m_temp_w);
  RooArgSet newpoi,newnp,newgobs,newobs;
  // For poi is easy 
  for(int j=0;j< (int) m_combined_pois_name.size(); j++) {
    if(m_temp_w->var(m_combined_pois_name.at(j).c_str())!=0) {
      newpoi.add(*m_temp_w->var(m_combined_pois_name.at(j).c_str()));
    }
    else {
      cout << m_combined_pois_name.at(j).c_str() << " not in the workspace" << endl;
    }
  }
  for(int i=0; i < (int) m_files_name.size();i++) {
    TFile *f = TFile::Open(m_files_name.at(i).c_str());
    RooWorkspace* old_w = (RooWorkspace*)f->Get(m_workspaces_name.at(i).c_str());
    ModelConfig *mc = (ModelConfig*) old_w->obj(m_ModelConfigs_name.at(i).c_str());
   
    RooArgSet obs = *mc->GetObservables();
    TIterator *obs_itr = obs.createIterator();
    for ( RooRealVar* v = (RooRealVar*)obs_itr->Next(); v!=0; v = (RooRealVar*)obs_itr->Next() ){
      if(m_temp_w->var(v->GetName())!=0) {
	newobs.add(*m_temp_w->var(v->GetName()));
      }
    }
    RooArgSet gobs = *mc->GetGlobalObservables();
    RooArgSet np = *mc->GetNuisanceParameters();
 	
    for(int j=0; j< (int) m_renamemap_old.at(i).size(); j++){
      // To be sure that the rename variable is in the np set
      if(np.find(m_renamemap_old.at(i).at(j).c_str())) {
	if(m_temp_w->var(m_renamemap_new.at(i).at(j).c_str()) !=0) {
	  newnp.add(*m_temp_w->var(m_renamemap_new.at(i).at(j).c_str()));
	  np.remove(*old_w->var(m_renamemap_old.at(i).at(j).c_str()));
	}
	if(m_temp_w->var((m_renamemap_new.at(i).at(j)+"_In").c_str())!=0){
	  newgobs.add(*m_temp_w->var((m_renamemap_new.at(i).at(j)+"_In").c_str()));
	}
      }
    }
    TIterator *np_itr = np.createIterator();
    for ( RooRealVar* v = (RooRealVar*)np_itr->Next(); v!=0; v = (RooRealVar*)np_itr->Next() ){
      newnp.add(*m_temp_w->var((string(v->GetName())+"_"+m_channels_name.at(i)).c_str()));
    }
    TIterator *gobs_itr = gobs.createIterator();
    for ( RooRealVar* v = (RooRealVar*)gobs_itr->Next(); v!=0; v = (RooRealVar*)gobs_itr->Next() ){
      // Condition needed due to the fact that 2 go in the old can become 1 
      if(m_temp_w->var((string(v->GetName())+"_"+m_channels_name.at(i)).c_str())!=0) {
	newgobs.add(*m_temp_w->var((string(v->GetName())+"_"+m_channels_name.at(i)).c_str()));
      }
    }
    f->Close();
  }
  newmc->SetParametersOfInterest(newpoi);
  newmc->SetNuisanceParameters(newnp);
  newmc->SetGlobalObservables(newgobs);
  newmc->SetObservables(newobs);
  cout << "End Creation" << endl;
  m_newmc = newmc;
  m_temp_w->import(*m_newmc);
  m_temp_w->writeToFile("temp.root", true);
  cout << endl; 
  return;
}

void Combine::MergeCategory(){
  
  RooRealVar comb_weight("combWeight","combWeight",-1e09,1e09);
  m_temp_w->import(comb_weight,Silence());
  RooArgSet newobs = *m_newmc->GetObservables(); 
  RooCategory *combcat = new RooCategory("combCat","combCat");
  newobs.add(*combcat);
  newobs.add(comb_weight);
  m_newmc->SetObservables(newobs);
  for(int j=0; j < (int) m_channels_name.size() ; j++) {
    RooCategory *tmp_cat = m_temp_w->cat(m_cat_name.at(j).c_str());
    if(tmp_cat==0){
      cout << m_cat_name.at(j).c_str() << " not in the temporary worksapace" << endl;
      return;
    }
    for(int i=0;i<tmp_cat->numTypes();i++){
      tmp_cat->setIndex(i);
      combcat->defineType(tmp_cat->getLabel());
    }
  }
  cout << "Create Merged DataSet" << endl;
  RooDataSet *data = new RooDataSet(m_combined_data_name.c_str(),m_combined_data_name.c_str(),newobs,WeightVar(comb_weight));
  for(int j=0; j < (int) m_channels_name.size(); j++) {
    RooSimultaneous *tmp_pdf =(RooSimultaneous*)m_temp_w->pdf((m_pdf_name.at(j)+"_"+m_channels_name.at(j)).c_str());
    if(tmp_pdf==0){
      cout << (m_pdf_name.at(j)+"_"+m_channels_name.at(j)).c_str() << " not in the temporary worksapace" << endl;
      return;
    }
    RooDataSet * tmp_data = (RooDataSet*)m_temp_w->data((m_channels_name.at(j)+"_data").c_str());
    if(tmp_data==0){
      cout << (m_channels_name.at(j)+"_data").c_str() << " not in the temporary worksapace" << endl;
      return;
    }
    TList *cat_list = tmp_data->split(*m_temp_w->cat(m_cat_name.at(j).c_str()));
    TIterator *it = cat_list->MakeIterator();
    for (RooDataSet *d = (RooDataSet*)it->Next();d!=0;d= (RooDataSet*)it->Next()){
      if(!m_containgg.at(j)){
	for (int i=0 ; i<d->numEntries() ; i++) {
	  const RooArgSet *row = d-> get(i) ;
	  RooArgSet *newrow = (RooArgSet*)row ;  
	  combcat->setLabel(d->GetName());
	  newrow->add(*combcat);
	  data->add(*newrow,d->weight());
	}
      }
      else {
	cout << "Dataset will be binned" << endl;
	RooProdPdf *pdf_ind = (RooProdPdf *)tmp_pdf->getPdf(d->GetName());
	if(pdf_ind==0){
	  cout << d->GetName() << " : pdf does not exist" << endl;
	}
	RooArgSet *observable =  (RooArgSet*)pdf_ind->getObservables(*d);
	TIterator* observable_itr = observable->createIterator();
	RooRealVar *obs =(RooRealVar *)  observable_itr->Next();
	TH1 *hist = d->createHistogram((string(d->GetName())+"hist").c_str(),*obs,Binning(500,obs->getMin(),obs->getMax()));
	RooBinning binning(500,obs->getMin(),obs->getMax());
	obs->setBinning(binning);
	for(int i =1; i <= hist->GetXaxis()->GetNbins();i++) {
	  obs->setVal(hist->GetXaxis()->GetBinCenter(i));
	  RooArgSet newrow;
	  combcat->setLabel(d->GetName());
	  newrow.add(*combcat);
	  newrow.add(*obs);
	  data->add(newrow,hist->GetBinContent(i));
	}
      }
    }
  }
  cout << "End Merged DataSet" << endl;
  cout << endl;
  m_data = data;
  cout << "Create Simultaneous Pdf" << endl;
  RooSimultaneous *combPdf = new RooSimultaneous("combPdf","combPdf",*combcat);
  for(int j=0; j < (int) m_pdf_name.size(); j++) {
    RooSimultaneous *tmp_pdf =(RooSimultaneous*)m_temp_w->pdf((m_pdf_name.at(j)+"_"+m_channels_name.at(j)).c_str());
    if(tmp_pdf==0) {
      cout << (m_pdf_name.at(j)+"_"+m_channels_name.at(j)).c_str() << " : pdf not in the temporary worksapace" << endl;
      return;
    }
    RooCategory *tmp_cat = m_temp_w->cat(m_cat_name.at(j).c_str());
    if(tmp_cat==0) {
      cout << m_cat_name.at(j).c_str() << " : category not in the temporary worksapace" << endl;
      return;
    }
    for(int i=0;i<tmp_cat->numTypes();i++){		
      tmp_cat->setIndex(i);
      RooProdPdf *pdf_ind = (RooProdPdf *)tmp_pdf->getPdf(tmp_cat->getLabel());
      RooArgSet *np = new RooArgSet();
      GetComponents(pdf_ind,np);      
      RooProdPdf *npdf = new RooProdPdf(pdf_ind->GetName(),pdf_ind->GetName(),*np);
      combPdf->addPdf(*npdf,tmp_cat->getLabel());  
    }
  }
  m_combPdf = combPdf;
  //  m_newmc->SetPdf(*m_combPdf);
  cout << "End Simultaneous Pdf" << endl;
  cout << endl;
}

void Combine::CreateFinalWorkspace(){
	
  cout << "Creation Final Workspace : " << endl;
  RooWorkspace* new_w = new RooWorkspace(m_combined_workspace_name.c_str(),m_combined_workspace_name.c_str());
  cout << "Import in the new workspace" << endl;
  CloseCoutSentry sentry1(1);
  new_w->import(*m_combPdf,RecycleConflictNodes(),Silence());
  new_w->import(*m_data);
  CloseCoutSentry sentry(0);
  cout << "Create combined Model Config : " << endl;
  ModelConfig * newmc = new ModelConfig(m_combined_ModelConfig_name.c_str(),new_w);
  newmc->SetPdf(*m_combPdf);
  RooArgSet newpoi,newnp,newgobs,newobs;
  RooArgSet obs = *m_newmc->GetObservables();
  RooArgSet gobs = *m_newmc->GetGlobalObservables();
  RooArgSet poi = *m_newmc->GetParametersOfInterest();
  RooArgSet np = *m_newmc->GetNuisanceParameters();
    
  TIterator *np_itr = np.createIterator();
  for ( RooRealVar* v = (RooRealVar*)np_itr->Next(); v!=0; v = (RooRealVar*)np_itr->Next() ){
    if(new_w->var(v->GetName())!=0){
      newnp.add(*new_w->var(v->GetName()));
    }
    else{
      cout << v->GetName() << " not in the workspace" << endl;
    }
  }
  newmc->SetNuisanceParameters(newnp);
  cout << "np done" << endl;
  
  for (int i =0; i < (int) m_combined_pois_name.size();i++) {
    if( new_w->var(m_combined_pois_name.at(i).c_str())!=0) {
      RooRealVar *var_temp = new_w->var(m_combined_pois_name.at(i).c_str());
      if(var_temp==0){
	cout << var_temp->GetName() << " not in the workspace" << endl;
	continue;
      }
      TString min_str(m_min_pois.at(i).c_str());
      TString max_str(m_max_pois.at(i).c_str());
      var_temp->setMin(min_str.Atof());
      var_temp->setMax(max_str.Atof());
      if(min_str.Atof()==max_str.Atof()){
	var_temp->setConstant(1);
      }
    }
  }
  TIterator *poi_itr = poi.createIterator();
  for ( RooRealVar* v = (RooRealVar*)poi_itr->Next(); v!=0; v = (RooRealVar*)poi_itr->Next() ){
    if(new_w->var(v->GetName())!=0){
      newpoi.add(*new_w->var(v->GetName()));
    }
    else{
      cout << v->GetName() << " not in the workspace" << endl;
    }
  }  
  newmc->SetParametersOfInterest(newpoi);
  cout << "poi done" << endl;
  TIterator *gobs_itr = gobs.createIterator();
  for ( RooRealVar* v = (RooRealVar*)gobs_itr->Next(); v!=0; v = (RooRealVar*)gobs_itr->Next() ){
    if(new_w->var(v->GetName())!=0){
      newgobs.add(*new_w->var(v->GetName()));
    }
    else{
      cout << v->GetName() << " not in the workspace" << endl;
    }
  }
  newmc->SetGlobalObservables(newgobs);
  cout << "gobs done" << endl;
  
  TIterator *obs_itr = obs.createIterator();
  for ( RooRealVar* v = (RooRealVar*)obs_itr->Next(); v!=0; v = (RooRealVar*)obs_itr->Next() ){
    if(new_w->var(v->GetName())!=0){
      newobs.add(*new_w->var(v->GetName()));
    }
    else{
      cout << v->GetName() << " not in the workspace" << endl;
    }
  }
  cout << "obs done" << endl;
  obs_itr->Reset();
  for ( RooCategory* v = ( RooCategory*)obs_itr->Next(); v!=0; v = ( RooCategory*)obs_itr->Next() ){
    if(new_w->cat(v->GetName())!=0){
      newobs.add(*new_w->cat(v->GetName()));
    }
    else{
      cout << v->GetName() << " not in the workspace" << endl;
    }
  }  
  newmc->SetObservables(newobs);
  cout << "cat done" << endl;
  cout << "End Creation" << endl;
  new_w->import(*newmc);
  cout << "End Final Workspace " << endl;
  cout << endl;
  cout << "Writing to file: " << m_combined_file_name << endl;
  new_w->writeToFile(m_combined_file_name.c_str(), true);
  cout << "Writing done to file: " << m_combined_file_name << endl;
}
void Combine::GetComponents(RooProdPdf *prodpdf, RooArgSet *np){
  const RooArgList list= prodpdf->pdfList();
  TIterator *it_list = list.createIterator();
  for ( RooAbsPdf* v = (RooAbsPdf*)it_list->Next(); v!=0; v = (RooAbsPdf*)it_list->Next() ){
    if( string(v->ClassName())!="RooProdPdf") {
      np->add(*v);
      //cout << v->GetName() << endl; 
    }
    else{
      RooProdPdf *v_temp = (RooProdPdf*)m_temp_w->pdf(v->GetName()); 
      //   cout << v->GetName() << endl;
      GetComponents(v_temp,np);
    }
  }
}

//################################################
int Combine::SplitPOI() {

  //Create a vector with the name of all final poi
  vector< RooRealVar* > combinedVariables;
  for ( unsigned int var=0; var < m_combined_pois_name.size(); var++ ) {
    combinedVariables.push_back( 0 );
    combinedVariables.back() = new RooRealVar( m_combined_pois_name[var].c_str(), m_combined_pois_name[var].c_str(), 1, 1, 1);
  }



  for ( unsigned int channel = 0; channel < m_files_name.size() ; channel++ ) {

    //Gather info from channel workspace
    TFile *channelFile = new TFile( m_files_name[channel].c_str() );
    if ( !channelFile ) {
      cout << "File " << m_files_name[channel] << " doest not exist" << endl;
      return 1;
    }
    RooWorkspace *channelWS = (RooWorkspace*) channelFile->Get( m_workspaces_name[channel].c_str() );
    if ( !channelWS ) {
      cout << "Worspace " << m_workspaces_name[channel] << " does not exist in " << m_files_name[channel] << endl;
      return 2;
    }
    ModelConfig *mc = (ModelConfig*) channelWS->obj( m_ModelConfigs_name[ channel ].c_str() );
    for ( unsigned int var=0; var < m_combined_pois_name.size(); var++ ) {
      channelWS->import( *combinedVariables[var], RooFit::RecycleConflictNodes() );
    }


    //Start creating the editing line
    stringstream editStr; 
    editStr << "EDIT::" << mc->GetPdf()->GetName() << "_CombConvention(" << mc->GetPdf()->GetName();


    for ( unsigned int poi = 0; poi < m_pois_name[channel].size(); poi++ ) {

      //Splite teh pois string into individual names
      vector< string > splittedVariables, mergedVariables;
      SplitVarNames( m_pois_name[channel][poi] , splittedVariables );      
      MergedVarNames( m_pois_name[channel][poi] , mergedVariables );      
      if ( splittedVariables.size()==1 && splittedVariables.front()=="dummy" ) continue;



      if ( mergedVariables.size() > 1 ) {

	for ( unsigned int var = 0; var < mergedVariables.size(); var++ ) {
	  editStr << "," << mergedVariables[var] << "=" << m_combined_pois_name[ poi ];
	}

	m_pois_name[channel][poi] = m_combined_pois_name[ poi ]; 
      }


      if ( splittedVariables.size() > 1 ) {
      string unknownVar = "";
      for ( unsigned int var = 0; var < splittedVariables.size(); var++ ) {

	//Check if the current variable belong to the combined variables
	int indexVar = FindVariable( splittedVariables[var], m_combined_pois_name );

	if ( indexVar == -1 ) {
	  if ( unknownVar == "" ) {
 	    unknownVar = splittedVariables[var];
	    m_pois_name[ channel ][ poi ] = m_combined_pois_name[ poi ];
	    //If the variable is to be renamed, start an editing line, including a product of names (in case of splitting)
	    editStr << "," << unknownVar << "=prod::" << unknownVar << "Split(" << m_combined_pois_name[ poi ];

	    //Cath up for the product with the variables that already existed
	    //Should not be used if the renamed variable is in first position in the xml file
	    for ( unsigned int i = 0; i < var; i++ ) {
	      editStr << "," << splittedVariables[i];
	    }
	  }//End if unknown=""

	  //Error message in case several variables do not belong to combined variables
	  else {
	    cout << "Error : Several unknown var in line " << m_pois_name[channel][poi] << endl;
	    return 3;
	  }}//End indexVar==-1


	else {
	  if ( m_pois_name[ channel ][ indexVar ] == "dummy" ) m_pois_name[ channel ][ indexVar ] = splittedVariables[ var ];
	  //Add the variable to the product
	  // The condition means that the renamed variable have already been identified so the start of the editing line is already done
	  if ( unknownVar != "" ) editStr << "," << splittedVariables[var];
	  }

      } //End loop var
      //Close the product in editing line if needed
      if ( unknownVar != "" ) editStr << ")";
      }//End splittedVar



    }//End loop poi

    //Close the editing line
    editStr << ")";

    cout << editStr.str() << endl;
    // channelWS->factory(editStr.str().c_str());

    // //Create an intermediate workspace to put the new pdf and data
    // RooWorkspace *tempWS = new RooWorkspace(  TString(channelWS->GetName()) + "_temp" , TString( channelWS->GetName()) + "_temp" );
    // tempWS->import( *channelWS->pdf( TString( mc->GetPdf()->GetName()) + "_CombConvention" ) );
    // tempWS->import( *channelWS->data( m_datas_name[ channel ].c_str() ) );
    // tempWS->writeToFile(  TString(m_files_name[ channel ].c_str()) + "_CombConvention.root" );

    // //Change the information for the following program to find the new input workspace
    // m_workspaces_name[ channel ] = tempWS->GetName();
    // m_files_name[ channel ] = TString(m_files_name[ channel ].c_str()) + "_CombConvention.root";
  }//End loop on cahnnels

  return 0;
}
