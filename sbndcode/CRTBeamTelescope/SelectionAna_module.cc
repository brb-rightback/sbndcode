////////////////////////////////////////////////////////////////////////
// Class:       SelectionAna
// Plugin Type: analyzer (Unknown Unknown)
// File:        SelectionAna_module.cc
//
// Generated at Tue Nov 14 06:04:47 2023 by Jiaoyang Li using cetskelgen
// from  version .
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Core/FileBlock.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art_root_io/TFileService.h"
#include "canvas/Persistency/Common/FindManyP.h"

#include "TTree.h"
#include "nusimdata/SimulationBase/MCTruth.h"

#include "larcoreobj/SummaryData/POTSummary.h"
#include "sbnobj/Common/CRT/CRTHit.hh"
#include "sbnobj/Common/CRT/CRTTrack.hh"
#include "sbndcode/CRT/CRTUtils/CRTBackTracker.h"
#include <numeric>
#include <algorithm>  // for sort
#include <functional> // for greater

class SelectionAna;

class SelectionAna : public art::EDAnalyzer {
public:
  explicit SelectionAna(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  SelectionAna(SelectionAna const&) = delete;
  SelectionAna(SelectionAna&&) = delete;
  SelectionAna& operator=(SelectionAna const&) = delete;
  SelectionAna& operator=(SelectionAna&&) = delete;

  // Required functions.
  void analyze(art::Event const& e) override;

  // Selected optional functions.
  void beginSubRun(art::SubRun const& sr) override;
  void respondToOpenInputFile(const art::FileBlock& fb) override;
  void calculateMeanStd(std::vector<double> vec, double &mean, double &std);
  bool canFormSquare(std::vector<double> distances);
private:
  sbnd::CRTBackTracker _crt_back_tracker;
  // Declare member data here.
  std::string _crthit_label;
  std::string _pot_label;
  std::string _mctruth_label;

  bool _debug;
  bool _save_input_file_name;
  bool _data_mode; 
  int _interationMode; // 0 -- dark neutrino 140 MeV; 1 -- dark neutrino 400 MeV; 2 -- dirt; 3 -- cosmic; 

  TTree* _tree;
  int _run, _subrun, _event;
  std::string _file_name;
  float _weight;                                 ///< Signal: the weight which store as the vertex of dark neutrino.

  int _n_chits_upstream, _n_chits_downstream;    ///< Number of CRT hits in the upstream/downstream CRTs
  bool _isSquare;                                ///< Can CRT hits form a square?
  std::vector<double> _chit_pes;                 ///< CRT hit PEs
  double _chit_pes_mean;                         ///< Mean of the CRT hit PE for one event. 
  double _chit_pes_sample_std;                   ///< Sample standard deviation of the CRT hit PE for one event.
  
  std::vector<double> _chit_t1_diff;            ///< CRT hit difference between the two 1D hit t1s
  double _chit_t1_diff_mean;                    ///< Mean of t1 difference between the two closest CRT hit times for one event.
  double _chit_t1_diff_sample_std;              ///< Sample standard deviation of t1 difference between the two closest CRT hit times for one event.
  
  std::vector<double> _distance_between_hits_downstream;     ///< Distance between any two hits. 
  double _distance_between_hits_downstream_mean;             ///< Mean of the distance between any two hits for one event.
  double _distance_between_hits_downstream_sample_std;       ///< Sample standard deviation of the distance between any two hits for one event.
  double _biggest_distance_between_hits_downstream_mean;     ///< Mean of the distance between any two hits for one event.
  double _biggest_distance_between_hits_downstream_diff;     ///< Mean of the distance between any two hits for one event.
  double _smallest_distance_between_hits_downstream_mean;    ///< Mean of the distance between any two hits for one event.
  double _smallest_distance_between_hits_downstream_diff;    ///< Mean of the distance between any two hits for one event.

  std::vector<double> _t1_between_hits_downstream;     ///< Distance between any two hits. 
  double _t1_between_hits_downstream_mean;             ///< Mean of the t1 between any two hits for one event.
  double _t1_between_hits_downstream_sample_std;       ///< Sample standard deviation of the t1 between any two hits for one event.
  double _biggest_distance_t1_between_hits_downstream_mean;     ///< Mean of the t1 between any two hits for one event.
  double _biggest_distance_t1_between_hits_downstream_diff;     ///< Mean of the t1 between any two hits for one event.
  double _smallest_distance_t1_between_hits_downstream_mean;    ///< Mean of the t1 between any two hits for one event.
  double _smallest_distance_t1_between_hits_downstream_diff;    ///< Mean of the t1 between any two hits for one event.

  std::vector<double> _chit_t0;     ///< CRT hit t0
  std::vector<double> _chit_t1;     ///< CRT hit t1
  std::vector<float>  _chit_true_t; ///< CRT hit true time (from sim energy dep)

  // truth info for crt_hit
  std::vector<int>    _chit_backtrack_pdg;              ///< CRT hit, backtracking truth information of the pdg code 
  std::vector<double> _chit_backtrack_energy;           ///< CRT hit, backtracking truth information of the particle energy
  std::vector<double> _chit_backtrack_deposited_energy; ///< CRT hit, backtracking truth information of the deposited energy 
  std::vector<double> _chit_backtrack_purity;           ///< CRT hit, backtracking truth information of selection purity
  std::vector<double> _chit_backtrack_trackID;          ///< CRT hit, backtracking truth information of track id

  // POT info. 
  TTree* _sr_tree;
  int _sr_run, _sr_subrun;
  double _sr_begintime, _sr_endtime;
  double _sr_pot; ///< Number of POTs per subrun
  double _sr_spills; ///< Number of Spills per subrun
};


SelectionAna::SelectionAna(fhicl::ParameterSet const& p)
  : EDAnalyzer{p}  // ,
  // More initializers here.
{
  // Call appropriate consumes<>() for any products to be retrieved by this module.
  _mctruth_label        = p.get<std::string>("MCTruthLabel", "generator");
  _crthit_label         = p.get<std::string>("CRTHitLabel", "crthit");
  _pot_label            = p.get<std::string>("POTLabel", "generator");
  _crt_back_tracker     = p.get<fhicl::ParameterSet>("CRTBackTracker", fhicl::ParameterSet());
  _interationMode       = p.get<int>("InteractionMode", 0);
  _debug                = p.get<bool>("Debug", false);
  _save_input_file_name = p.get<bool>("SaveInputFileName", true);
  _data_mode            = p.get<bool>("DataMode", false);

  art::ServiceHandle<art::TFileService> fs;

  _tree = fs->make<TTree>("selection_tree","");
  _tree->Branch("run", &_run, "run/I");
  _tree->Branch("subrun", &_subrun, "subrun/I");
  _tree->Branch("event", &_event, "event/I");
  if (_save_input_file_name) _tree->Branch("file_name", &_file_name);
  if (!_data_mode) _tree->Branch("weight", &_weight, "weight/F");
  _tree->Branch("n_chits_upstream", &_n_chits_upstream, "n_chits_upstream/I");
  _tree->Branch("n_chits_downstream", &_n_chits_downstream, "n_chits_downstream/I");
  _tree->Branch("isSquare", &_isSquare, "isSquare/O");

  _tree->Branch("chit_pes", &_chit_pes);
  _tree->Branch("chit_pes_mean", &_chit_pes_mean, "chit_pes_mean/D");
  _tree->Branch("chit_pes_sample_std", &_chit_pes_sample_std, "chit_pes_sample_std/D");

  _tree->Branch("chit_t1_diff", &_chit_t1_diff);
  _tree->Branch("chit_t1_diff_mean", &_chit_t1_diff_mean, "chit_t1_diff_mean/D");
  _tree->Branch("chit_t1_diff_sample_std", &_chit_t1_diff_sample_std, "chit_t1_diff_sample_std/D");

  _tree->Branch("distance_between_hits", &_distance_between_hits_downstream);
  _tree->Branch("distance_between_hits_mean", &_distance_between_hits_downstream_mean, "distance_between_hits_mean/D");
  _tree->Branch("distance_between_hits_sample_std", &_distance_between_hits_downstream_sample_std, "distance_between_hits_sample_std/D");

  _tree->Branch("biggest_distance_between_hits_mean", &_biggest_distance_between_hits_downstream_mean, "biggest_distance_between_hits_mean/D");
  _tree->Branch("biggest_distance_between_hits_diff", &_biggest_distance_between_hits_downstream_diff, "biggest_distance_between_hits_diff/D");

  _tree->Branch("smallest_distance_between_hits_mean", &_smallest_distance_between_hits_downstream_mean, "smallest_distance_between_hits_mean/D");
  _tree->Branch("smallest_distance_between_hits_diff", &_smallest_distance_between_hits_downstream_diff, "smallest_distance_between_hits_diff/D");

  _tree->Branch("t1_between_hits", &_t1_between_hits_downstream);
  _tree->Branch("t1_between_hits_mean", &_t1_between_hits_downstream_mean, "t1_between_hits_mean/D");
  _tree->Branch("t1_between_hits_sample_std", &_t1_between_hits_downstream_sample_std, "t1_between_hits_sample_std/D");

  _tree->Branch("biggest_distance_t1_between_hits_mean", &_biggest_distance_t1_between_hits_downstream_mean, "biggest_distance_t1_between_hits_mean/D");
  _tree->Branch("biggest_distance_t1_between_hits_diff", &_biggest_distance_t1_between_hits_downstream_diff, "biggest_distance_t1_between_hits_diff/D");

  _tree->Branch("smallest_distance_t1_between_hits_mean", &_smallest_distance_t1_between_hits_downstream_mean, "smallest_distance_t1_between_hits_mean/D");
  _tree->Branch("smallest_distance_t1_between_hits_diff", &_smallest_distance_t1_between_hits_downstream_diff, "smallest_distance_t1_between_hits_diff/D");

  _tree->Branch("chit_t1", &_chit_t1);

  if (!_data_mode){
    _tree->Branch("chit_true_t", &_chit_true_t);
    _tree->Branch("chit_backtrack_pdg", &_chit_backtrack_pdg);
    _tree->Branch("chit_backtrack_energy", &_chit_backtrack_energy);
    _tree->Branch("chit_backtrack_deposited_energy", &_chit_backtrack_deposited_energy);
    _tree->Branch("chit_backtrack_purity", &_chit_backtrack_purity);
    _tree->Branch("chit_backtrack_trackID", &_chit_backtrack_trackID);
  }

  // POT tree. 
  _sr_tree = fs->make<TTree>("pottree","");
  _sr_tree->Branch("run", &_sr_run, "run/I");
  _sr_tree->Branch("subrun", &_sr_subrun, "subrun/I");
  _sr_tree->Branch("begintime", &_sr_begintime, "begintime/D");
  _sr_tree->Branch("endtime", &_sr_endtime, "endtime/D");
  _sr_tree->Branch("pot", &_sr_pot, "pot/D");
}

void SelectionAna::analyze(art::Event const& e)
{
  // Implementation of required member function here.
  if (!_data_mode) _crt_back_tracker.Initialize(e); // Initialise the backtrack alg. 

  _run    = e.id().run();
  _subrun = e.id().subRun();
  _event  = e.id().event();
  if(_debug) {
    std::cout << "This is file: " << _file_name << std::endl;
    std::cout << "This is run:subrun:event " << _run << ":" << _subrun << ":" << _event << std::endl;
  }

  // Get the mc truth information for the weight for signal. 
  if(!_data_mode){
    if (_interationMode==0 || _interationMode==1) {
      art::Handle<std::vector<simb::MCTruth>> mctruth_handle;
      e.getByLabel(_mctruth_label, mctruth_handle);
      if(!mctruth_handle.isValid()){
        std::cout << "MCTruth product " << _mctruth_label << " not found..." << std::endl;
        throw std::exception();
      }
      std::vector<art::Ptr<simb::MCTruth>> mct_v;
      art::fill_ptr_vector(mct_v, mctruth_handle);
      for (size_t imct=0; imct< mct_v.size(); imct++){
        auto mct = mct_v[imct];
        for (int ipar = 0; ipar < mct->NParticles(); ipar++) {
          if (ipar==2) {
            auto particle = mct->GetParticle(ipar);
            _weight = particle.Vx();
          }
        }
      }
    }else{
        _weight = 1.;
    }
  }

  //
  // Get the CRT Hits
  //
  art::Handle<std::vector<sbn::crt::CRTHit>> crt_hit_handle;
  e.getByLabel(_crthit_label, crt_hit_handle);
  if(!crt_hit_handle.isValid()){
    std::cout << "CRTHit product " << _crthit_label << " not found..." << std::endl;
    throw std::exception();
  }
  std::vector<art::Ptr<sbn::crt::CRTHit>> crt_hit_v;
  art::fill_ptr_vector(crt_hit_v, crt_hit_handle);

  size_t n_hits = crt_hit_v.size();
  _chit_t0.resize(n_hits);
  _chit_t1.resize(n_hits);
  _chit_t1_diff.resize(n_hits);
  _chit_pes.resize(n_hits);
  if(!_data_mode){
    _chit_true_t.resize(n_hits);
    _chit_backtrack_pdg.resize(n_hits);
    _chit_backtrack_energy.resize(n_hits);
    _chit_backtrack_deposited_energy.resize(n_hits);
    _chit_backtrack_purity.resize(n_hits);
    _chit_backtrack_trackID.resize(n_hits);
  }
  std::vector<double> chit_x_downstream, chit_y_downstream, chit_t1_downstream;
  chit_x_downstream.clear(); chit_y_downstream.clear(); chit_t1_downstream.clear();
  _n_chits_upstream = 0; _n_chits_downstream = 0;
  for (size_t ihit = 0; ihit < n_hits; ihit++) {
    auto hit = crt_hit_v[ihit];

    _chit_t0[ihit]      = hit->ts0_ns;
    _chit_t1[ihit]      = hit->ts1_ns;
    _chit_pes[ihit]     = hit->peshit;
    _chit_t1_diff[ihit] = hit->ts0_ns_corr; // the variable is old and just a placeholder for diff

    if (hit->tagger == "volTaggerNorth_0") { // downstream
      _n_chits_downstream++; 
      chit_x_downstream.push_back(hit->x_pos); chit_y_downstream.push_back(hit->y_pos); 
      chit_t1_downstream.push_back(hit->ts1_ns);
    } else {  // upstream
      _n_chits_upstream++;
    }

    const sbnd::CRTBackTracker::TruthMatchMetrics truthMatch = _crt_back_tracker.TruthMatrixFromTotalEnergy(e, hit);
    _chit_backtrack_pdg[ihit]              = truthMatch.pdg;
    _chit_backtrack_energy[ihit]           = truthMatch.particle_energy;
    _chit_backtrack_deposited_energy[ihit] = truthMatch.depEnergy_total;
    _chit_backtrack_purity[ihit]           = truthMatch.purity;
    _chit_backtrack_trackID[ihit]          = truthMatch.trackid;
  }

  // Calculate the distance between any two hits.
  std::map<double, std::vector<double>> distance_to_t1_diff_map; distance_to_t1_diff_map.clear();
  for (size_t i=0; i<chit_x_downstream.size(); i++){
    for (size_t j=i+1; j<chit_x_downstream.size(); j++){
      double distance = std::hypot(chit_x_downstream[i]-chit_x_downstream[j], chit_y_downstream[i]-chit_y_downstream[j]);
      _distance_between_hits_downstream.push_back(distance);

      double t1_diff_between_hits = std::abs(chit_t1_downstream[i]-chit_t1_downstream[j]);
      _t1_between_hits_downstream.push_back(t1_diff_between_hits);
      
      distance_to_t1_diff_map[distance].push_back(t1_diff_between_hits);
    }
  }

  if (canFormSquare(_distance_between_hits_downstream)) _isSquare = true;
  else _isSquare = false;

  // Calculate the mean and std of the distance between any two hits.
  calculateMeanStd(_distance_between_hits_downstream, _distance_between_hits_downstream_mean, _distance_between_hits_downstream_sample_std);

  // Calculate the mean and std of the t1 between any two hits.
  calculateMeanStd(_t1_between_hits_downstream, _t1_between_hits_downstream_mean, _t1_between_hits_downstream_sample_std);

  size_t counter=0; 
  _smallest_distance_between_hits_downstream_mean=0;    _biggest_distance_between_hits_downstream_mean=0;
  _smallest_distance_t1_between_hits_downstream_diff=0; _biggest_distance_t1_between_hits_downstream_diff=0;
  _smallest_distance_between_hits_downstream_diff=0;    _biggest_distance_between_hits_downstream_diff=0;
  _smallest_distance_t1_between_hits_downstream_diff=0; _biggest_distance_t1_between_hits_downstream_diff=0;
  size_t counter_smallest=0, counter_biggest=0;
  for (auto & [distance, t1_vec] : distance_to_t1_diff_map){
    if (counter==0){
        for(size_t ipair=0; ipair<t1_vec.size(); ipair++){
            counter_smallest++;
            _smallest_distance_between_hits_downstream_mean    += distance;
            _smallest_distance_t1_between_hits_downstream_diff += t1_vec[ipair];

            if (ipair==0){
                _smallest_distance_between_hits_downstream_diff    = distance;
                _smallest_distance_t1_between_hits_downstream_diff = t1_vec[ipair];
            }else{
                _smallest_distance_between_hits_downstream_diff    -= distance;
                _smallest_distance_t1_between_hits_downstream_diff -= t1_vec[ipair];

                _smallest_distance_between_hits_downstream_diff    = std::abs(_smallest_distance_between_hits_downstream_diff);
                _smallest_distance_t1_between_hits_downstream_diff = std::abs(_smallest_distance_t1_between_hits_downstream_diff);
            }
        }
    }
    if (counter==distance_to_t1_diff_map.size()-1){ // the maximum distance should belongs to the diagonal hits.
        counter_biggest++;
        for(size_t ipair=0; ipair<t1_vec.size(); ipair++){
            _biggest_distance_between_hits_downstream_mean    += distance;
            _biggest_distance_t1_between_hits_downstream_diff += t1_vec[ipair];
            
            if (ipair==0){
                _biggest_distance_between_hits_downstream_diff    = distance;
                _biggest_distance_t1_between_hits_downstream_diff = t1_vec[ipair];
            }else{
                _biggest_distance_between_hits_downstream_diff    -= distance;
                _biggest_distance_t1_between_hits_downstream_diff -= t1_vec[ipair];

                _biggest_distance_between_hits_downstream_diff    = std::abs(_biggest_distance_between_hits_downstream_diff);
                _biggest_distance_t1_between_hits_downstream_diff = std::abs(_biggest_distance_t1_between_hits_downstream_diff);
            }
        }
    }
    counter++;
  }
  _biggest_distance_between_hits_downstream_mean  = _biggest_distance_between_hits_downstream_mean/counter_biggest;
  _biggest_distance_t1_between_hits_downstream_diff  = _biggest_distance_t1_between_hits_downstream_diff/counter_biggest;

  _smallest_distance_between_hits_downstream_mean = _smallest_distance_between_hits_downstream_mean/counter_smallest;
  _smallest_distance_t1_between_hits_downstream_diff = _smallest_distance_t1_between_hits_downstream_diff/counter_smallest;

  // Calculate the mean and std of the CRT hit PE.
  calculateMeanStd(_chit_pes, _chit_pes_mean, _chit_pes_sample_std);

  // Calculate the mean and std of the CRT hit t1 difference.
  calculateMeanStd(_chit_t1_diff, _chit_t1_diff_mean, _chit_t1_diff_sample_std);

  _tree->Fill();
}

void SelectionAna::beginSubRun(art::SubRun const& sr) 
{
  _sr_run       = sr.run();
  _sr_subrun    = sr.subRun();
  _sr_begintime = sr.beginTime().value();
  _sr_endtime   = sr.endTime().value();

  art::Handle<sumdata::POTSummary> pot_handle;
  sr.getByLabel(_pot_label, pot_handle);

  if (pot_handle.isValid()) {
    _sr_pot = pot_handle->totpot;
    _sr_spills = pot_handle->totspills;
  } else {
    _sr_pot = 0.;
    _sr_spills = 0.;
  }

  std::cout << "POT for this subrun: " << _sr_pot << " (" << _sr_spills << " spills)" << std::endl;
  _sr_tree->Fill();
}

void SelectionAna::respondToOpenInputFile(const art::FileBlock& fb)
{
    _file_name = fb.fileName();
}

void SelectionAna::calculateMeanStd(std::vector<double> vec, double &mean, double &std){
  mean = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
  double accum = 0.0;
  std::for_each(vec.begin(), vec.end(), [&](const double d) {
      accum += (d - mean) * (d - mean);
  });
  std = sqrt(accum / (vec.size()-1));
}

bool SelectionAna::canFormSquare(std::vector<double> distances) {
    if (distances.size() != 6) {
        return false; // Need exactly 4 points to form a square
    }

    // Sort the distances in non-decreasing order
    std::sort(distances.begin(), distances.end());

    // Check if the distances satisfy square properties
    return distances[0] > 0 &&                    // Nonzero distance
       distances[0] == distances[1] &&            // Opposite sides equal
       distances[2] == distances[3] &&
       distances[4] == distances[5] &&           // Diagonals may or may not be equal
       distances[4] == std::sqrt(distances[0]*distances[0] + distances[2]*distances[2]);
}

DEFINE_ART_MODULE(SelectionAna)
