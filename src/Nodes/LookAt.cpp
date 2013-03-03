/* VariousStuff_Plugin - for licensing and copyright see license.txt */
//  - 2/17/13 : Plugin SDK Port - Hendrik

// Copyright (C), RenEvo Software & Designs, 2008
//  - 7/26/08 : File created - KAK
// http://fgps.sourceforge.net/

#include <StdAfx.h>
#include <CPluginVariousStuff.h>
#include <Nodes/G2FlowBaseNode.h>

#include <IEntitySystem.h>
#include <IActorSystem.h>
#include <IMovementController.h>

namespace VariousStuffPlugin
{
    class CFlowNode_LookAtEntity :
        public CFlowBaseNode<eNCT_Instanced>
    {
            enum EInputPorts
            {
                EIP_Look,
                EIP_Stop,
                EIP_Target,
                EIP_Speed,
                EIP_Constant,
            };

            IEntity* m_pTarget;
            float m_fSlerpValue;
            float m_fSlerpSpeed;
            bool m_bConstantLook;

            Vec3 m_lastTargetPos;
            Vec3 m_lastEntityPos;
            Quat m_oldRot;
            Quat m_reqQuat;

        public:
            CFlowNode_LookAtEntity( SActivationInfo* pActInfo )
            {
                m_pTarget = NULL;
                m_fSlerpValue = 0.0f;
                m_fSlerpSpeed = 0.0f;
                m_bConstantLook = false;

                m_lastTargetPos.Set( 0.f, 0.f, 0.f );
                m_lastEntityPos.Set( 0.f, 0.f, 0.f );
                m_oldRot.SetIdentity();
                m_reqQuat.SetIdentity();
            }

            virtual ~CFlowNode_LookAtEntity( void )
            {

            }

            virtual void Serialize( SActivationInfo* pActInfo, TSerialize ser )
            {
                ser.Value( "m_fSlerpValue", m_fSlerpValue );
                ser.Value( "m_lastTargetPos", m_lastTargetPos );
                ser.Value( "m_lastEntityPos", m_lastEntityPos );
                ser.Value( "m_oldRot", m_oldRot );
                ser.Value( "m_reqQuat", m_reqQuat );
            }

            virtual void GetConfiguration( SFlowNodeConfig& config )
            {
                static const SInputPortConfig inputs[] =
                {
                    InputPortConfig_Void( "Look", _HELP( "Instruct the entity to look at the target" ) ),
                    InputPortConfig_Void( "Stop", _HELP( "Instruct the entity to stop looking at the target" ) ),
                    InputPortConfig<EntityId>( "Target", _HELP( "Target entity to look at" ) ),
                    InputPortConfig<float>( "Speed", 0.0f, _HELP( "How fast to rotate (0 for instant)" ) ),
                    InputPortConfig<bool>( "Constant", false, _HELP( "Set if entity should constantly keep looking at the target" ) ),
                    InputPortConfig_Null(),
                };

                static const SOutputPortConfig outputs[] =
                {
                    OutputPortConfig_Null(),
                };

                // Fill in configuration
                config.pInputPorts = inputs;
                config.pOutputPorts = outputs;
                config.sDescription = _HELP( "Orients an entity to look at another" );
                config.SetCategory( EFLN_APPROVED );
                config.nFlags |= EFLN_TARGET_ENTITY;
            }

            virtual void ProcessEvent( EFlowEvent event, SActivationInfo* pActInfo )
            {
                switch ( event )
                {
                    case eFE_Initialize:
                        {
                            m_pTarget = NULL;
                            m_fSlerpValue = 0.0f;
                            m_fSlerpSpeed = 0.0f;
                            m_bConstantLook = false;

                            m_lastTargetPos.Set( 0.f, 0.f, 0.f );
                            m_lastEntityPos.Set( 0.f, 0.f, 0.f );
                            m_oldRot.SetIdentity();
                            m_reqQuat.SetIdentity();
                        }
                        break;

                    case eFE_Activate:
                        {
                            if ( IsPortActive( pActInfo, EIP_Constant ) )
                            {
                                m_bConstantLook = GetPortBool( pActInfo, EIP_Constant );
                            }

                            if ( IsPortActive( pActInfo, EIP_Target ) )
                            {
                                const EntityId targetId = GetPortEntityId( pActInfo, EIP_Target );
                                m_pTarget = gEnv->pEntitySystem->GetEntity( targetId );
                            }

                            if ( IsPortActive( pActInfo, EIP_Speed ) )
                            {
                                m_fSlerpSpeed = MAX( GetPortFloat( pActInfo, EIP_Speed ), 0.f );
                            }

                            if ( IsPortActive( pActInfo, EIP_Look ) )
                            {
                                m_bConstantLook = GetPortBool( pActInfo, EIP_Constant );
                                const EntityId targetId = GetPortEntityId( pActInfo, EIP_Target );
                                m_pTarget = gEnv->pEntitySystem->GetEntity( targetId );
                                m_fSlerpSpeed = MAX( GetPortFloat( pActInfo, EIP_Speed ), 0.f );

                                m_lastTargetPos.Set( 0.f, 0.f, 0.f );
                                m_lastEntityPos.Set( 0.f, 0.f, 0.f );
                                m_oldRot.SetIdentity();
                                m_reqQuat.SetIdentity();

                                if ( LookAt( pActInfo->pEntity, m_pTarget ) )
                                {
                                    if ( m_bConstantLook )
                                    {
                                        pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, true );
                                    }
                                }
                            }

                            if ( IsPortActive( pActInfo, EIP_Stop ) )
                            {
                                m_bConstantLook = false;
                                pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, false );
                            }
                        }
                        break;

                    case eFE_Update:
                        {
                            if ( !m_bConstantLook )
                            {
                                pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, false );
                            }

                            else
                            {
                                LookAt( pActInfo->pEntity, m_pTarget );
                            }
                        }
                        break;
                }
            }

            virtual void GetMemoryUsage( ICrySizer* s ) const
            {
                s->Add( *this );
            }

            virtual IFlowNodePtr Clone( SActivationInfo* pActInfo )
            {
                return new CFlowNode_LookAtEntity( pActInfo );
            }

            bool LookAt( IEntity* pEntity, IEntity* pTarget )
            {
                bool bResult = false;

                if ( pEntity && pTarget )
                {
                    // Check old pos
                    const Vec3 vEntityPos = pEntity->GetWorldPos();
                    const Vec3 vTargetPos = pTarget->GetWorldPos();

                    if ( vTargetPos != m_lastTargetPos || vEntityPos != m_lastEntityPos )
                    {
                        m_lastTargetPos = vTargetPos;
                        m_lastEntityPos = vEntityPos;

                        if ( m_fSlerpSpeed <= 0.f )
                        {
                            m_fSlerpValue = 1.f;
                        }

                        else
                        {
                            m_fSlerpValue = 0.f;
                        }

                        // Use center bounds for forward calc
                        AABB targetBounds;
                        pTarget->GetWorldBounds( targetBounds );
                        Vec3 vTargetLookPos = targetBounds.GetCenter();

                        // If entity is actor, use eye position instead!
                        Vec3 vEntityLookPos( 0.f, 0.f, 0.f );
                        bool bIsEntityActor = false;
                        IActorSystem* pActSys = gEnv->pGameFramework->GetIActorSystem();

                        if ( pActSys )
                        {
                            IActor* pActor = pActSys->GetActor( pEntity->GetId() );

                            if ( pActor )
                            {
                                bIsEntityActor = true;

                                SMovementState state;
                                pActor->GetMovementController()->GetMovementState( state );
                                vEntityLookPos = state.eyePosition;
                            }
                        }

                        if ( !bIsEntityActor )
                        {
                            AABB entityBounds;
                            pEntity->GetWorldBounds( entityBounds );
                            vEntityLookPos = entityBounds.GetCenter();
                        }

                        const Vec3 forward = ( vTargetLookPos - vEntityLookPos ).GetNormalizedSafe( Vec3Constants<float>::fVec3_OneY );
                        const Vec3 right = ( forward.Cross( Vec3Constants<float>::fVec3_OneZ ) ).GetNormalizedSafe( Vec3Constants<float>::fVec3_OneX );

                        Matrix34 m;
                        m.SetFromVectors( right, forward, Vec3Constants<float>::fVec3_Zero, Vec3Constants<float>::fVec3_Zero );
                        m.OrthonormalizeFast();

                        m_oldRot = pEntity->GetRotation();
                        m_reqQuat = Quat( m );
                    }

                    if ( m_fSlerpValue <= 1.f )
                    {
                        // Increase slerp
                        if ( m_fSlerpSpeed > 0.f )
                        {
                            if ( m_bConstantLook )
                            {
                                m_fSlerpValue += m_fSlerpSpeed * gEnv->pTimer->GetFrameTime();
                                m_fSlerpValue = CLAMP( m_fSlerpValue, 0.f, 1.f );
                            }

                            else
                            {
                                // Speed is used as ratio away when non-constant
                                m_fSlerpValue = CLAMP( m_fSlerpSpeed, 0.f, 1.f );
                            }
                        }

                        // Slerp to new orientation
                        Quat qNew;
                        qNew.SetSlerp( m_oldRot, m_reqQuat, m_fSlerpValue );
                        pEntity->SetRotation( qNew );

                        bResult = true;
                    }
                }

                return bResult;
            }
    };
}

REGISTER_FLOW_NODE_EX( "VariousStuff:LookAtEntity", VariousStuffPlugin::CFlowNode_LookAtEntity, CFlowNode_LookAtEntity );