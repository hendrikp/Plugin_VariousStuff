/* VariousStuff_Plugin - for licensing and copyright see license.txt */
//  - 3/2/13 : Plugin SDK Port - Hendrik

// Copyright (C), Russell Lowe, 2012
// http://www.crydev.net/viewtopic.php?f=311&t=82303

#include <StdAfx.h>
#include <CPluginVariousStuff.h>
#include <Nodes/G2FlowBaseNode.h>
#include <Game.h>
#include <IRenderAuxGeom.h>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <sstream> //for string stuff

#pragma comment(lib, "Ws2_32.lib")
//#pragma comment(lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 8024

namespace TCPClientPlugin
{
    class CFlowTCPCommunicationNode :
        public CFlowBaseNode<eNCT_Instanced>
    {
        WSADATA wsaData;
        char recvbuf[DEFAULT_BUFLEN];

#pragma pack(1)
        struct joint
        {
            double mT[3];
            double mR[3];
            double mConfidence;
        };
#pragma pack()

        SOCKET ConnectSocket;
        int active;
        struct addrinfo* result, *ptr, hints;


        // -------------------------------------------------------------------------------------------------------------------------------
        // DEFINE INPUT PORTS
        // -------------------------------------------------------------------------------------------------------------------------------
        typedef enum
        {
            PORT_IN_ACTIVE,
            PORT_IN_PORT,
            PORT_IN_IP,
        } InputPorts;


        // -------------------------------------------------------------------------------------------------------------------------------
        // DEFINE OUTPUT PORTS
        // -------------------------------------------------------------------------------------------------------------------------------
        typedef enum
        {
            PORT_OUT_WAIST_POS,
            PORT_OUT_WAIST_ROT,
            PORT_OUT_TORSO_POS,
            PORT_OUT_TORSO_ROT,
            PORT_OUT_NECK_POS,
            PORT_OUT_NECK_ROT,
            PORT_OUT_HEAD_POS,
            PORT_OUT_HEAD_ROT,
            PORT_OUT_SHOULDER_R_POS,
            PORT_OUT_SHOULDER_R_ROT,
            PORT_OUT_ELBOW_R_POS,
            PORT_OUT_ELBOW_R_ROT,
            PORT_OUT_HAND_R_POS,
            PORT_OUT_HAND_R_ROT,
            PORT_OUT_SHOULDER_L_POS,
            PORT_OUT_SHOULDER_L_ROT,
            PORT_OUT_ELBOW_L_POS,
            PORT_OUT_ELBOW_L_ROT,
            PORT_OUT_HAND_L_POS,
            PORT_OUT_HAND_L_ROT,
            PORT_OUT_HIP_R_POS,
            PORT_OUT_HIP_R_ROT,
            PORT_OUT_KNEE_R_POS,
            PORT_OUT_KNEE_R_ROT,
            PORT_OUT_FOOT_R_POS,
            PORT_OUT_FOOT_R_ROT,
            PORT_OUT_HIP_L_POS,
            PORT_OUT_HIP_L_ROT,
            PORT_OUT_KNEE_L_POS,
            PORT_OUT_KNEE_L_ROT,
            PORT_OUT_FOOT_L_POS,
            PORT_OUT_FOOT_L_ROT,

            PORT_OUT_DEBUG,
        } OutputPorts;



    public:
        virtual IFlowNodePtr Clone( SActivationInfo* pActInfo )
        {
            return new CFlowTCPCommunicationNode( pActInfo );
        }

        // -------------------------------------------------------------------------------------------------------------------------------
        CFlowTCPCommunicationNode( SActivationInfo* pActInfo )
        {
            ConnectSocket = INVALID_SOCKET;
            result = NULL;
            ptr = NULL;
            active = 0;
        }

        // -------------------------------------------------------------------------------------------------------------------------------
        ~CFlowTCPCommunicationNode()
        {

            freeaddrinfo( result );
            closesocket( ConnectSocket );
            WSACleanup();
        }

        // -------------------------------------------------------------------------------------------------------------------------------
        virtual void GetConfiguration( SFlowNodeConfig& config )
        {
            // ACHTUNG! Do not include spaces in the port name! Flow graph editor will be unable to save/load correctly due to spaces when xml paring
            static const SInputPortConfig inputs[] =
            {
                InputPortConfig <int>( "ACTIVE", _HELP( "ACTIVE" ) ),
                InputPortConfig <string>( "PORT", "4444", _HELP( "Port to be opened" ) ),
                InputPortConfig <string>( "IP", "127.0.0.1", _HELP( "IP address" ) ),
                InputPortConfig_Null(),
            };

            static const SOutputPortConfig outputs[] =
            {
                OutputPortConfig <Vec3>( "WAIST_POS", _HELP( "Waist Position relative to Kinect" ) ),
                OutputPortConfig <Vec3>( "WAIST_ROT", _HELP( "Waist Rotation" ) ),
                OutputPortConfig <Vec3>( "TORSO_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "TORSO_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "NECK_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "NECK_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HEAD_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HEAD_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "SHOULDER_R_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "SHOULDER_R_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "ELBOW_R_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "ELBOW_R_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HAND_R_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HAND_R_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "SHOULDER_L_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "SHOULDER_L_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "ELBOW_L_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "ELBOW_L_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HAND_L_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HAND_L_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HIP_R_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HIP_R_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "KNEE_R_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "KNEE_R_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "FOOT_R_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "FOOT_R_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HIP_L_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "HIP_L_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "KNEE_L_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "KNEE_L_ROT", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "FOOT_L_POS", _HELP( "Received Data" ) ),
                OutputPortConfig <Vec3>( "FOOT_L_ROT", _HELP( "Received Data" ) ),

                OutputPortConfig <string>( "DEBUG", _HELP( "Debug Messages" ) ),
                OutputPortConfig_Null(),
            };

            config.pInputPorts = inputs;
            config.pOutputPorts = outputs;
            config.sDescription = _HELP( "TCP Server" );
            config.SetCategory( EFLN_APPROVED );

        }

        // -------------------------------------------------------------------------------------------------------------------------------
        // CALLED ON EVENT TRIGGER, ACTIVATE OR UPDATE
        // -------------------------------------------------------------------------------------------------------------------------------
        virtual void ProcessEvent( EFlowEvent event, SActivationInfo* pActInfo )
        {

            switch ( event )
            {

            case eFE_Update:
                OnUpdate( pActInfo );
                break;

            case eFE_Activate:
                OnActivate( pActInfo );
                break;

            }
        }



        // -------------------------------------------------------------------------------------------------------------------------------
        // ON NODE TRIGGER - DETERMINE WHICH PORT WAS SENT DATA
        // -------------------------------------------------------------------------------------------------------------------------------
        void OnActivate( SActivationInfo* pActInfo )
        {
            IFlowGraph* pGraph = pActInfo->pGraph;
            TFlowNodeId nodeId = pActInfo->myID;


            if ( IsPortActive( pActInfo, PORT_IN_ACTIVE ) )
            {


                if ( active == 0 )
                {
                    active = 1;

                    // SETUP SOCKET
                    string port = GetPortString( pActInfo, PORT_IN_PORT );
                    string ip = GetPortString( pActInfo, PORT_IN_IP );

                    int iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData ); // this wasn't nessasary for the server??

                    if ( iResult != 0 )
                    {
                        string debug = "fail at WSAStartup";
                        ActivateOutput( pActInfo, PORT_OUT_DEBUG, debug );
                        return;
                    }

                    ZeroMemory( &hints, sizeof( hints ) );

                    hints.ai_family   = AF_UNSPEC;
                    hints.ai_socktype = SOCK_STREAM;
                    hints.ai_protocol = IPPROTO_TCP;


                    iResult = getaddrinfo( ip, port, &hints, &result );

                    if ( iResult != 0 )
                    {
                        string debug = "fail at getaddrinfo";
                        ActivateOutput( pActInfo, PORT_OUT_DEBUG, debug );
                        WSACleanup();

                        return;
                    }

                    ConnectSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );

                    if ( ConnectSocket == INVALID_SOCKET )
                    {
                        string debug = "fail at socket";
                        ActivateOutput( pActInfo, PORT_OUT_DEBUG, debug );
                        freeaddrinfo( result );
                        WSACleanup();
                        return;
                    }

                    iResult = connect( ConnectSocket, result->ai_addr, ( int )result->ai_addrlen );

                    if ( iResult == SOCKET_ERROR )
                    {
                        string debug = "fail at connect";
                        ActivateOutput( pActInfo, PORT_OUT_DEBUG, debug );
                        freeaddrinfo( result );
                        closesocket( ConnectSocket );
                        WSACleanup();
                        return;
                    }

                    freeaddrinfo( result );

                    iResult = shutdown( ConnectSocket, SD_SEND );

                    if ( iResult == SOCKET_ERROR )
                    {
                        string debug = "fail at shutdown";
                        ActivateOutput( pActInfo, PORT_OUT_DEBUG, debug );
                        closesocket( ConnectSocket );
                        WSACleanup();
                        return;
                    }

                    // set up non blocking mode
                    u_long iMode = 1;
                    ioctlsocket( ConnectSocket, FIONBIO, &iMode );


                    // Activate update loop
                    pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, true );

                    string debug = "Active";
                    ActivateOutput( pActInfo, PORT_OUT_DEBUG, debug );


                }

                else
                {

                    active = 0;

                    // Disable update loop
                    pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, false );
                    closesocket( ConnectSocket );
                    WSACleanup();



                    string debug = "Disabled";
                    ActivateOutput( pActInfo, PORT_OUT_DEBUG, debug );

                }

            }

            return;
        }



        // -------------------------------------------------------------------------------------------------------------------------------
        // NODE UPDATE LOOP, ACTIVE STATE TRIGGERED FROM WITHIN ONACTIVATE FUNCTION
        // -------------------------------------------------------------------------------------------------------------------------------
        void OnUpdate( SActivationInfo* pActInfo )
        {
            // CLEAR BUFFER
            //for (int i=0;i<DEFAULT_BUFLEN;i++) {
            //   recvbuf[i]=NULL;
            //}

            // RECEIVE SOCKET DATA

            int iResult = recv( ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0 );

            // CAST CHAR* to JOINT* (BREKEL'S PROTOCOL STRUCTURE) + drop the first two doubles [16](some header perhaps?)
            //joint *ptr = reinterpret_cast <joint*> (recvbuf); //alternate option to cast char* to struct*

            joint* ptr = ( joint* ) &recvbuf[16];

            // DECLARE EACH JOINT

            joint waist = ptr[0];
            joint torso = ptr[1];
            joint neck = ptr[2];
            joint head = ptr[3];
            joint shoulderR = ptr[4];
            joint elbowR = ptr[5];
            joint handR = ptr[6];
            joint shoulderL = ptr[7];
            joint elbowL = ptr[8];
            joint handL = ptr[9];
            joint hipR = ptr[10];
            joint kneeR = ptr[11];
            joint footR = ptr[12];
            joint hipL = ptr[13];
            joint kneeL = ptr[14];
            joint footL = ptr[15];


            Vec3 waistPos = Vec3( ( float )waist.mT[0], ( float )waist.mT[1], ( float )waist.mT[2] );
            Vec3 waistRot = Vec3( ( float )waist.mR[0], ( float )waist.mR[1], ( float )waist.mR[2] );
            Vec3 torsoPos = Vec3( ( float )torso.mT[0], ( float )torso.mT[1], ( float )torso.mT[2] );
            Vec3 torsoRot = Vec3( ( float )torso.mR[0], ( float )torso.mR[1], ( float )torso.mR[2] );
            Vec3 neckPos = Vec3( ( float )neck.mT[0], ( float )neck.mT[1], ( float )neck.mT[2] );
            Vec3 neckRot = Vec3( ( float )neck.mR[0], ( float )neck.mR[1], ( float )neck.mR[2] );
            Vec3 headPos = Vec3( ( float )head.mT[0], ( float )head.mT[1], ( float )head.mT[2] );
            Vec3 headRot = Vec3( ( float )head.mR[0], ( float )head.mR[1], ( float )head.mR[2] );
            Vec3 shoulderRPos = Vec3( ( float )shoulderR.mT[0], ( float )shoulderR.mT[1], ( float )shoulderR.mT[2] );
            Vec3 shoulderRRot = Vec3( ( float )shoulderR.mR[0], ( float )shoulderR.mR[1], ( float )shoulderR.mR[2] );
            Vec3 elbowRPos = Vec3( ( float )elbowR.mT[0], ( float )elbowR.mT[1], ( float )elbowR.mT[2] );
            Vec3 elbowRRot = Vec3( ( float )elbowR.mR[0], ( float )elbowR.mR[1], ( float )elbowR.mR[2] );
            Vec3 handRPos = Vec3( ( float )handR.mT[0], ( float )handR.mT[1], ( float )handR.mT[2] );
            Vec3 handRRot = Vec3( ( float )handR.mR[0], ( float )handR.mR[1], ( float )handR.mR[2] );
            Vec3 shoulderLPos = Vec3( ( float )shoulderL.mT[0], ( float )shoulderL.mT[1], ( float )shoulderL.mT[2] );
            Vec3 shoulderLRot = Vec3( ( float )shoulderL.mR[0], ( float )shoulderL.mR[1], ( float )shoulderL.mR[2] );
            Vec3 elbowLPos = Vec3( ( float )elbowL.mT[0], ( float )elbowL.mT[1], ( float )elbowL.mT[2] );
            Vec3 elbowLRot = Vec3( ( float )elbowL.mR[0], ( float )elbowL.mR[1], ( float )elbowL.mR[2] );
            Vec3 handLPos = Vec3( ( float )handL.mT[0], ( float )handL.mT[1], ( float )handL.mT[2] );
            Vec3 handLRot = Vec3( ( float )handL.mR[0], ( float )handL.mR[1], ( float )handL.mR[2] );
            Vec3 hipRPos = Vec3( ( float )hipR.mT[0], ( float )hipR.mT[1], ( float )hipR.mT[2] );
            Vec3 hipRRot = Vec3( ( float )hipR.mR[0], ( float )hipR.mR[1], ( float )hipR.mR[2] );
            Vec3 kneeRPos = Vec3( ( float )kneeR.mT[0], ( float )kneeR.mT[1], ( float )kneeR.mT[2] );
            Vec3 kneeRRot = Vec3( ( float )kneeR.mR[0], ( float )kneeR.mR[1], ( float )kneeR.mR[2] );
            Vec3 footRPos = Vec3( ( float )footR.mT[0], ( float )footR.mT[1], ( float )footR.mT[2] );
            Vec3 footRRot = Vec3( ( float )footR.mR[0], ( float )footR.mR[1], ( float )footR.mR[2] );
            Vec3 hipLPos = Vec3( ( float )hipL.mT[0], ( float )hipL.mT[1], ( float )hipL.mT[2] );
            Vec3 hipLRot = Vec3( ( float )hipL.mR[0], ( float )hipL.mR[1], ( float )hipL.mR[2] );
            Vec3 kneeLPos = Vec3( ( float )kneeL.mT[0], ( float )kneeL.mT[1], ( float )kneeL.mT[2] );
            Vec3 kneeLRot = Vec3( ( float )kneeL.mR[0], ( float )kneeL.mR[1], ( float )kneeL.mR[2] );
            Vec3 footLPos = Vec3( ( float )footL.mT[0], ( float )footL.mT[1], ( float )footL.mT[2] );
            Vec3 footLRot = Vec3( ( float )footL.mR[0], ( float )footL.mR[1], ( float )footL.mR[2] );

            // create string of doubles to debug
            //double *ptr2 = (double*) &recvbuf[16]; //cast *char to struct* + drop first 2 doubles [16]

            std::ostringstream strs;
            strs.str( std::string() );
            strs << "debug";
            std::string str = strs.str();
            const char* c = str.c_str();

            //string debug = "debug";

            // set node output values
            ActivateOutput( pActInfo, PORT_OUT_WAIST_POS, waistPos );
            ActivateOutput( pActInfo, PORT_OUT_WAIST_ROT, waistRot );
            ActivateOutput( pActInfo, PORT_OUT_TORSO_ROT, torsoPos );
            ActivateOutput( pActInfo, PORT_OUT_TORSO_ROT, torsoRot );
            ActivateOutput( pActInfo, PORT_OUT_NECK_POS, neckPos );
            ActivateOutput( pActInfo, PORT_OUT_NECK_ROT, neckRot );
            ActivateOutput( pActInfo, PORT_OUT_HEAD_POS, headPos );
            ActivateOutput( pActInfo, PORT_OUT_HEAD_ROT, headRot );
            ActivateOutput( pActInfo, PORT_OUT_SHOULDER_R_POS, shoulderRPos );
            ActivateOutput( pActInfo, PORT_OUT_SHOULDER_R_ROT, shoulderRRot );
            ActivateOutput( pActInfo, PORT_OUT_ELBOW_R_POS, elbowRPos );
            ActivateOutput( pActInfo, PORT_OUT_ELBOW_R_ROT, elbowRRot );
            ActivateOutput( pActInfo, PORT_OUT_HAND_R_POS, handRPos );
            ActivateOutput( pActInfo, PORT_OUT_HAND_R_ROT, handRRot );
            ActivateOutput( pActInfo, PORT_OUT_SHOULDER_L_POS, shoulderLPos );
            ActivateOutput( pActInfo, PORT_OUT_SHOULDER_L_ROT, shoulderLRot );
            ActivateOutput( pActInfo, PORT_OUT_ELBOW_L_POS, elbowLPos );
            ActivateOutput( pActInfo, PORT_OUT_ELBOW_L_ROT, elbowLRot );
            ActivateOutput( pActInfo, PORT_OUT_HAND_L_POS, handLPos );
            ActivateOutput( pActInfo, PORT_OUT_HAND_L_ROT, handLRot );
            ActivateOutput( pActInfo, PORT_OUT_HIP_R_POS, hipRPos );
            ActivateOutput( pActInfo, PORT_OUT_HIP_R_ROT, hipRRot );
            ActivateOutput( pActInfo, PORT_OUT_KNEE_R_POS, kneeRPos );
            ActivateOutput( pActInfo, PORT_OUT_KNEE_R_ROT, kneeRRot );
            ActivateOutput( pActInfo, PORT_OUT_FOOT_R_POS, footRPos );
            ActivateOutput( pActInfo, PORT_OUT_FOOT_R_ROT, footRRot );
            ActivateOutput( pActInfo, PORT_OUT_HIP_L_POS, hipLPos );
            ActivateOutput( pActInfo, PORT_OUT_HIP_L_ROT, hipLRot );
            ActivateOutput( pActInfo, PORT_OUT_KNEE_L_POS, kneeLPos );
            ActivateOutput( pActInfo, PORT_OUT_KNEE_L_ROT, kneeLRot );
            ActivateOutput( pActInfo, PORT_OUT_FOOT_L_POS, footLPos );
            ActivateOutput( pActInfo, PORT_OUT_FOOT_L_ROT, footLRot );

            ActivateOutput( pActInfo, PORT_OUT_DEBUG, ( string ) c );
        }

        virtual void GetMemoryUsage( ICrySizer* s ) const
        {
            s->Add( *this );
        }
    };
}

REGISTER_FLOW_NODE_EX( "VariousStuff:TCPCommunication", TCPClientPlugin::CFlowTCPCommunicationNode, CFlowTCPCommunicationNode );