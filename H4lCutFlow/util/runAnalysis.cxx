////////////////////
// To Run the analysis
//
//
//
//

// C++ includes
#include <stdlib.h>
#include <iostream>
#include <vector>

// Local includes
#include "H4lCutFlow/Analysis4l.h"
#include "H4lCutFlow/AnalysisFiducial.h"
#include "H4lCutFlow/Analysisbkg.h"
#include "HZZCutCommon/AnalysisBase.h"

// Root includes
#include "TSystemFile.h"
#include "TSystemDirectory.h"


using namespace std;

// Functions
bool cmdline(int argc, char** argv, map<TString,TString>& opts);
void initializeCutFlow();
void setSampleType();
void setAnalysisType();
void runAnalysis();


// get the list of files in the directory
std::vector<std::string> getFileList(TString dirName, TString pattern);

// Variables
AnalysisBase* cutflow = 0;
//TString dirPath = "/Users/Haider/ATLAS/sw/data/";
//TString dirPath = "/afs/cern.ch/work/s/sabidi/public/data/";
//TString dirPath = "/afs/cern.ch/work/b/bhaney/public/theory_truth4/";
TString dirPath = "/afs/cern.ch/work/m/mmittal/public/test";
//TString dirPath = "/afs/cern.ch/work/m/mmckay/public/test";
map<TString,TString> opts;

int main(int argc, char** argv)
{

    if (!cmdline(argc,argv,opts)) return 0;

    initializeCutFlow();
    setSampleType();

    setAnalysisType();

    runAnalysis();

    // If we weren't printing truth
    // we can print the cutflow
    if(opts["printTruth"].Length() < 1) cutflow->printCutFlow();

    //cutflow->printTruth(58);

    delete cutflow;
}


bool cmdline(int argc, char** argv, map<TString,TString>& opts)
{
    opts.clear();

    // defaults
    opts["sampleType"]      = "mc15c_VBF125";
    opts["analType"]        = "reco";
    opts["systematic"]      = "";
    opts["debug"]           = "";
    opts["printTruth"]      = "";
    opts["nEvents"]         = "";
    opts["prodVersion"]     = "";
    opts["doTrigStudy"]     = "false";
    opts["addTheoryVar"]    = "false";
    opts["truthLeptonType"] = "dressed";
    opts["doMassWindowCut"] = "true";
    opts["fullEgammaSys"]   = "false";
    opts["reducedSys"]      = "false";
    opts["comment"]         = "";

    for (int i=1;i<argc;i++) {

        string opt=argv[i];

        if (opt=="--help") {
            cout<<"-sampleType      : sample Type"<<endl;
            cout<<"-analType        : analysis type (reco, bkg, fid)"<<endl;
            cout<<"-systematic      : List of systematics to run over, wildcarded by '.' (a dot)"<<endl;
            cout<<"-nEvents         : number of events to run over - for testing"<<endl;
            cout<<"-debug           : event number for debugging (delimted by comma)"<<endl;
            cout<<"-doTrigStudy     : (false, true) save trigger variables to minitree to do efficiency studies."<<endl;
            cout<<"-addTheoryVar    : (false, true) save ScalePDF variables to minitree to do theory systematics."<<endl;
            cout<<"-truthLeptonType : (false, true) choices are bare, born, or dressed. Default is dressed."<<endl;
            cout<<"-doMassWindowCut : (false, true) Only for fiducial analysis. Apply the mass window cut in cutflow. Default is true."<<endl;
            cout<<"-reducedSys      : (false, true) Only useful if systematics are being run. It set to true, only the reduced systematic set will be run. Default is false"<<endl;
            cout<<"-fullEgammaSys   : (false, true) Do the full egamma systematics. Default is false"<<endl;
            cout<<"-printTruth      : event number for printing Truth (delimted by comma)"<<endl;
            return false;
        }

        if(0!=opt.find("-")) {
            cout<<"ERROR: options start with '-'!"<<endl;
            return false;
        }
        opt.erase(0,1);
        if(opts.find(opt)==opts.end()) {
            cout<<"ERROR: invalid option '"<<opt<<"'!"<<endl;
            return false;
        }
        string nxtopt=argv[i+1];
        if(0==nxtopt.find("-")||i+1>=argc) {
            cout<<"ERROR: option '"<<opt<<"' requires value!"<<endl;
            return false;
        }

        opts[opt]=nxtopt;
        i++;
    }

    return true;
}

void initializeCutFlow()
{
    TString analType = opts["analType"];
    if(analType.Contains("reco"))
    {
        cutflow = new Analysis4l();
    }
    else if(analType.Contains("bkg"))
    {
        cutflow = new Analysisbkg();
    }
    else if(analType.Contains("fid"))
    {
        cutflow = new AnalysisFiducial();
    }
    else
    {
        cout<<"analtype option not recongnized"<<endl;
        cout<<"input option: "<<analType<<endl;
    }

    cutflow->setMinitreeVersion(opts["prodVersion"]);
    cutflow->setComment(opts["comment"]);
    cutflow->setCommandOpts(opts);

}


void runAnalysis()
{
    // If Debug analysis
    if(opts["debug"].Length() != 0)
    {
        // Splitting the file path
        TObjArray *parts = opts["debug"].Tokenize(",");
        vector<TString> partName;
        if(parts->GetEntriesFast()) {
            TIter iString(parts);
            TObjString* os=0;
            while ((os=(TObjString*)iString())) {
                partName.push_back(os->GetString());
            }
        }

        for(Int_t i = 0; i < (Int_t) partName.size(); i++)
        {
            if(!partName[i].IsDec())
            {
                cout<<"eventNumber ("<<partName[i]<<") is not a digit"<<endl;
                exit(1);
            }
            cutflow->processDebug(partName[i].Atoll());
        }
    }
    // If print truth
    else if(opts["printTruth"].Length() != 0)
    {
        // Splitting the file path
        TObjArray *parts = opts["printTruth"].Tokenize(",");
        vector<TString> partName;
        if(parts->GetEntriesFast()) {
            TIter iString(parts);
            TObjString* os=0;
            while ((os=(TObjString*)iString())) {
                partName.push_back(os->GetString());
            }
        }

        for(Int_t i = 0; i < (Int_t) partName.size(); i++)
        {
            if(!partName[i].IsDec())
            {
                cout<<"eventNumber ("<<partName[i]<<") is not a digit"<<endl;
                exit(1);
            }
            cutflow->printTruth(partName[i].Atoi());
        }
    }
    // Nothing special just run the code
    else
    {
        cutflow->process();
    }

}

void setAnalysisType()
{
    // Setting the Analysis Type
    cutflow->setAnalysisType(AnalysisType::Nominal);
    if(opts["analType"].Length() != 0)
    {
        TString analType = opts["analType"];
        if(analType.Contains("fid"))
        {
            cutflow->setAnalysisType(AnalysisType::Fiducial);
        }
    }

    // Setting the systematics
    vector<TString> sysList;
    if(opts["systematic"].Length() != 0)
    {
        // Splitting the file path
        TObjArray *parts = opts["systematic"].Tokenize(",");
        if(parts->GetEntriesFast()) {
            TIter iString(parts);
            TObjString* os=0;
            while ((os=(TObjString*)iString())) {
                sysList.push_back(os->GetString());
            }
        }
        cutflow->doSystematic(true, sysList);
    }
    else
    {
        cutflow->doSystematic(false, sysList);
    }


    // Number of events to run over
    if(opts["nEvents"].Length() != 0)
    {
        cutflow->setnEventTest(opts["nEvents"].Atoi());
    }
}


void setSampleType()
{
    vector<string> inputFilenames;

    //////////////////////////////////////
    // DxAOD samples
    //////////////////////////////////////
    if(opts["sampleType"].EqualTo("mc15c_qq2ZZSherpa"))
    {
        cutflow->setFileName("mc15_13TeV.363490.Sherpa_221_NNPDF30NNLO_llll.merge.DAOD_HIGG2D1.e5332_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15_qq2ZZSherpa");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.363490.Sherpa_221_NNPDF30NNLO_llll.merge.DAOD_HIGG2D1.e5332_s2726_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.10169623._000011.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_qq2ZZSherpa2222"))
    {
        cutflow->setFileName("mc15_13TeV.364250.Sherpa_222_NNPDF30NNLO_llll.merge.DAOD_TRUTH4.e5894_p3075_tid11117648_00");
        cutflow->setOutName("mc15_qq2ZZSherpa222");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.364250.Sherpa_222_NNPDF30NNLO_llll.merge.DAOD_TRUTH4.e5894_p3075_tid11117648_00/DAOD_TRUTH4.11117648._000077.pool.root.1"
        };
    }

   
     else if(opts["sampleType"].EqualTo("mc15c_ZZllllTruth4"))
    {
        cutflow->setFileName("group.phys-higgs.361603.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4.TRUTH4.e4475.20160409.v01_EXT0");
        cutflow->setOutName("mc15_ZZllllTruth5");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/s/sabidi/public/data/group.phys-higgs.361603.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4.TRUTH4.e4475.20160409.v01_EXT0/group.phys-higgs.11144269.EXT0._000118.DAOD_TRUTH4.out.root"
        };
    }
    
    else if(opts["sampleType"].EqualTo("mc15c_ZZllll"))
    {
        cutflow->setFileName("mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.merge.DAOD_HIGG2D1.e4366_s2608_r7772_r7676_p2879/");
        cutflow->setOutName("mc15_ZZllll");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.merge.DAOD_HIGG2D1.e4366_s2608_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09956420._000001.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e3951_s2608_s2183_r7772_r7676_p2879");
        cutflow->setOutName("mc15_ggH125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e3951_s2608_s2183_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09961447._000001.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ggH125NNLOPS"))
    {
        cutflow->setFileName("mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_HIGG2D2.e5682_s2726_r7772_r7676_p2987");
        cutflow->setOutName("mc15_ggH125NNLOPS");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_HIGG2D2.e5682_s2726_r7772_r7676_p2987/mc15_13TeV/DAOD_HIGG2D2.10644886._000028.pool.root.1"
        };
    }   
    else if(opts["sampleType"].EqualTo("mc15c_VBF125"))
    {
        cutflow->setFileName("mc15_13TeV.341518.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e3951_s2608_s2183_r7772_r7676_p2666");
        cutflow->setOutName("mc15_VBF125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.341518.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e3951_s2608_s2183_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09953988._000012.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_VBF125NNPDF"))
    {
        cutflow->setFileName("mc15_13TeV.344235.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125_ZZ4lep_notau.merge.DAOD_HIGG2D1.e5500_s2726_r7772_r7676_p2987/");
        cutflow->setOutName("mc15_VBF125NNPDF");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.344235.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125_ZZ4lep_notau.merge.DAOD_HIGG2D1.e5500_s2726_r7772_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10557762._000021.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_WH125"))
    {
        cutflow->setFileName("mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.merge.DAOD_HIGG2D1.e4161_s2608_s2183_r7772_r7676_p2879");
        cutflow->setOutName("mc15_WH125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.merge.DAOD_HIGG2D1.e4161_s2608_s2183_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09953655._000001.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_WpH125Minlo"))
    {
        cutflow->setFileName("mc15_13TeV.345039.PowhegPythia8EvtGen_NNPDF30_AZNLO_WpH125J_Wincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987");
        cutflow->setOutName("mc15_WpH125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.345039.PowhegPythia8EvtGen_NNPDF30_AZNLO_WpH125J_Wincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10552973._000011.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_WmH125Minlo"))
    {
        cutflow->setFileName("mc15_13TeV.345040.PowhegPythia8EvtGen_NNPDF30_AZNLO_WmH125J_Wincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987");
        cutflow->setOutName("mc15_WmH125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.345040.PowhegPythia8EvtGen_NNPDF30_AZNLO_WmH125J_Wincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10552968._000011.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ZH125Minlo"))
    {
        cutflow->setFileName("mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987");
        cutflow->setOutName("mc15_ZH125Minlo");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10603983._000018.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ZH125"))
    {
        cutflow->setFileName("mc15_13TeV.341947.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZ4l.merge.DAOD_HIGG2D1.e4161_s2608_s2183_r7772_r7676_p2879");
        cutflow->setOutName("mc15_ZH125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.341947.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZ4l.merge.DAOD_HIGG2D1.e4161_s2608_s2183_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09955991._000001.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ttH125"))
    {
        cutflow->setFileName("mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.merge.DAOD_HIGG2D1.e4540_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15_ttH125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.merge.DAOD_HIGG2D1.e4540_s2726_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.10196908._000002.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_tWH125"))
    {
        cutflow->setFileName("mc15_13TeV.342001.aMcAtNloHppEG_UEEE5_CTEQ6L1_CT10ME_tWH125_lep_yt_plus1.merge.DAOD_HIGG2D2.e4394_s2608_r7772_r7676_p2879");
        cutflow->setOutName("mc15_tWH125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.342001.aMcAtNloHppEG_UEEE5_CTEQ6L1_CT10ME_tWH125_lep_yt_plus1.merge.DAOD_HIGG2D2.e4394_s2608_r7772_r7676_p2879/DAOD_HIGG2D2.11123972._000011.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_tHjb125"))
    {
        cutflow->setFileName("mc15_13TeV.343273.MadGraphPythia8EvtGen_A14_CT10ME_tHjb125_lep.merge.DAOD_HIGG2D2.e4606_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15_tHjb125");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.343273.MadGraphPythia8EvtGen_A14_CT10ME_tHjb125_lep.merge.DAOD_HIGG2D2.e4606_s2726_r7772_r7676_p2879/DAOD_HIGG2D2.11123974._000007.pool.root.1"
        };
    }   
    else if(opts["sampleType"].EqualTo("mc15c_bbH125_ybyt"))
    {
        cutflow->setFileName("mc15_13TeV.344974.aMcAtNloPythia8EvtGen_A14NNPDF23LO_bbH125_ybyt_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e5537_a766_a821_r7676_p2879");
        cutflow->setOutName("mc15_bbH125_ybyt");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.344974.aMcAtNloPythia8EvtGen_A14NNPDF23LO_bbH125_ybyt_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e5537_a766_a821_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10557768._000001.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_Zee"))
    {
        cutflow->setFileName("mc15_13TeV.344295.Sherpa_NNPDF30NNLO_Zee_4lMassFilter40GeV8GeV.merge.DAOD_HIGG2D1.e5169_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15_Zee");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.344295.Sherpa_NNPDF30NNLO_Zee_4lMassFilter40GeV8GeV.merge.DAOD_HIGG2D1.e5169_s2726_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09955153._000011.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_Zmumu"))
    {
        cutflow->setFileName("mc15_13TeV.344296.Sherpa_NNPDF30NNLO_Zmumu_4lMassFilter40GeV8GeV.merge.DAOD_HIGG2D1.e5169_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15_Zmumu");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.344296.Sherpa_NNPDF30NNLO_Zmumu_4lMassFilter40GeV8GeV.merge.DAOD_HIGG2D1.e5169_s2726_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09962344._000011.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_bbH125_yb2"))
    {
        cutflow->setFileName("mc15_13TeV.344973.aMcAtNloPythia8EvtGen_A14NNPDF23LO_bbH125_yb2_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e5537_a766_a821_r7676_p2987");
        cutflow->setOutName("mc15_bbH125_yb2");
        inputFilenames = std::vector<std::string> {
            "/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.344973.aMcAtNloPythia8EvtGen_A14NNPDF23LO_bbH125_yb2_ZZ4lep_noTau.merge.DAOD_HIGG2D1.e5537_a766_a821_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10557749._000009.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ttll"))
    {
        cutflow->setFileName("mc15_13TeV.410142.Sherpa_NNPDF30NNLO_ttll_mll5.merge.DAOD_HIGG2D1.e4686_a766_a818_r7676_p2879");
        cutflow->setOutName("mc15_ttll");
        inputFilenames = std::vector<std::string> {
        "/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.410142.Sherpa_NNPDF30NNLO_ttll_mll5.merge.DAOD_HIGG2D1.e4686_a766_a818_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.09956531._000001.pool.root.1"
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ZZZ"))
    {
        cutflow->setFileName("mc15_13TeV.361626.Sherpa_CT10_ZZZ_4l2v.merge.DAOD_HIGG2D1.e4093_s2608_s2183_r7772_r7676_p2666/");
        cutflow->setOutName("mc15_ZZZ");

        TString temp = "/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.361626.Sherpa_CT10_ZZZ_4l2v.merge.DAOD_HIGG2D1.e4093_s2608_s2183_r7772_r7676_p2666/DAOD_HIGG2D1.08565414._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }

     
    else if(opts["sampleType"].EqualTo("mc15c_llll_m100"))
    {
        cutflow->setFileName("mc15_13TeV.345107.Sherpa_221_NNPDF30NNLO_llll_m4l100_300_filt100_150.merge.DAOD_HIGG2D1.e5630_s2726_r7772_r7676_p2987");
        cutflow->setOutName("mc15_llll_m100");

        TString temp = "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.345107.Sherpa_221_NNPDF30NNLO_llll_m4l100_300_filt100_150.merge.DAOD_HIGG2D1.e5630_s2726_r7772_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10648518._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_llll_m300"))
    {
        cutflow->setFileName("mc15_13TeV.345108.Sherpa_221_NNPDF30NNLO_llll_m4l300.merge.DAOD_HIGG2D1.e5630_s2726_r7772_r7676_p2987");
        cutflow->setOutName("mc15_llll_m300");

	TString temp = "/afs/cern.ch/work/a/anlaudra/public/MC/MC15c/mc15_13TeV.345108.Sherpa_221_NNPDF30NNLO_llll_m4l300.merge.DAOD_HIGG2D1.e5630_s2726_r7772_r7676_p2987/mc15_13TeV/DAOD_HIGG2D1.10648503._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_tWH125"))
    {
        cutflow->setFileName("mc15_13TeV.342001.aMcAtNloHppEG_UEEE5_CTEQ6L1_CT10ME_tWH125_lep_yt_plus1.merge.DAOD_HIGG2D2.e4394_s2608_r7772_r7676_p2879");
        cutflow->setOutName("mc15_tWH125");

	TString temp = "/afs/cern.ch/work/s/schaffer/public/data/mc15_13TeV.342001.aMcAtNloHppEG_UEEE5_CTEQ6L1_CT10ME_tWH125_lep_yt_plus1.merge.DAOD_HIGG2D2.e4394_s2608_r7772_r7676_p2879/mc15_13TeV/DAOD_HIGG2D2.11123972._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("truth4test2"))
    {
	cutflow->setFileName("group.phys-higgs.345038.PhPy8EG_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.variation.2070122.v03_DAOD_TRUTH4.AZNLO_EV2Down.root");
        cutflow->setOutName("mc15_truth4test");

	TString temp = "/afs/cern.ch/work/m/mmckay/public/test/group.phys-higgs.345038.PhPy8EG_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.variation.2070122.v03_DAOD_TRUTH4.AZNLO_EV2Down.root/group.phys-higgs.10538749._004804.DAOD_TRUTH4.AZNLO_EV2Down.root";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("truth4test"))
    {
	cutflow->setFileName("group.phys-higgs.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.variation.20170218.v03_DAOD_TRUTH4.AZNLO_EV1Down.root");
	//cutflow->setOutName("group.phys-higgs.10538427._000001.DAOD_TRUTH4.AZNLO_EV1Down.root");
    cutflow->setOutName("mc15_truth4test1");
	TString temp = "/afs/cern.ch/work/m/mmckay/public/test/group.phys-higgs.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.variation.20170218.v03_DAOD_TRUTH4.AZNLO_EV1Down.root/group.phys-higgs.10748055._000123.DAOD_TRUTH4.AZNLO_EV1Down.root";  
        inputFilenames.push_back(temp.Data());
    }
    //////////////////////////////////////
    // AOD samples
    //////////////////////////////////////
    else if(opts["sampleType"].EqualTo("mc15c_AOD_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.merge.AOD.e3951_s2608_s2183_r7772_r7676");
        cutflow->setOutName("mc15_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.merge.AOD.e3951_s2608_s2183_r7772_r7676/AOD.08216950._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc16_AOD_ggH125"))
    {
        cutflow->setFileName("mc16_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.merge.AOD.e3951_s2997_r8957_r8996");
        cutflow->setOutName("mc16_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2017/mc16_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.merge.AOD.e3951_s2997_r8957_r8996/AOD.10340771._000007.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_AOD_VBFH125"))
    {
        cutflow->setFileName("mc15_13TeV.341518.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_ZZ4lep_noTau.merge.AOD.e3951_s2608_s2183_r7772_r7676");
        cutflow->setOutName("mc15_VBFH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.341518.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_ZZ4lep_noTau.merge.AOD.e3951_s2608_s2183_r7772_r7676/AOD.08217462._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_AOD_ttH125"))
    {
        cutflow->setFileName("mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.merge.AOD.e4540_s2726_r7772_r7676");
        cutflow->setOutName("mc15_ttH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.merge.AOD.e4540_s2726_r7772_r7676/AOD.08226523._000010.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_AOD_WH125"))
    {
        cutflow->setFileName("mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.merge.AOD.e4161_s2608_s2183_r7772_r7676");
        cutflow->setOutName("mc15_WH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.merge.AOD.e4161_s2608_s2183_r7772_r7676/AOD.08217332._000007.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_AOD_ZH125"))
    {
        cutflow->setFileName("mc15_13TeV.341947.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZ4l.merge.AOD.e4161_s2608_s2183_r7772_r7676");
        cutflow->setOutName("mc15_ZH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.341947.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZ4l.merge.AOD.e4161_s2608_s2183_r7772_r7676/AOD.08217327._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("arthur_MG5_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.344167.aMcAtNloPythia8EvtGen_A14NNPDF23LO_FxFx_kAgg0p5_cosa0p707.merge.AOD.e5195_s2726_r7772_r7676");
        cutflow->setOutName("MG5_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/arthur/AOD.08945364._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("arthur_MG5_VBF125"))
    {
        cutflow->setFileName("mc15_13TeV.343249.MadGraphPythia8EvtGen_A14NNPDF23LO_vbfhzz4l_0phkHv10.merge.AOD.e4682_s2726_r7772_r7676");
        cutflow->setOutName("MG5_VBF125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/arthur/AOD.08346176._000004.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("andrea_MG5_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.344158.aMcAtNloPythia8EvtGen_A14NNPDF23LO_ppx0_FxFx_Np012_SM");
        cutflow->setOutName("MG5_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/truth/MadGraph/mc15_13TeV.123456.ggH125.DAOD.TRUTH4.MG5.SM.root";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_AOD_ZH125J"))
    {
        cutflow->setFileName("mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.AOD.e5590_s2726_r7772_r7676");
        cutflow->setOutName("AOD_ZH125J");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2017/mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.AOD.e5590_s2726_r7772_r7676/AOD.10247618._000005.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_ZH125J"))
    {
        cutflow->setFileName("mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987");
        cutflow->setOutName("AOD_ZH125J");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2017/mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.DAOD_HIGG2D1.e5590_s2726_r7772_r7676_p2987/DAOD_HIGG2D1.10603983._000006.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_truth4_ZH125J"))
    {
        cutflow->setFileName("mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.DAOD_TRUTH4.e5590_p2913");
        cutflow->setOutName("TRUTH4_ZH125J");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/truth4/mc15_13TeV.345038.PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO.merge.DAOD_TRUTH4.e5590_p2913/DAOD_TRUTH4.10550111._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_ggHNNLOPS"))
    {
        cutflow->setFileName("mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_HIGG2D1.e5682_s2726_r7772_r7676_p2987");
        cutflow->setOutName("DAOD_HIGG2D1_ggHNNLOPS");
        TString temp = "/home/bijanh/datadir/signal2017/mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_HIGG2D1.e5682_s2726_r7772_r7676_p2987/DAOD_HIGG2D1.10644889._000002.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_truth4_ggHNNLOPS"))
    {
        cutflow->setFileName("mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_TRUTH4.e5682_p2913");
        cutflow->setOutName("TRUTH4_ggHNNLOPS");
        TString temp = "/home/bijanh/datadir/truth4/mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_TRUTH4.e5682_p2913/DAOD_TRUTH4.10550099._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_AOD_ggHNNLOPS"))
    {
        cutflow->setFileName("mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.AOD.e5682_s2726_r7772_r7676");
        cutflow->setOutName("AOD_ggHNNLOPS");
        TString temp = "/home/bijanh/datadir/signal2017/mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.AOD.e5682_s2726_r7772_r7676/AOD.10372468._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_EVGEN_ggHNNLOPS"))
    {
        cutflow->setFileName("mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.evgen.EVNT.e5682");
        cutflow->setOutName("EVGEN_ggHNNLOPS");
        TString temp = "/home/bijanh/datadir/signal2017/mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.evgen.EVNT.e5682/EVNT.10371570._000003.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_EFT_kHvv6"))
    {
        cutflow->setFileName("mc15_13TeV.344160.aMcAtNloPythia8EvtGen_A14NNPDF23LO_FxFx_kHvv6_cosa1p0.merge.AOD.e5195_s2726_r7772_r7676");
        cutflow->setOutName("mc15_EFT_FxFx_kHvv6");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.344160.aMcAtNloPythia8EvtGen_A14NNPDF23LO_FxFx_kHvv6_cosa1p0.merge.AOD.e5195_s2726_r7772_r7676/AOD.08937143._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_EFT_kAvv6"))
    {
        cutflow->setFileName("mc15_13TeV.344163.aMcAtNloPythia8EvtGen_A14NNPDF23LO_FxFx_kAvv6_cosa0p707.merge.AOD.e5195_s2726_r7772_r7676");
        cutflow->setOutName("mc15_EFT_FxFx_kAvv6");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.344163.aMcAtNloPythia8EvtGen_A14NNPDF23LO_FxFx_kAvv6_cosa0p707.merge.AOD.e5195_s2726_r7772_r7676/AOD.08943046._000005.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_EFT_ggHSM"))
    {
        cutflow->setFileName("mc15_13TeV.344158.aMcAtNloPythia8EvtGen_A14NNPDF23LO_ppx0_FxFx_Np012_SM.merge.AOD.e5403_s2726_r7772_r7676");
        cutflow->setOutName("mc15_EFT_FxFx_SM");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/mc15_13TeV.344158.aMcAtNloPythia8EvtGen_A14NNPDF23LO_ppx0_FxFx_Np012_SM.merge.AOD.e5403_s2726_r7772_r7676/AOD.09596371._000012.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    //////////////////////////////////////
    // Truth samples
    //////////////////////////////////////
    else if(opts["sampleType"].EqualTo("mc15c_ggZZNLO"))
    {
        cutflow->setFileName("group.phys-higgs.mc15_13TeV.123456.Powheg_NNPDF30nlo_mZZ_ggZZ2e2mu.TRUTH4.20160102.v01_DAOD_TRUTH4.output.root");
        cutflow->setOutName("mc15_ggZZNLO");
        TString temp = "/afs/cern.ch/work/s/sabidi/public/data/group.phys-higgs.mc15_13TeV.123456.Powheg_NNPDF30nlo_mZZ_ggZZ2e2mu.TRUTH4.20160102.v01_DAOD_TRUTH4.output.root/group.phys-higgs.10390347._000007.DAOD_TRUTH4.output.root";
        inputFilenames.push_back(temp.Data());
    }  
    else if(opts["sampleType"].EqualTo("mc15c_gg2VV"))
    {
        cutflow->setFileName("group.phys-higgs.mc15_13TeV.343212.gg2vvPythia8EvtGen_gg_ZZ_bkg_2e2mu.TRUTH4.20160102.v01_EXT0");
        cutflow->setOutName("mc15_ggVV");
        TString temp = "/afs/cern.ch/work/s/sabidi/public/data/group.phys-higgs.mc15_13TeV.343212.gg2vvPythia8EvtGen_gg_ZZ_bkg_2e2mu.TRUTH4.20160102.v01_EXT0/group.phys-higgs.10390168.EXT0._000015.DAOD_TRUTH4.output.root";
        inputFilenames.push_back(temp.Data());
    }  
    else if(opts["sampleType"].EqualTo("mc15c_truth0_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0");
        cutflow->setOutName("mc15_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/truth/mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0/user.gabarone.8350657.EXT0._000021.DAOD_TRUTH0.truth.pool.root";
        inputFilenames.push_back(temp.Data());
        temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/truth/mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0/user.gabarone.8350657.EXT0._000022.DAOD_TRUTH0.truth.pool.root";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_truth_ttH125"))
    {
        cutflow->setFileName("mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.evgen.EVNT.e4540_prod09_EXT0");
        cutflow->setOutName("mc15_ttH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/truth/mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.evgen.EVNT.e4540_prod09_EXT0/user.gabarone.8350806.EXT0._000010.DAOD_TRUTH0.truth.pool.root";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("gaetano_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0");
        cutflow->setOutName("mc15_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/gaetano/user.gabarone.8350657.EXT0._000011.DAOD_TRUTH0.truth.pool.root";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("sarah_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0");
        cutflow->setOutName("mc15_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/sarah/DAOD_TRUTH0.truth0_EVNT.05551009._000001.pool.root";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_WH125_Truth"))
    {
        cutflow->setFileName("mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.merge.DAOD_TRUTH1.e5243_p2666");
        cutflow->setOutName("mc15_WH125_truth");
        TString temp = "/afs/cern.ch/work/a/angabrie/public/Haider/DAOD_TRUTH1.09176025._000001.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15c_truth0_ggH125"))
    {
        cutflow->setFileName("mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0");
        cutflow->setOutName("mc15_ggH125");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/truth/mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0/user.gabarone.8350657.EXT0._000021.DAOD_TRUTH0.truth.pool.root";
        inputFilenames.push_back(temp.Data());
        temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/signal2016/truth/mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.evgen.EVNT.e3951_prod09_EXT0/user.gabarone.8350657.EXT0._000022.DAOD_TRUTH0.truth.pool.root";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("mc15_qq2ZZ"))
    {
        cutflow->setFileName("mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.merge.DAOD_HIGG2D1.e4366_s2608_r7772_r7676_p2719");
        cutflow->setOutName("mc15_qq2ZZ");
        TString temp = "/disk/userdata00/atlas_data2/bijanh/HZZRun2/background/mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.merge.DAOD_HIGG2D1.e4366_s2608_r7772_r7676_p2719/DAOD_HIGG2D1.09041085._000003.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("TestqqZZ"))
    {
        cutflow->setFileName("mc15_13TeV.363490.Sherpa_221_NNPDF30NNLO_llll.merge.DAOD_HIGG2D1.e5332_s2726_r7772_r7676_p2879/DAOD_HIGG2D1.10169623._000011.pool.root.1");
        cutflow->setOutName("mc15_qqZZTest");

        TString temp = "/home/goblirsc/F/Datasets/H4l/DAODs/mc15_13TeV/DAOD_HIGG2D1.10169623._000011.pool.root.1";
        inputFilenames.push_back(temp.Data());
    }
    else if(opts["sampleType"].EqualTo("Testgg2VVggZZ"))
    {
        cutflow->setFileName("user.gabarone.mc15_13TeV.343212.Powheggg2vvPythia8EvtGen_gg_ZZ_bkg_2e2mu_13TeV.evgen.EVNT.e4568_prod09_EXT0/user.gabarone.8350823.EXT0._000005.DAOD_TRUTH0.truth.pool.root");
        cutflow->setOutName("mc15_gg2VVGGZZTest");

        inputFilenames.push_back("/home/goblirsc/F/Datasets/H4l/Truth0Plus/Prod09/user.gabarone.mc15_13TeV.343212.Powheggg2vvPythia8EvtGen_gg_ZZ_bkg_2e2mu_13TeV.evgen.EVNT.e4568_prod09_EXT0/user.gabarone.8350823.EXT0._000005.DAOD_TRUTH0.truth.pool.root");
        inputFilenames.push_back("/home/goblirsc/F/Datasets/H4l/Truth0Plus/Prod09/user.gabarone.mc15_13TeV.343213.Powheggg2vvPythia8EvtGen_gg_ZZ_bkg_4l_noTau_13TeV.evgen.EVNT.e4568_prod09_EXT0/user.gabarone.8350826.EXT0._000006.DAOD_TRUTH0.truth.pool.root");
    }
    else if(opts["sampleType"].EqualTo("TestMCFMggH"))
    {
        cutflow->setFileName("mc15_13TeV.344832.MCFMPythia8EvtGen_ggH_ZZ_4mu_m4l130.merge.DAOD_TRUTH4.e5518_p2913/DAOD_TRUTH4.10176333._000001.pool.root.1");
        cutflow->setOutName("mc15_MCFMPythia8EvtGen_ggH_ZZ_4mu_m4l130");

        inputFilenames.push_back("/home/goblirsc/F/Datasets/H4l/TRUTH4/mc15_13TeV.344832.MCFMPythia8EvtGen_ggH_ZZ_4mu_m4l130.merge.DAOD_TRUTH4.e5518_p2913/DAOD_TRUTH4.10176333._000001.pool.root.1");
    }
    else if(opts["sampleType"].EqualTo("TestMCFMggZZ"))
    {
        cutflow->setFileName("mc15_13TeV.344835.MCFMPythia8EvtGen_gg_ZZ_4mu_m4l100.merge.DAOD_TRUTH4.e5518_p2913/DAOD_TRUTH4.10176349._000001.pool.root.1");
        cutflow->setOutName("mc15_MCFMPythia8EvtGen_gg_ZZ_4mu_m4l100");

        inputFilenames.push_back("/home/goblirsc/F/Datasets/H4l/TRUTH4/mc15_13TeV.344835.MCFMPythia8EvtGen_gg_ZZ_4mu_m4l100.merge.DAOD_TRUTH4.e5518_p2913/DAOD_TRUTH4.10176349._000001.pool.root.1");
    }
    else if(opts["sampleType"].EqualTo("TestSherpaGGZZ"))
    {
        cutflow->setFileName("user.gabarone.mc15_13TeV.361073.Sherpa_CT10_ggllll.evgen.EVNT.e3836_prod09_EXT0/user.gabarone.8350832.EXT0._000009.DAOD_TRUTH0.truth.pool.root");
        cutflow->setOutName("Sherpa_CT10_ggllll");

        inputFilenames = std::vector<std::string> {
     "/home/goblirsc/F/Datasets/H4l/Truth0Plus/Prod09/user.gabarone.mc15_13TeV.361073.Sherpa_CT10_ggllll.evgen.EVNT.e3836_prod09_EXT0/user.gabarone.8350832.EXT0._000009.DAOD_TRUTH0.truth.pool.root",
     "/home/goblirsc/F/Datasets/H4l/Truth0Plus/Prod09/user.gabarone.mc15_13TeV.361073.Sherpa_CT10_ggllll.evgen.EVNT.e3836_prod09_EXT0/user.gabarone.8350832.EXT0._000010.DAOD_TRUTH0.truth.pool.root",
     "/home/goblirsc/F/Datasets/H4l/Truth0Plus/Prod09/user.gabarone.mc15_13TeV.361073.Sherpa_CT10_ggllll.evgen.EVNT.e3836_prod09_EXT0/user.gabarone.8350832.EXT0._000031.DAOD_TRUTH0.truth.pool.root",
     "/home/goblirsc/F/Datasets/H4l/Truth0Plus/Prod09/user.gabarone.mc15_13TeV.361073.Sherpa_CT10_ggllll.evgen.EVNT.e3836_prod09_EXT0/user.gabarone.8350832.EXT0._000056.DAOD_TRUTH0.truth.pool.root",
     "/home/goblirsc/F/Datasets/H4l/Truth0Plus/Prod09/user.gabarone.mc15_13TeV.361073.Sherpa_CT10_ggllll.evgen.EVNT.e3836_prod09_EXT0/user.gabarone.8350832.EXT0._000057.DAOD_TRUTH0.truth.pool.root"
        };
    }

    else if(opts["sampleType"].EqualTo("mc15c_ggH125_HerwigTest"))
    {
        cutflow->setFileName("user.salvucci.PowhegHerwig7_NNLOPS_ggH125_ZZ4l_Herwig_TRUTH4.20160322.v01_EXT0");
        cutflow->setOutName("mc15_ggF125Herwig");
        inputFilenames = getFileList("/afs/cern.ch/work/s/sabidi/public/data/user.salvucci.PowhegHerwig7_NNLOPS_ggH125_ZZ4l_Herwig_TRUTH4.20160322.v01_EXT0/", ".root");
    }
    else if(opts["sampleType"].EqualTo("mc15c_VBF125_HerwigTest"))
    {
        cutflow->setFileName("user.salvucci.PowhegHerwig7_NNPDF30_VBFH125_ZZ4l_Herwig_TRUTH4.20160322.v01_EXT0");
        cutflow->setOutName("mc15_VBF125Herwig");
        inputFilenames = getFileList("/afs/cern.ch/work/s/sabidi/public/data/user.salvucci.PowhegHerwig7_NNPDF30_VBFH125_ZZ4l_Herwig_TRUTH4.20160322.v01_EXT0/", ".root");
    }
    else if(opts["sampleType"].EqualTo("mc15c_ggH125_noVarTest"))
    {
        cutflow->setFileName("group.phys-higgs.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.variation.20170314.v02_DAOD_TRUTH4.NoVar.root");
        cutflow->setOutName("mc15_ggF125noVar");
        inputFilenames = std::vector<std::string> {
     "/afs/cern.ch/user/j/jomeyer/public/group.phys-higgs.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.variation.20170405.t03_DAOD_TRUTH4.NoVar.root/group.phys-higgs.11112505._000001.DAOD_TRUTH4.NoVar.root",
        };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ggH125_TRUTH4"))
    {
        cutflow->setFileName("mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_TRUTH4.e5682_p3075");
        cutflow->setOutName("mc15_ggF125Truth4");
        inputFilenames = std::vector<std::string> {
     "/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.345060.PowhegPythia8EvtGen_NNLOPS_nnlo_30_ggH125_ZZ4l.merge.DAOD_TRUTH4.e5682_p3075/DAOD_TRUTH4.11117682._000001.pool.root.1",
        };
    }
    else if(opts["sampleType"].EqualTo("MCFMRDSTest"))
    {
        cutflow->setFileName("mc15_13TeV.344826.MCFMPythia8EvtGen_ggH_gg_ZZ_5SMW_4mu_m4l130.merge.DAOD_HIGG2D1.e5518_a766_a821_r7676_p2879");
        cutflow->setOutName("MCFMTest5k");
        inputFilenames = std::vector<std::string> {
     "/afs/cern.ch/work/s/schaffer/public/data/mc15_13TeV.344826.MCFMPythia8EvtGen_ggH_gg_ZZ_5SMW_4mu_m4l130.merge.DAOD_HIGG2D1.e5518_a766_a821_r7676_p2879/mc15_13TeV/DAOD_HIGG2D1.10149480._000011.pool.root.1",
        };
    }
    else if(opts["sampleType"].EqualTo("SherpaTruth4Test"))
    {
        cutflow->setFileName("mc15_13TeV.363490.Sherpa_221_NNPDF30NNLO_llll.merge.DAOD_TRUTH4.e5332_p2913");
        cutflow->setOutName("SherpaTruth4Test");
        inputFilenames = std::vector<std::string> {
     "/home/goblirsc/F/Datasets/H4l/TRUTH4/mc15_13TeV.363490.Sherpa_221_NNPDF30NNLO_llll.merge.DAOD_TRUTH4.e5332_p2913/DAOD_TRUTH4.10762374._000163.pool.root.1",
        };
    }
    //////////////////////////////////////
    // Data
    //////////////////////////////////////

    else if(opts["sampleType"].EqualTo("data"))
    {
        cutflow->setFileName("data15_13TeV");
        cutflow->setOutName("data_13TeVExtra");
           inputFilenames = std::vector<std::string> {
        //"/afs/cern.ch/user/r/rdinardo/work/public/HZZCandidatesMoriond2017/user.rdinardo.10747723.EXT0._000001.merge.pool.2015.data.root",
        //"/afs/cern.ch/user/r/rdinardo/work/public/HZZCandidatesMoriond2017/2015_merge.root",
        //"/afs/cern.ch/user/r/rdinardo/work/public/HZZCandidatesMoriond2017/HZZCandidatesMoriond2017_2015_merge.root",
        //"/afs/cern.ch/user/r/rdinardo/work/public/HZZCandidatesMoriond2017/HZZCandidatesMoriond2017_2016_merge.root",
        //"/afs/cern.ch/work/j/jomeyer/public/DAOD_HIGG2D1.output.root",
        "/afs/cern.ch/user/j/jomeyer/public/DAOD_HIGG2D1.output.root",
           };
    }
    else if(opts["sampleType"].EqualTo("mc15c_ttHdiLep"))
    {
        cutflow->setFileName("mc15_13TeV.343365.aMcAtNloPythia8EvtGen_A14_NNPDF23_NNPDF30ME_ttH125_dilep.merge.DAOD_HIGG2D2.e4706_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15c_ttHdiLep");
        inputFilenames = getFileList("/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.343365.aMcAtNloPythia8EvtGen_A14_NNPDF23_NNPDF30ME_ttH125_dilep.merge.DAOD_HIGG2D1.e4706_s2726_r7772_r7676_p2879/", ".root");
    }
    else if(opts["sampleType"].EqualTo("mc15c_ttHsemiLep"))
    {
        cutflow->setFileName("mc15_13TeV.343366.aMcAtNloPythia8EvtGen_A14_NNPDF23_NNPDF30ME_ttH125_semilep.merge.DAOD_HIGG2D1.e4706_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15c_ttHsemiLep");
        inputFilenames = getFileList("/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.343366.aMcAtNloPythia8EvtGen_A14_NNPDF23_NNPDF30ME_ttH125_semilep.merge.DAOD_HIGG2D1.e4706_s2726_r7772_r7676_p2879/", ".root");
    }
    else if(opts["sampleType"].EqualTo("mc15c_ttHallHad"))
    {
        cutflow->setFileName("mc15_13TeV.343367.aMcAtNloPythia8EvtGen_A14_NNPDF23_NNPDF30ME_ttH125_allhad.merge.DAOD_HIGG2D1.e4706_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15c_ttHallHad");
        inputFilenames = getFileList("/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.343367.aMcAtNloPythia8EvtGen_A14_NNPDF23_NNPDF30ME_ttH125_allhad.merge.DAOD_HIGG2D1.e4706_s2726_r7772_r7676_p2879/", ".root");
    }

    else if(opts["sampleType"].EqualTo("mc15c_tHjb"))
    {
        cutflow->setFileName("mc15_13TeV.343273.MadGraphPythia8EvtGen_A14_CT10ME_tHjb125_lep.merge.DAOD_HIGG2D2.e4606_s2726_r7772_r7676_p2879");
        cutflow->setOutName("mc15c_tHjb");
        inputFilenames = getFileList("/afs/cern.ch/work/s/sabidi/public/data/mc15_13TeV.343273.MadGraphPythia8EvtGen_A14_CT10ME_tHjb125_lep.merge.DAOD_HIGG2D2.e4606_s2726_r7772_r7676_p2879/", ".root");
    }
    else if(opts["sampleType"].EqualTo("mc15c_tHW"))
    {
        cutflow->setFileName("mc15_13TeV.342001.aMcAtNloHppEG_UEEE5_CTEQ6L1_CT10ME_tWH125_lep_yt_plus1.merge.DAOD_HIGG2D2.e4394_s2608_r7772_r7676_p2879");
        cutflow->setOutName("mc15c_tHW");
        inputFilenames = getFileList("/afs/cern.ch/work/s/sabidi/public/data/user.rdinardo.342001_tWH.Cand.slim.v07_daod_out.root/", ".root");
    }
    else if(opts["sampleType"].EqualTo("mc15c_POSM"))
    {
        cutflow->setFileName("group.phys-higgs.user.sabidi.990260.daidaideriv.v03_EXT0");
        cutflow->setOutName("mc15c_POSM");
        inputFilenames = getFileList("/afs/cern.ch/user/a/angabrie/Work/public/POtest/group.phys-higgs.user.sabidi.990260.daidaideriv.v03_EXT0/", ".root");
    }
     else if(opts["sampleType"].EqualTo("mc15c_POBSM"))
    {
        cutflow->setFileName("group.phys-higgs.user.sabidi.990299.daidaideriv.v03_EXT0");
        cutflow->setOutName("mc15c_POBSM");
        inputFilenames = getFileList("/afs/cern.ch/user/a/angabrie/Work/public/POtest/group.phys-higgs.user.sabidi.990260.daidaideriv.v03_EXT0/", ".root");
    }
   
    else
    {
        cout<<"Cannot recognize sampleType"<<endl;
        cout<<"Input sampleType: "<<opts["sampleType"]<<endl;
        exit(1);
    }
    cutflow->setFiles(inputFilenames);
}

std::vector<std::string> getFileList(TString dirName, TString pattern)
{

    std::vector<std::string> fileNameVec;

    TSystemDirectory dir(dirName, dirName);
    TList *files = dir.GetListOfFiles();
    if (files) {
        TSystemFile *file;
        TString fname;
        TIter next(files);
        while ((file=(TSystemFile*)next())) {
            fname = file->GetName();
            //cout << fname.Data() << endl;         
            if (!file->IsDirectory() && fname.Contains(pattern)) {
                //cout << fname.Data() << endl;
                TString fileName = dirName + "/" + fname;
                fileNameVec.push_back(fileName.Data());
            }
        }
    }

    return fileNameVec;
}