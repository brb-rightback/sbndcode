////////////////////////////////////////////////////////////////////////
// Class:       SBNDPMTDecoder
// Plugin Type: producer (Unknown Unknown)
// File:        SBNDPMTDecoder_module.cc
// Author:      Lynn Tung (lynnt@uchicago.edu)
//
// Generated at Wed Aug 23 12:28:48 2023 by Lynn Tung using cetskelgen
// from  version .
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbndaq-artdaq-core/Overlays/Common/CAENV1730Fragment.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include "art_root_io/TFileService.h"
#include "TH1D.h"
#include "TNtuple.h"

#include "lardataobj/RawData/OpDetWaveform.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <bitset>
#include <memory>

namespace sbndaq {
    class SBNDPMTDecoder;
}


class sbndaq::SBNDPMTDecoder : public art::EDProducer {
public:
    explicit SBNDPMTDecoder(fhicl::ParameterSet const& p);
    // The compiler-generated destructor is fine for non-base
    // classes without bare pointers or other resource use.

    // Plugins should not be copied or assigned.
    SBNDPMTDecoder(SBNDPMTDecoder const&) = delete;
    SBNDPMTDecoder(SBNDPMTDecoder&&) = delete;
    SBNDPMTDecoder& operator=(SBNDPMTDecoder const&) = delete;
    SBNDPMTDecoder& operator=(SBNDPMTDecoder&&) = delete;

    // Required functions.
    void produce(art::Event& e) override;
    bool check_fragments(std::vector<artdaq::Fragment> frag_v, uint32_t& trig_ttt, uint32_t& trig_len);
    void get_waveforms(artdaq::Fragment & frag, std::vector<std::vector<short>> & wvfm_v);
    
    std::vector<raw::OpDetWaveform> fWaveforms;
    std::vector<raw::OpDetWaveform> fTriggerWaveforms;

    std::vector<std::vector<artdaq::Fragment>>  trig_frag_v; // every entry should correspond to 1 [flash] trigger 

private:
    bool fverbose;
    std::string fch_instance_name;
    std::string ftr_instance_name;

    // histogram info  
    std::stringstream histname; //raw waveform hist name
    art::ServiceHandle<art::TFileService> tfs;
    uint evt_counter = 0;
};


sbndaq::SBNDPMTDecoder::SBNDPMTDecoder(fhicl::ParameterSet const& p)
    : EDProducer{p}  // ,
{
    fverbose          = p.get<bool>("Verbose",true);
    fch_instance_name = p.get<std::string>("pmtInstanceName","PMTChannels");
    ftr_instance_name = p.get<std::string>("ftrigInstanceName","FTrigChannels");

    produces< std::vector< raw::OpDetWaveform > >(fch_instance_name); 
    produces< std::vector< raw::OpDetWaveform > >(ftr_instance_name);
}

void sbndaq::SBNDPMTDecoder::produce(art::Event& evt)
{
    std::unique_ptr< std::vector< raw::OpDetWaveform > > wvfmVec(std::make_unique< std::vector< raw::OpDetWaveform > > ());
    std::unique_ptr< std::vector< raw::OpDetWaveform > > twvfmVec(std::make_unique< std::vector< raw::OpDetWaveform > > ());

    std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
    fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();

    evt_counter++;

    trig_frag_v.clear(); 
    // ! make the max number of flashes configurable?
    trig_frag_v.resize(30); // assume up to 30 flashes 
    for (auto &v : trig_frag_v)
        v.reserve(8);

    fWaveforms.clear();
    fTriggerWaveforms.clear();
    uint ncont = 0; // counter for number of containers

    for (auto handle : fragmentHandles){
        if (!handle.isValid() || handle->size() == 0){
            std::cout << "No CAEN V1730 fragments found." << std::endl;
            continue;
        }
        if (handle->front().type() == artdaq::Fragment::ContainerFragmentType){
            //  Pull mode!
            for (auto cont : *handle){
                artdaq::ContainerFragment contf(cont);
                ncont++;

                if (contf.fragment_type() == sbndaq::detail::FragmentType::CAENV1730){
                    if (ncont==1){ 
                        // use the size of the first container to re-initialize 
                        trig_frag_v.resize(contf.block_count());
                    }
                    for (size_t ii = 0; ii < contf.block_count(); ++ii){
                        auto frag = *contf[ii].get();
                        trig_frag_v[ii].push_back(frag);                        
                    }
                }
            }
        }
        else{
            if (handle->front().type() == sbndaq::detail::FragmentType::CAENV1730){
                //  Push mode! 
                for (auto frag : *handle){
                        if (frag.fragmentID() == 19) // ! hardcoded for now!
                            continue;
                }
            }
        }
    }
    auto ntrig = trig_frag_v.size();

    std::vector<uint32_t> trig_ttt_v(ntrig,0);
    std::vector<uint32_t> trig_len_v(ntrig,0);

    for (size_t i=0; i < trig_frag_v.size(); i++){
        // frag_v contains all fragments for a single trigger 
        auto frag_v = trig_frag_v.at(i);
        if (frag_v.empty()) continue;
        uint32_t trig_ttt = 0;
        uint32_t trig_len = 0;

        bool pass_check = check_fragments(frag_v, trig_ttt, trig_len);        
        if (pass_check == false)
            continue;
        // TODO: once we get the event trigger from other timing subystems,
        //       we can pass trigger based on the event trigger time
        //       instead of passing all triggers
        trig_ttt_v.at(i) = trig_ttt;
        trig_len_v.at(i) = trig_len;
        auto nfrag = frag_v.size();
        std::cout << "Trigger " << i << " has TTT=" << trig_ttt << " ns and nentries=" << trig_len  << " ticks in " << nfrag << " fragments." << std::endl;
    }

    for (size_t itrig=0; itrig < ntrig; itrig++){

        auto ittt = trig_ttt_v.at(itrig);
        auto ilen = trig_len_v.at(itrig);
        auto ifrag_v = trig_frag_v.at(itrig);

        std::vector<int> fragid_v(ifrag_v.size(),0);

        // if this waveform is short, skip it 
        // TODO: REPLACE 5000 WITH VARIABLE SET TO NSAMPLES FROM THE METADATA
        if (ilen < 5000) continue;

        std::vector<std::vector<short>> iwvfm_v;
        for (size_t idx=0; idx<ifrag_v.size(); idx++){
            auto frag = ifrag_v.at(idx);
            get_waveforms(frag, iwvfm_v);
            fragid_v.at(idx) = frag.fragmentID();
        }
        for (size_t jtrig=itrig+1; jtrig < ntrig; jtrig++){
            bool pass_checks = true;
            auto jttt = trig_ttt_v.at(jtrig);
            auto jlen = trig_len_v.at(jtrig);
            auto jfrag_v = trig_frag_v.at(jtrig);

            // if the next trigger is more than 10 us away or is 10 us long, stop looking
            if (((signed)(jttt - ittt) > 1e4) || (jlen >= 5000)) break; 
            else if (((jttt - ittt) < 1e4) && (jlen < 5000)){
                std::vector<std::vector<short>> jwvfm_v;
                for (size_t idx=0; idx<jfrag_v.size(); idx++){
                    auto frag = jfrag_v.at(idx);
                    if (fragid_v.at(idx) != frag.fragmentID()){
                        // check that the two fragments originated from the same board
                        std::cout << "Error: fragment IDs do not match between triggers " << itrig << " and " << jtrig << std::endl;
                        pass_checks=false;
                        break;
                    }
                    get_waveforms(frag, jwvfm_v);
                }
                // check that the number of waveforms are the same 
                if (jwvfm_v.size() != iwvfm_v.size()){
                    std::cout << "Error: number of channels in extended fragment " << jtrig << " does not match number of channels in original fragment " << itrig << std::endl;
                    pass_checks=false;
                }
                if (pass_checks==false) continue;
                // combine the waveforms
                for (size_t ich=0; ich < iwvfm_v.size(); ich++){
                    std::vector<short>& orig_wvfm = iwvfm_v.at(ich);
                    std::vector<short>  extd_wvfm = jwvfm_v.at(ich); // extended waveform 
                    orig_wvfm.insert( orig_wvfm.end(), extd_wvfm.begin(), extd_wvfm.end());
                }
            } // extended trigger found 
        } // loop over subsequent triggers

        std::cout << "Obtained waveforms for TTT at " << ittt << "\n" 
                    << "\tNumber of channels: " << iwvfm_v.size() << "\n"
                    << "\tNumber of entries: " << iwvfm_v.at(0).size() << std::endl;

        // histo: save waveforms section for combined waveforms
        // only save from the first event in every file
        if (evt_counter==1){
            for (size_t i = 0; i < iwvfm_v.size(); i++){
                auto combined_wvfm = iwvfm_v[i];
                histname.str(std::string());
                histname << "evt_" << evt.event() << "_ch_" << i << "_combined_wvfm";

                TH1D *wvfmHist = tfs->make< TH1D >(histname.str().c_str(), histname.str().c_str(), combined_wvfm.size(), 0, combined_wvfm.size());
                wvfmHist->GetXaxis()->SetTitle("ticks");
                for(unsigned int n = 0; n < combined_wvfm.size(); n++) 
                    wvfmHist->SetBinContent(n + 1, (double)combined_wvfm[n]);
            }
        } // histo section 
    } // loop over triggers 



    if (fWaveforms.empty() == false){
        std::cout << "Number of PMT waveforms: " << fWaveforms.size() << std::endl;
        std::cout << "Number of trigger waveforms: " << fTriggerWaveforms.size() << std::endl;
        for (const raw::OpDetWaveform &waveform : fWaveforms)
            wvfmVec->push_back(waveform);
        for (const raw::OpDetWaveform &waveform : fTriggerWaveforms)
            twvfmVec->push_back(waveform);
    }
    else 
        std::cout << "pushing empty waveform vec!" << std::endl;
    
    evt.put(std::move(wvfmVec),fch_instance_name);  
    evt.put(std::move(twvfmVec),ftr_instance_name);
}

bool sbndaq::SBNDPMTDecoder::check_fragments(std::vector<artdaq::Fragment> frag_v, uint32_t& trig_ttt, uint32_t& trig_len){
    bool pass = true;
    uint32_t this_ttt = 0; 
    uint32_t this_len = 0;
    for (size_t ifrag=0; ifrag<frag_v.size(); ifrag++){
        CAENV1730Fragment bb(frag_v.at(ifrag));
        CAENV1730Event const* event_ptr = bb.Event();
        auto const * md = bb.Metadata();
        CAENV1730EventHeader header = event_ptr->Header;

        uint32_t ttt = header.triggerTimeTag*8;
        auto nch = md->nChannels;    
        uint32_t ev_size_quad_bytes = header.eventSize;
        uint32_t evt_header_size_quad_bytes = sizeof(CAENV1730EventHeader)/sizeof(uint32_t);
        uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
        uint32_t wvfm_length = data_size_double_bytes/nch; // downsampled trigger time tag; TTT points to end of wvfm w.r.t. pps
        
        if (ifrag==0){ 
            this_ttt = ttt; 
            this_len = wvfm_length; 
        }
        if (this_ttt != ttt || this_len != wvfm_length){
            // std::cout << "Fragment " << fragID << " from board " << boardID << " has length/TTT mismatch" << std::endl;
            // std::cout << "expected TTT vs. actual TTT: " << this_ttt << " vs. " << ttt << std::endl;
            // std::cout << "expected length vs. actual length: " << this_len << " vs. " << wvfm_length << std::endl;
            // int = this_ttt - ttt; 
            if (abs((signed)(this_ttt - ttt)) > 16 || abs((signed)(this_len - wvfm_length)) > 8 ){
                std::cout << "Mismatch is greater than maximum 16 ns jitter" << std::endl;
                pass=false;
                break;
            }
            // else{
            //     std::cout << "Mismatch is within 16 ns jitter." << std::endl;
            // }
        }
    }
    trig_ttt = this_ttt;
    trig_len = this_len;
    return pass;
}

void sbndaq::SBNDPMTDecoder::get_waveforms(artdaq::Fragment & frag, std::vector<std::vector<short>> & wvfm_v){

    CAENV1730Fragment bb(frag);
    auto const* md = bb.Metadata();
    auto nch = md->nChannels;    

    CAENV1730Event const* event_ptr = bb.Event();
    CAENV1730EventHeader header = event_ptr->Header;
    uint32_t ev_size_quad_bytes = header.eventSize;
    uint32_t evt_header_size_quad_bytes = sizeof(CAENV1730EventHeader)/sizeof(uint32_t);
    uint32_t data_size_double_bytes = 2*(ev_size_quad_bytes - evt_header_size_quad_bytes);
    uint32_t wvfm_length = data_size_double_bytes/nch;

    const uint16_t* data_begin = reinterpret_cast<const uint16_t*>(frag.dataBeginBytes() 
								   + sizeof(CAENV1730EventHeader));
    const uint16_t* value_ptr =  data_begin;
    uint16_t value = 0;
    size_t ch_offset = 0;

    //--loop over channels
    for (size_t i_ch=0; i_ch<nch; ++i_ch){
        ch_offset = (size_t)(i_ch * wvfm_length);      
        std::vector<short> wvfm(wvfm_length,0);
        //--loop over waveform entries
        for(size_t i_t=0; i_t<wvfm_length; ++i_t){ 
            value_ptr = data_begin + ch_offset + i_t; /*pointer arithmetic*/
            value = *(value_ptr);

            wvfm.at(i_t) = value;
        }
        wvfm_v.push_back(wvfm);
    }
}

DEFINE_ART_MODULE(sbndaq::SBNDPMTDecoder)
