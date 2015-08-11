#include "map"
#include "FWCore/Common/interface/TriggerNames.h"
#include "ggAnalysis/ggNtuplizer/interface/ggNtuplizer.h"

using namespace std;

// local variables: per-filter per-electron/muon/photon arrays of matched trigger objects
// NOTE: number of elements in the arrays equals sizeof(Int_t)
vector<float> trgElePt[32], trgEleEta[32], trgElePhi[32];
vector<float> trgPhoPt[32], trgPhoEta[32], trgPhoPhi[32];
vector<float> trgMuPt[32],  trgMuEta[32],  trgMuPhi[32];

void ggNtuplizer::initTriggerFilters(const edm::Event &e) {
  // Fills the arrays above.

  // cleanup from previous execution
  for (size_t i = 0; i < 32; i++) {
    trgElePt [i].clear();
    trgEleEta[i].clear();
    trgElePhi[i].clear();
    trgPhoPt [i].clear();
    trgPhoEta[i].clear();
    trgPhoPhi[i].clear();
    trgMuPt  [i].clear();
    trgMuEta [i].clear();
    trgMuPhi [i].clear();
  }

  // filter => index (in trg*[] arrays) mappings
  static std::map<string,size_t> eleFilters;
  static std::map<string,size_t> phoFilters;
  static std::map<string,size_t> muFilters;

  // one-time initialization
  if (eleFilters.size() == 0) {
    // FIXME: define actual filters
    // FIXME: using only the latest filter in a HLT is not sufficient
    eleFilters["hltSingleEle22WPLooseGsfTrackIsoFilter"] = 0;
    eleFilters["hltL1sL1SingleEG20ORL1SingleEG15"] = 1;
    eleFilters["hltEle25WP60SC4HcalIsoFilter"] = 2;
  }

  if (phoFilters.size() == 0) {
    phoFilters["hltEG22HEFilter"]    = 0;
    phoFilters["hltEG30HEFilter"]    = 1;
    phoFilters["hltEG36HEFilter"]    = 2;
    phoFilters["hltEG50HEFilter"]    = 3;
    phoFilters["hltEG75HEFilter"]    = 4;
    phoFilters["hltEG90HEFilter"]    = 5;
    phoFilters["hltEG120HEFilter"]   = 6;
    phoFilters["hltEG165HE10Filter"] = 7;
    phoFilters["hltEG175HEFilter"]   = 8;
    phoFilters["hltEG250erEtFilter"] = 9;
    phoFilters["hltEG300erEtFilter"] = 10;
    phoFilters["hltEG500HEFilter"]   = 11;
    phoFilters["hltEG600HEFilter"]   = 12;
  }

  // AOD vs miniAOD
  if (isAOD_) {
    edm::Handle<trigger::TriggerEvent> triggerHandle;
    e.getByToken(trgEventLabel_, triggerHandle);

    const trigger::TriggerObjectCollection& trgObjects = triggerHandle->getObjects();

    // loop over particular filters (and not over full HLTs)
    for (trigger::size_type iF = 0; iF != triggerHandle->sizeFilters(); ++iF) {
      // full filter name and its keys each corresponding to a matched (pt, eta, phi, ...) object
      string const&        label = triggerHandle->filterTag(iF).label();
      const trigger::Keys& keys  = triggerHandle->filterKeys(iF);

      std::map<string,size_t>::iterator idxEle = eleFilters.find(label);
      std::map<string,size_t>::iterator idxPho = phoFilters.find(label);
      std::map<string,size_t>::iterator idxMu  = muFilters.find(label);

      // electron filters
      if (idxEle != eleFilters.end()) {
        size_t idx = idxEle->second;

        for (size_t iK = 0; iK < keys.size(); ++iK) {
          const trigger::TriggerObject& trgV = trgObjects.at(keys[iK]);
          trgElePt [idx].push_back(trgV.pt());
          trgEleEta[idx].push_back(trgV.eta());
          trgElePhi[idx].push_back(trgV.phi());
        }
      }

      // photon filters
      if (idxPho != phoFilters.end()) {
        size_t idx = idxPho->second;

        for (size_t iK = 0; iK < keys.size(); ++iK) {
          const trigger::TriggerObject& trgV = trgObjects.at(keys[iK]);
          trgPhoPt [idx].push_back(trgV.pt());
          trgPhoEta[idx].push_back(trgV.eta());
          trgPhoPhi[idx].push_back(trgV.phi());
        }
      }

      // muon filters
      if (idxMu != muFilters.end()) {
        size_t idx = idxMu->second;

        for (size_t iK = 0; iK < keys.size(); ++iK) {
          const trigger::TriggerObject& trgV = trgObjects.at(keys[iK]);
          trgMuPt [idx].push_back(trgV.pt());
          trgMuEta[idx].push_back(trgV.eta());
          trgMuPhi[idx].push_back(trgV.phi());
        }
      }
    } // HLT filter loop

    return;
  } // if AOD

  //
  // miniAOD treatment
  //

  edm::Handle<pat::TriggerObjectStandAloneCollection> triggerHandleMiniAOD;
  e.getByToken(triggerObjectsLabel_, triggerHandleMiniAOD);

  edm::Handle<edm::TriggerResults> trgResultsHandle;
  e.getByToken(trgResultsLabel_, trgResultsHandle);

  const edm::TriggerNames &names = e.triggerNames(*trgResultsHandle);

  for (pat::TriggerObjectStandAlone obj : *triggerHandleMiniAOD) {
    obj.unpackPathNames(names);

    // loop over filters
    for (size_t iF = 0; iF < obj.filterLabels().size(); ++iF) {
      string label = obj.filterLabels()[iF];

      std::map<string,size_t>::iterator idxEle = eleFilters.find(label);
      std::map<string,size_t>::iterator idxPho = phoFilters.find(label);
      std::map<string,size_t>::iterator idxMu  = muFilters.find(label);

      // electron filters
      if (idxEle != eleFilters.end()) {
        size_t idx = idxEle->second;
        trgElePt [idx].push_back(obj.pt());
        trgEleEta[idx].push_back(obj.eta());
        trgElePhi[idx].push_back(obj.phi());
      }

      // photon filters
      if (idxPho != phoFilters.end()) {
        size_t idx = idxPho->second;
        trgPhoPt [idx].push_back(obj.pt());
        trgPhoEta[idx].push_back(obj.eta());
        trgPhoPhi[idx].push_back(obj.phi());
      }

      // muon filters
      if (idxMu != muFilters.end()) {
        size_t idx = idxMu->second;
        trgMuPt [idx].push_back(obj.pt());
        trgMuEta[idx].push_back(obj.eta());
        trgMuPhi[idx].push_back(obj.phi());
      }
    }
  }

}

Int_t ggNtuplizer::matchElectronTriggerFilters(double pt, double eta, double phi) {

  // bits in the return value correspond to decisions from filters defined above
  Int_t result = 0;

  for (size_t f = 0; f < 32; ++f)
    for (size_t v = 0; v < trgElePt[f].size(); ++v)
      if (fabs(pt - trgElePt[f][v])/trgElePt[f][v] < trgFilterDeltaPtCut_ &&
          deltaR(eta, phi, trgEleEta[f][v], trgElePhi[f][v]) < trgFilterDeltaRCut_) {
        result |= (1<<f);
        break;
      }

  return result;
}

Int_t ggNtuplizer::matchPhotonTriggerFilters(double pt, double eta, double phi) {

  // bits in the return value correspond to decisions from filters defined above
  Int_t result = 0;

  for (size_t f = 0; f < 32; ++f)
    for (size_t v = 0; v < trgPhoPt[f].size(); ++v)
      if (fabs(pt - trgPhoPt[f][v])/trgPhoPt[f][v] < trgFilterDeltaPtCut_ &&
          deltaR(eta, phi, trgPhoEta[f][v], trgPhoPhi[f][v]) < trgFilterDeltaRCut_) {
        result |= (1<<f);
        break;
      }

  return result;
}

Int_t ggNtuplizer::matchMuonTriggerFilters(double pt, double eta, double phi) {

  // bits in the return value correspond to decisions from filters defined above
  Int_t result = 0;

  for (size_t f = 0; f < 32; ++f)
    for (size_t v = 0; v < trgMuPt[f].size(); ++v)
      if (fabs(pt - trgMuPt[f][v])/trgMuPt[f][v] < trgFilterDeltaPtCut_ &&
          deltaR(eta, phi, trgMuEta[f][v], trgMuPhi[f][v]) < trgFilterDeltaRCut_) {
        result |= (1<<f);
        break;
      }

  return result;
}

Double_t ggNtuplizer::deltaPhi(Double_t phi1, Double_t phi2) {

  Double_t dPhi = phi1 - phi2;
  if (dPhi > TMath::Pi()) dPhi -= 2.*TMath::Pi();
  if (dPhi < -TMath::Pi()) dPhi += 2.*TMath::Pi();

  return dPhi;
}

Double_t ggNtuplizer::deltaEta(Double_t eta1, Double_t eta2) {
  return eta1 - eta2;
}

Double_t ggNtuplizer::deltaR(Double_t eta1, Double_t phi1, Double_t eta2, Double_t phi2) {

  Double_t dEta, dPhi ;
  dEta = eta1 - eta2;
  dPhi = deltaPhi(phi1, phi2);

  return sqrt(dEta*dEta+dPhi*dPhi);
}
