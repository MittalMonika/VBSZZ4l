// 
// Author Yanyan Gao (Yanyan.Gao@cern.ch)
// 

#include "TFile.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TProfile.h"
#include <iostream>
#include "Math/LorentzVector.h"
#include "TLorentzRotation.h"
#include "Math/VectorUtil.h"
// ME related
#include "MCFM_MatrixElement/TVar.h"
#include "MCFM_MatrixElement/TEvtProb.h"
#include "PathResolver/PathResolver.h"
#include "math.h"

#include <boost/filesystem.hpp>

typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<float> > LorentzVector; 

float ERRORthreshold=1.0;
using namespace std;
//using namespace boost::filesystem;

namespace HSG2_MCFM{
  bool OrganiseMCFMStuff();
}

vector<TLorentzVector> Calculate4Momentum(float Mx,float M1,float M2,float theta,float theta1,float theta2,float Phi1,float Phi)
{
  float phi1,phi2;
  phi1=TMath::Pi()-Phi1;
  phi2=Phi1+Phi;


  float gamma1,gamma2,beta1,beta2;

  gamma1=(Mx*Mx+M1*M1-M2*M2)/(2*Mx*M1);
  gamma2=(Mx*Mx-M1*M1+M2*M2)/(2*Mx*M2);
  beta1=sqrt(1-1/(gamma1*gamma1));
  beta2=sqrt(1-1/(gamma2*gamma2));


  //gluon 4 vectors
  TLorentzVector p1CM(0,0,Mx/2,Mx/2);
  TLorentzVector p2CM(0,0,-Mx/2,Mx/2);

  //vector boson 4 vectors
  TLorentzVector kZ1(gamma1*M1*sin(theta)*beta1,0, gamma1*M1*cos(theta)*beta1,gamma1*M1*1);   
  TLorentzVector kZ2(-gamma2*M2*sin(theta)*beta2,0, -gamma2*M2*cos(theta)*beta2,gamma2*M2*1);

  //Rotation and Boost matrices. Note gamma1*beta1*M1=gamma2*beta2*M2.

  TLorentzRotation Z1ToZ,Z2ToZ;

  Z1ToZ.Boost(0,0,beta1);
  Z2ToZ.Boost(0,0,beta2);
  Z1ToZ.RotateY(theta);
  Z2ToZ.RotateY(TMath::Pi()+theta);


  //fermons 4 vectors in vector boson rest frame

  TLorentzVector p3Z1((M1/2)*sin(theta1)*cos(phi1),(M1/2)*sin(theta1)*sin(phi1),(M1/2)*cos(theta1),(M1/2)*1);

  TLorentzVector p4Z1(-(M1/2)*sin(theta1)*cos(phi1),-(M1/2)*sin(theta1)*sin(phi1),-(M1/2)*cos(theta1),(M1/2)*1);

  TLorentzVector p5Z2((M2/2)*sin(theta2)*cos(phi2),(M2/2)*sin(theta2)*sin(phi2),(M2/2)*cos(theta2),(M2/2)*1);

  TLorentzVector p6Z2(-(M2/2)*sin(theta2)*cos(phi2),-(M2/2)*sin(theta2)*sin(phi2),-(M2/2)*cos(theta2),(M2/2)*1);


  // fermions 4 vectors in CM frame

  TLorentzVector p3CM,p4CM,p5CM,p6CM;

  p3CM=Z1ToZ*p3Z1;
  p4CM=Z1ToZ*p4Z1;
  p5CM=Z2ToZ*p5Z2;
  p6CM=Z2ToZ*p6Z2;

  vector<TLorentzVector> p;

  p.push_back(p3CM);
  p.push_back(p4CM);
  p.push_back(p5CM);
  p.push_back(p6CM);

  return p;
}



//###################
//# main function
//###################
int main(int , char* []) {

  TVar::VerbosityLevel verbosity = TVar::INFO;

  double dXsec_ZZ_MCFM = 0.;
  double dXsec_GGZZ_MCFM = 0.;
  double dXsec_GGZZTOT_MCFM = 0.;
  double dXsec_GGZZINT_MCFM = 0.;
  double dXsec_HZZ_MCFM = 0.;
  
  float mzz = 126.; 
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  int mflavor = 3; 

  // Create the instance of TEvtProb to calculate the differential cross-section
  std::cout << "Organising the inputs..."<<std::endl;
  if (!HSG2_MCFM::OrganiseMCFMStuff()){
    std::cerr << "This went wrong, bailing out!"<<std::endl;
    return EXIT_FAILURE;
  }
  TEvtProb Xcal2;
  hzz4l_event_type hzz4l_event;

  // set four momenta
  vector<TLorentzVector> p;
  p=Calculate4Momentum(mzz,m1,m2,acos(hs),acos(h1),acos(h2),phi1,phi);

  TLorentzVector Z1_minus = p[0];
  TLorentzVector Z1_plus  = p[1];
  TLorentzVector Z2_minus = p[2];
  TLorentzVector Z2_plus  = p[3];

  hzz4l_event.p[0].SetXYZM(Z1_minus.Px(), Z1_minus.Py(), Z1_minus.Pz(), 0.);
  hzz4l_event.p[1].SetXYZM(Z1_plus.Px(), Z1_plus.Py(), Z1_plus.Pz(), 0.);
  hzz4l_event.p[2].SetXYZM(Z2_minus.Px(), Z2_minus.Py(), Z2_minus.Pz(), 0.);
  hzz4l_event.p[3].SetXYZM(Z2_plus.Px(), Z2_plus.Py(), Z2_plus.Pz(), 0.);


  // flavor 1 for 4e, 2 for 4m, 3 for 2e2mu  
  if ( mflavor == 1 ) {
    hzz4l_event.PdgCode[0] = 11;
    hzz4l_event.PdgCode[1] = -11;
    hzz4l_event.PdgCode[2] = 11;
    hzz4l_event.PdgCode[3] = -11;
  }
  if ( mflavor == 2 ) {
    hzz4l_event.PdgCode[0] = 13;
    hzz4l_event.PdgCode[1] = -13;
    hzz4l_event.PdgCode[2] = 13;
    hzz4l_event.PdgCode[3] = -13;
  }
  if ( mflavor == 3 ) {
    hzz4l_event.PdgCode[0] = 11;
    hzz4l_event.PdgCode[1] = -11;
    hzz4l_event.PdgCode[2] = 13;
    hzz4l_event.PdgCode[3] = -13;
  }


  float z1mass = (hzz4l_event.p[0]+hzz4l_event.p[1]).M();
  float z2mass = (hzz4l_event.p[2]+hzz4l_event.p[3]).M();
  float zzmass = (hzz4l_event.p[0]+hzz4l_event.p[1]+hzz4l_event.p[2]+hzz4l_event.p[3]).M();

  if (verbosity >= TVar::INFO) {
    std::cout << "Input: ==================================================" <<endl;
    printf("lep1 (Px, Py, Pz, E) = (%4.4f, %4.4f, %4.4f, %4.4f)\n",  p[0].Px(), p[0].Py(), p[0].Pz(), p[0].E());
    printf("lep2 (Px, Py, Pz, E) = (%4.4f, %4.4f, %4.4f, %4.4f)\n",  p[1].Px(), p[1].Py(), p[1].Pz(), p[1].E()); 
    printf("lep3 (Px, Py, Pz, E) = (%4.4f, %4.4f, %4.4f, %4.4f)\n",  p[2].Px(), p[2].Py(), p[2].Pz(), p[2].E());
    printf("lep4 (Px, Py, Pz, E) = (%4.4f, %4.4f, %4.4f, %4.4f)\n",  p[3].Px(), p[3].Py(), p[3].Pz(), p[3].E()); 
    std::cout << "ZZ system (pX, pY, pZ, E, mass) = ( " 
      << (hzz4l_event.p[0]+hzz4l_event.p[1]+hzz4l_event.p[2]+hzz4l_event.p[3]).Px() << ", "
      << (hzz4l_event.p[0]+hzz4l_event.p[1]+hzz4l_event.p[2]+hzz4l_event.p[3]).Py() << ", "
      << (hzz4l_event.p[0]+hzz4l_event.p[1]+hzz4l_event.p[2]+hzz4l_event.p[3]).Pz() << ", "
      << (hzz4l_event.p[0]+hzz4l_event.p[1]+hzz4l_event.p[2]+hzz4l_event.p[3]).Energy()  << ", "
      << zzmass << ")\n";
    std::cout << "Z1 mass = " << z1mass << "\tz2mass = " << z2mass << "\n";
    std::cout << "=========================================================\n";
  } 
  // finish event information

  // ==== Begin the differential cross-section calculation
  Xcal2.SetHiggsMass(zzmass);
  // calculate the ZZ using MCFM
  Xcal2.SetMatrixElement(TVar::MCFM);
  /*
  if ( mflavor < 3  )
    dXsec_ZZ_MCFM = Xcal2.XsecCalc(TVar::ZZ_4e, TVar::GG, hzz4l_event,verbosity);
  else 
    dXsec_ZZ_MCFM = Xcal2.XsecCalc(TVar::ZZ_2e2m, TVar::GG, hzz4l_event,verbosity);
  */
  dXsec_ZZ_MCFM = Xcal2.XsecCalc(TVar::QQbZZ_4l, TVar::GG, hzz4l_event,verbosity);
  dXsec_GGZZ_MCFM = Xcal2.XsecCalc(TVar::GGZZ_4l, TVar::GG, hzz4l_event,verbosity);
  dXsec_HZZ_MCFM = Xcal2.XsecCalc(TVar::HZZ_4l, TVar::GG, hzz4l_event,verbosity);
  dXsec_GGZZTOT_MCFM = Xcal2.XsecCalc(TVar::GGZZTOT_4l, TVar::GG, hzz4l_event,verbosity);
  dXsec_GGZZINT_MCFM = Xcal2.XsecCalc(TVar::GGZZINT_4l, TVar::GG, hzz4l_event,verbosity);

if ( verbosity >= TVar::INFO ) { 
    FILE *output = fopen("output.txt", "w");
    fprintf(output, "===== MCFM Matrix Element outputs==============\n");
    fprintf(output, "gg->H->ZZ: %7.7e\n",  dXsec_HZZ_MCFM); 
    fprintf(output, "qqbar->ZZ: %7.7e\n",  dXsec_ZZ_MCFM); 
    fprintf(output, "gg->ZZ %7.7e\n", dXsec_GGZZ_MCFM);
    fprintf(output, "gg->ZZ(including HZZ): %7.7e\n", dXsec_GGZZTOT_MCFM);
    fprintf(output, "gg->ZZ and gg->H->ZZ interference:%7.7e\n", dXsec_GGZZINT_MCFM);
    fprintf(output, "==============================================\n");
    fclose(output);
  }

    return EXIT_SUCCESS;
}
