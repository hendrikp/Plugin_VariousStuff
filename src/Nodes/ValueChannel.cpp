/* VariousStuff_Plugin - for licensing and copyright see license.txt */

// Idea based on FGPS Signaler

#include <StdAfx.h>
#include <Nodes/G2FlowBaseNode.h>

#include <map>
#include <list>
#include <algorithm>

#include <CPluginVariousStuff.h>

#define MAXRECURSIONDEPTH 20 //!< Maximal recursion depth (safety measure, currently handled by cryengine anyways)

namespace VariousStuffPlugin
{
    /**
    * @brief Data type to hold full node address
    */
    struct SFlowAddressInfo
    {
        TFlowGraphId nGraphId; //!< Flowgraph Id this Flownode belongs to
        SFlowAddress faAddress; //!< Flownode Address inside the graph

        /**
        * @brief Data type to hold full node address
        * @param id Flowgraph Id this Flownode belongs to
        * @param address Flownode Address inside the graph
        */
        SFlowAddressInfo( TFlowGraphId id, SFlowAddress& address ) :
            nGraphId( id ),
            faAddress( address )
        { };
    };

    static int gValueChannelRecursionDepth = 0; //!< Current recursion depth (safety measure, currently handled by cryengine anyways)

    /**
    * @brief Value Channel Flownode class
    * @tparam tValueType Value Type (needs to be supported by Flowsystem)
    */
    template <typename tValueType>
    class CValueChannelNode :
        public CFlowBaseNode<eNCT_Instanced>
    {
        private:
            static std::map< string, std::list< CValueChannelNode<tValueType>* > > m_Channels; //!< Channel registry for each datatype
            string m_sCurrentChannel; //!< Current channel of this node
            SFlowAddressInfo m_Address; //!< Current Flowsystem Address of this node

            enum EInputPorts
            {
                EIP_CHANNEL = 0, //!< Channelname
                EIP_VALUE, //!< Input value
            };

            enum EOutputPorts
            {
                EOP_VALUE = 0, //!< Output value
            };

            /**
            * @brief Changes the Channel this Node belongs to
            * @param sNewChannel New Channel
            */
            void ChangeChannel( string sNewChannel )
            {
                // Channel changes
                if ( m_sCurrentChannel != sNewChannel )
                {
                    // Find Channel
                    auto elemOldChannel = m_Channels.find( m_sCurrentChannel );

                    // First remove from old channel
                    if ( elemOldChannel != m_Channels.end() )
                    {
                        // Find current node
                        auto elemOldAddress = std::find( elemOldChannel->second.begin(), elemOldChannel->second.end(), this );

                        // Remove it
                        // Info: If a transmitter is changing the channel iterators will become invalid
                        //       (this is handled by CryEngine by delaying port activations until after ProcessEvent)
                        elemOldChannel->second.erase( elemOldAddress );
                    }

                    // Add to new channel
                    if ( !sNewChannel.empty() )
                    {
                        m_Channels[sNewChannel].push_back( this );
                    }

                    // Set new channel name
                    m_sCurrentChannel = sNewChannel;
                }
            }

        public:
            CValueChannelNode( SActivationInfo* pActInfo )
                : m_Address( InvalidFlowGraphId, SFlowAddress( InvalidFlowNodeId, EOP_VALUE, true ) )
            {
            }

            virtual void GetMemoryUsage( ICrySizer* s ) const
            {
                s->Add( *this );
            }

            virtual IFlowNodePtr Clone( SActivationInfo* pActInfo )
            {
                return new CValueChannelNode( pActInfo );
            }

            /**
            * @brief Destructor
            * Cleans up bound channel
            */
            virtual ~CValueChannelNode()
            {
                ChangeChannel( "" );
            }

            /**
            * @brief Configuration retrieval
            */
            virtual void GetConfiguration( SFlowNodeConfig& config )
            {
                static const SInputPortConfig inputs[] =
                {
                    InputPortConfig<string>( "Channel", _HELP( "Identifier of the Channel to use" ) ),
                    InputPortConfig<tValueType>( "Value", _HELP( "Value to transmit" ) ),
                    InputPortConfig_Null(),
                };

                static const SOutputPortConfig outputs[] =
                {
                    OutputPortConfig<tValueType>( "Value", _HELP( "Value received" ) ),
                    OutputPortConfig_Null(),
                };

                config.pInputPorts = inputs;
                config.pOutputPorts = outputs;
                config.sDescription = _HELP( "Transmit/Receives values through a channel between multiple flowgraphs" );

                config.SetCategory( EFLN_APPROVED );
            }

            /**
            * @brief Process Node Events
            * Handles initialization, transmitting and receiving
            */
            virtual void ProcessEvent( EFlowEvent evt, SActivationInfo* pActInfo )
            {
                m_Address.nGraphId = pActInfo->pGraph->GetGraphId();
                m_Address.faAddress.node = pActInfo->myID;

                switch ( evt )
                {
                    case eFE_Initialize:
                        // Initialize channel name
                        ChangeChannel( GetPortString( pActInfo, EIP_CHANNEL ) );
                        break;

                    case eFE_Activate:

                        // Change channel name
                        if ( IsPortActive( pActInfo, EIP_CHANNEL ) )
                        {
                            ChangeChannel( GetPortString( pActInfo, EIP_CHANNEL ) );
                        }

                        // If value changes and we haven't reached the maximal recursion depth
                        if ( IsPortActive( pActInfo, EIP_VALUE ) && gValueChannelRecursionDepth < MAXRECURSIONDEPTH )
                        {
                            const TFlowInputData& vValue = GetPortAny( pActInfo, EIP_VALUE );

                            auto elemChannel = m_Channels.find( m_sCurrentChannel );

                            // Found channel (should be the case every time..)
                            if ( elemChannel != m_Channels.end() )
                            {
                                ++gValueChannelRecursionDepth;

                                // Now Transmit to all nodes bound to this channel
                                for ( auto iter = elemChannel->second.begin(); iter != elemChannel->second.end(); ++iter )
                                {
                                    IFlowGraph* pGraph = gEnv->pFlowSystem->GetGraphById( ( *iter )->m_Address.nGraphId );

                                    if ( pGraph )
                                    {
                                        // Send the value
                                        pGraph->ActivatePortAny( ( *iter )->m_Address.faAddress, vValue );
                                    }
                                }

                                --gValueChannelRecursionDepth;
                            }
                        }

                        break;
                }
            }
    };
}

/**
* @brief Creates a new ValueChannel node type
* @param valtype C-Datatype
* @param tname Human name
*/
#define CREATE_VALUECHANNELTYPE(valtype, tname) \
    std::map<string, std::list< VariousStuffPlugin::CValueChannelNode<valtype>* >> VariousStuffPlugin::CValueChannelNode<valtype>::m_Channels;\
    REGISTER_FLOW_NODE_EX( "VariousStuff:ValueChannel:" tname, VariousStuffPlugin::CValueChannelNode<valtype>, CValueChannelNode_ ## valtype );

// Create a node for each type supported by the Flowsystem
CREATE_VALUECHANNELTYPE( int , "Int" );
CREATE_VALUECHANNELTYPE( float, "Float" );
CREATE_VALUECHANNELTYPE( string, "String" );
CREATE_VALUECHANNELTYPE( SFlowSystemVoid, "Void" );
CREATE_VALUECHANNELTYPE( Vec3, "Vec3" );
CREATE_VALUECHANNELTYPE( bool, "Bool" );
CREATE_VALUECHANNELTYPE( EntityId, "EntityId" );
