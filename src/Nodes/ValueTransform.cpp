/* VariousStuff_Plugin - for licensing and copyright see license.txt */

#include <StdAfx.h>
#include <CPluginVariousStuff.h>
#include <Nodes/G2FlowBaseNode.h>

#include <Game.h>

namespace VariousStuffPlugin
{
    class CFlowValueTransformNode :
        public CFlowBaseNode<eNCT_Instanced>
    {
        private:
            enum EInputPorts
            {
                EIP_VALUE = 0,
                EIP_OLDMIN,
                EIP_OLDMAX,
                EIP_NEWMIN,
                EIP_NEWMAX,
            };

            enum EOutputPorts
            {
                EOP_VALUE = 0,
            };

        public:
            CFlowValueTransformNode( SActivationInfo* pActInfo )
            {
            }

            virtual void GetMemoryUsage( ICrySizer* s ) const
            {
                s->Add( *this );
            }

            virtual IFlowNodePtr Clone( SActivationInfo* pActInfo )
            {
                return new CFlowValueTransformNode( pActInfo );
            }

            virtual ~CFlowValueTransformNode()
            {

            }

            virtual void GetConfiguration( SFlowNodeConfig& config )
            {
                static const SInputPortConfig inputs[] =
                {
                    InputPortConfig<float>( "Value", 0, _HELP( "Value" ) ),
                    InputPortConfig<float>( "OldMin", 0, _HELP( "Value" ) ),
                    InputPortConfig<float>( "OldMax", 0, _HELP( "Value" ) ),
                    InputPortConfig<float>( "NewMin", 0, _HELP( "Value" ) ),
                    InputPortConfig<float>( "NewMax", 0, _HELP( "Value" ) ),
                    InputPortConfig_Null(),
                };

                static const SOutputPortConfig outputs[] =
                {
                    OutputPortConfig<float>( "Value", _HELP( "Value" ) ),
                    OutputPortConfig_Null(),
                };

                config.pInputPorts = inputs;
                config.pOutputPorts = outputs;
                config.sDescription = _HELP( "Clamps and scales an input value" );

                config.SetCategory( EFLN_APPROVED );
            }

            virtual void ProcessEvent( EFlowEvent evt, SActivationInfo* pActInfo )
            {
                switch ( evt )
                {
                    case eFE_Suspend:
                        break;

                    case eFE_Resume:
                        break;

                    case eFE_Activate:
                        if ( IsPortActive( pActInfo, EIP_VALUE ) )
                        {
                            float fValue = GetPortFloat( pActInfo, EIP_VALUE );
                            float fOldMin = GetPortFloat( pActInfo, EIP_OLDMIN );
                            float fOldMax = GetPortFloat( pActInfo, EIP_OLDMAX );
                            float fNewMin = GetPortFloat( pActInfo, EIP_NEWMIN );
                            float fNewMax = GetPortFloat( pActInfo, EIP_NEWMAX );

                            fValue = CLAMP( fValue, fOldMin, fOldMax );

                            ActivateOutput<float>( pActInfo, EOP_VALUE, float( ( fValue - fOldMin ) / ( fOldMax - fOldMin ) * ( fNewMax - fNewMin ) + fNewMin ) );
                        }

                        break;

                    case eFE_SetEntityId:
                        //m_pEntity = pActInfo->pEntity;
                        break;

                    case eFE_Update:
                        break;
                }
            }
    };
}

REGISTER_FLOW_NODE_EX( "VariousStuff:ValueTransform", VariousStuffPlugin::CFlowValueTransformNode, CFlowValueTransformNode );