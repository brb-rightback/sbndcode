#ifndef SBNDCODE_DECODERS_CHANNELMAPPING_SBNDCHANNELMAPPROVIDER_H
#define SBNDCODE_DECODERS_CHANNELMAPPING_SBNDCHANNELMAPPROVIDER_H


// SBND libraries
#include "sbndcode/Decoders/ChannelMapping/ISBNDChannelMap.h"
#include "sbndcode/Decoders/ChannelMapping/IChannelMapping.h"

// framework libraries
#include "fhiclcpp/ParameterSet.h"
#include "cetlib_except/exception.h"

// C/C++ standard libraries
#include <string>
#include <memory> // std::unique_ptr<>


// -----------------------------------------------------------------------------
namespace sbndDB { class SBNDChannelMapProvider; }
class sbndDB::SBNDChannelMapProvider: public ISBNDChannelMap
{
public:
    
    // Constructor, destructor.
    SBNDChannelMapProvider(const fhicl::ParameterSet& pset);
    
    // Section to access fragment to board mapping
    bool                                    hasFragmentID(const unsigned int)       const override;

    /// Returns the number of TPC fragment IDs known to the service.
    unsigned int                            nTPCfragmentIDs() const override;
    const std::string&                      getCrateName(const unsigned int)        const override;
    const ReadoutIDVec&                     getReadoutBoardVec(const unsigned int)  const override;
    const TPCReadoutBoardToChannelMap&      getReadoutBoardToChannelMap()           const override;

    // Section to access channel information for a given board
    bool                                    hasBoardID(const unsigned int)          const override;

    /// Returns the number of TPC board IDs known to the service.
    unsigned int                            nTPCboardIDs() const override;
    unsigned int                            getBoardSlot(const unsigned int)        const override;
    const ChannelPlanePairVec&              getChannelPlanePair(const unsigned int) const override;

    // Section for PMT channel mapping
    bool                                    hasPMTDigitizerID(const unsigned int)   const override;

    /// Returns the number of PMT fragment IDs known to the service.
    unsigned int                            nPMTfragmentIDs() const override;
    const DigitizerChannelChannelIDPairVec& getChannelIDPairVec(const unsigned int) const override;

    /// Returns the channel mapping database key for the specified PMT fragment ID.
    static constexpr unsigned int PMTfragmentIDtoDBkey(unsigned int fragmentID);
    
    /// Returns the PMT fragment ID for the specified channel mapping database key.
    static constexpr unsigned int DBkeyToPMTfragmentID(unsigned int DBkey);

private:
    
    bool fDiagnosticOutput;
      
    IChannelMapping::TPCFragmentIDToReadoutIDMap   fFragmentToReadoutMap;
      
    IChannelMapping::TPCReadoutBoardToChannelMap   fReadoutBoardToChannelMap;

    IChannelMapping::FragmentToDigitizerChannelMap fFragmentToDigitizerMap; 

    std::unique_ptr<IChannelMapping>               fChannelMappingTool;

    /// Returns the list of board channel-to-PMT channel ID mapping within the specified fragment.
    /// @returns a pointer to the mapping list, or `nullptr` if invalid fragment
    DigitizerChannelChannelIDPairVec const* findPMTfragmentEntry
      (unsigned int fragmentID) const;

}; // sbndDB::SBNDChannelMapProvider


// -----------------------------------------------------------------------------

#endif // SBNDCODE_DECODERS_CHANNELMAPPING_SBNDCHANNELMAPPROVIDER_H
