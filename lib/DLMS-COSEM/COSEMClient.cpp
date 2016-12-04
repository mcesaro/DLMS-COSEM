#include <cstring>

#include "COSEM.h"

namespace EPRI
{
    
    COSEMClient::COSEMClient(COSEMAddressType ClientAddress) :
        COSEM(ClientAddress)
    {
    }
    
    COSEMClient::~COSEMClient()
    {
    }
    //
    // COSEM
    //
    COSEMRunResult COSEMClient::Process()
    {

    }
    //
    // COSEM-OPEN Service
    //
    bool COSEMClient::OpenRequest(const APPOpenRequestOrIndication& Parameters)
    {
        bool bAllowed = false;
        BEGIN_TRANSITION_MAP
            TRANSITION_MAP_ENTRY(ST_INACTIVE, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_IDLE, ST_ASSOCIATION_PENDING)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATION_PENDING, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATION_RELEASE_PENDING, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATED, EVENT_IGNORED)
        END_TRANSITION_MAP(bAllowed, new OpenRequestEventData(Parameters));
        return bAllowed;
    }
    
    void COSEMClient::RegisterOpenConfirm(CallbackFunction Callback)
    {
        RegisterCallback(APPOpenConfirmOrResponse::ID, Callback);
    }
    //
    // COSEM-GET Service
    //
    bool COSEMClient::GetRequest(const APPGetRequestOrIndication& Parameters)
    {
        bool bAllowed = false;
        BEGIN_TRANSITION_MAP
            TRANSITION_MAP_ENTRY(ST_INACTIVE, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_IDLE, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATION_PENDING, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATION_RELEASE_PENDING, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATED, ST_ASSOCIATED)
        END_TRANSITION_MAP(bAllowed, new GetRequestEventData(Parameters));
        return bAllowed;
    }
    
    void COSEMClient::RegisterGetConfirm(CallbackFunction Callback)
    {
        RegisterCallback(APPGetConfirmOrResponse::ID, Callback);
    }
    //
    // COSEM-RELEASE Service
    //
    bool COSEMClient::ReleaseRequest(const APPReleaseRequestOrIndication& Parameters)
    {
        bool bAllowed = false;
        BEGIN_TRANSITION_MAP
            TRANSITION_MAP_ENTRY(ST_INACTIVE, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_IDLE, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATION_PENDING, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATION_RELEASE_PENDING, EVENT_IGNORED)
            TRANSITION_MAP_ENTRY(ST_ASSOCIATED, ST_ASSOCIATION_RELEASE_PENDING)
        END_TRANSITION_MAP(bAllowed, new ReleaseRequestEventData(Parameters));
        return bAllowed;
    }
    
    void COSEMClient::RegisterReleaseConfirm(CallbackFunction Callback)
    {
        RegisterCallback(APPReleaseConfirmOrResponse::ID, Callback);
    }
    //
	// State Machine Handlers
    //
    void COSEMClient::ST_Inactive_Handler(EventData * pData)
    {
    }
    
    void COSEMClient::ST_Idle_Handler(EventData * pData)
    {
        TransportEventData * pTransport;
        if ((pTransport = dynamic_cast<TransportEventData *>(pData)) != nullptr)
        {
            // If we have disconnected, then we need to go back to the Inactive state.
            //
            if (pTransport->Data == Transport::TRANSPORT_DISCONNECTED)
            {
                InternalEvent(ST_INACTIVE);
            }
        }
    }
    
    void COSEMClient::ST_Association_Pending_Handler(EventData * pData)
    {
        //
        // Receive OPEN Response
        //
        OpenResponseEventData * pConnectResponse = dynamic_cast<OpenResponseEventData *>(pData);
        if (pConnectResponse)
        {
            bool  RetVal = false;
            if (FireCallback(APPOpenConfirmOrResponse::ID, pConnectResponse->Data, &RetVal) && RetVal)
            {
                // Denied by upper layers.  Go back to ST_IDLE.
                //
                InternalEvent(ST_ASSOCIATED);
            }
            else
            {
                // Denied by upper layers or catastrophic failure.  Go back to ST_IDLE.
                //
                InternalEvent(ST_IDLE);
            }
            return;            
        }
        //
        // Transmit OPEN Request
        //
        OpenRequestEventData * pEventData;
        if ((pEventData = dynamic_cast<OpenRequestEventData *>(pData)) != nullptr)
        {
            AARQ                        Request;
            APPOpenRequestOrIndication& Parameters = pEventData->Data;
            if (Parameters.m_LogicalNameReferencing && !Parameters.m_WithCiphering)
            {
                Request.application_context_name.Append(ContextLNRNoCipher);
            }
            else if (!Parameters.m_LogicalNameReferencing && !Parameters.m_WithCiphering)
            {
                Request.application_context_name.Append(ContextSNRNoCipher);
            }
            
            Request.sender_acse_requirements.Append(ASNBitString(1, 1));
            
            if (COSEM::SECURITY_LOW_LEVEL == Parameters.m_SecurityLevel)
            {
                Request.mechanism_name.Append(MechanismNameLowLevelSecurity);
                Request.calling_authentication_value.SelectChoice(APDUConstants::AuthenticationValueChoice::charstring);
                Request.calling_authentication_value.Append(Parameters.m_Password);
            }
            else if (COSEM::SECURITY_HIGH_LEVEL == Parameters.m_SecurityLevel)
            {
                Request.mechanism_name.Append(MechanismNameHighLevelSecurity);
                //
                // TODO - HLS
                //
            }
            //
            // TODO - xDLMS
            //
            Request.user_information.Append(DLMSVector({ 0x01, 0x00, 0x00, 0x00, 0x06, 0x5F, 
                                                         0x1F, 0x04, 0x00, 0x00, 0x7E, 0x1F, 0x00, 0x00 }));
            Transport * pTransport = GetTransport();
            if (nullptr != pTransport)
            {
                pTransport->DataRequest(Transport::DataRequestParameter(GetAddress(),
                                                                        Parameters.m_DestinationAddress,
                                                                        Request.GetBytes()));
            }
        }
        
    }
    
    void COSEMClient::ST_Association_Release_Pending_Handler(EventData * pData)
    {
        //
        // Transmit RELEASE Request
        //
        ReleaseRequestEventData * pRequestData;
        if ((pRequestData = dynamic_cast<ReleaseRequestEventData *>(pData)) != nullptr)
        {
            APPReleaseRequestOrIndication& Parameters = pRequestData->Data;
            //
            // We only need to send an RLRQ if the user asks for it, otherwise
            // just disconnect and call it done.
            //
            if (Parameters.m_UseRLRQRLRE)
            {
                RLRQ Request;
                
                Transport * pTransport = GetTransport();
                if (nullptr != pTransport)
                {
                    pTransport->DataRequest(Transport::DataRequestParameter(GetAddress(),
                        Parameters.m_DestinationAddress,
                        Request.GetBytes()));
                }
            }
            else
            {
                //
                // TODO - Just drop the connection
                //
            }
            return;
        }
        //
        // Receive RELEASE Response
        //
        ReleaseResponseEventData * pReleaseResponse = dynamic_cast<ReleaseResponseEventData *>(pData);
        if (pReleaseResponse)
        {
            bool RetVal = false;
            //
            // Let the upper layers know that we have been released
            //
            FireCallback(APPReleaseConfirmOrResponse::ID, pReleaseResponse->Data, &RetVal);
            //
            // We have been released, go back to the IDLE state
            //
            InternalEvent(ST_IDLE);
            return;            
        }
        
    }
    
    void COSEMClient::ST_Associated_Handler(EventData * pData)
    {
        if (nullptr == pData)
        {
            return;
        }
        // 
        // Transmit GET Request
        //
        Transport *           pTransport = GetTransport();
        GetRequestEventData * pGetRequest = dynamic_cast<GetRequestEventData *>(pData);
        if (pTransport && pGetRequest)
        {
            Transport::DataRequestParameter TransportParam;
                
            switch (pGetRequest->Data.m_Type)
            {
            case APPGetRequestOrIndication::GetRequestType::get_request_normal:
                {
                    Get_Request_Normal Request;
                    Request.invoke_id_and_priority = pGetRequest->Data.m_InvokeIDAndPriority;
                    Request.cosem_attribute_descriptor = 
                        pGetRequest->Data.m_Parameter.get<Cosem_Attribute_Descriptor>();
                    
                    TransportParam.SourceAddress = GetAddress();
                    TransportParam.DestinationAddress = pGetRequest->Data.m_SourceAddress;
                    TransportParam.Data = Request.GetBytes();
                }
                break;
                    
            case APPGetRequestOrIndication::GetRequestType::get_request_next:
                throw std::logic_error("get_request_next Not Implemented!");

            case APPGetRequestOrIndication::GetRequestType::get_request_with_list:
                throw std::logic_error("get_request_with_list Not Implemented!");

            }
                
            pTransport->DataRequest(TransportParam);
        }
        //
        // Receive GET Response
        //
        GetResponseEventData * pGetResponse = dynamic_cast<GetResponseEventData *>(pData);
        if (pGetResponse)
        {
            bool  RetVal = false;
            FireCallback(APPGetConfirmOrResponse::ID, pGetResponse->Data, &RetVal);
            return;            
        }
    }
    //
    // APDU Handlers
    //
    bool COSEMClient::AARE_Handler(const IAPDUPtr& pAPDU)
    {
        bool      RetVal = false;
        DLMSValue AssociationResult;
        AARE *    pAARE = dynamic_cast<AARE *>(pAPDU.get());
        if (pAARE && 
            (ASNType::GetNextResult::VALUE_RETRIEVED == pAARE->result.GetNextValue(&AssociationResult)))
        {
            BEGIN_TRANSITION_MAP
                TRANSITION_MAP_ENTRY(ST_INACTIVE, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_IDLE, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATION_PENDING, ST_ASSOCIATION_PENDING)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATION_RELEASE_PENDING, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATED, EVENT_IGNORED)
            END_TRANSITION_MAP(RetVal,
                new OpenResponseEventData(APPOpenConfirmOrResponse(pAARE->GetSourceAddress(),
                        pAARE->GetDestinationAddress(),
                        (APPOpenConfirmOrResponse::AssociationResultType) DLMSValueGet<int8_t>(AssociationResult))))
        }
        else
        {
            ExternalEvent(ST_IDLE);
        }
        return RetVal;
    }

    bool COSEMClient::AARQ_Handler(const IAPDUPtr& pAPDU)
    {
        return false;
    }
    
    bool COSEMClient::GET_Request_Handler(const IAPDUPtr& pAPDU)
    {
        return false;
    }
    
    bool COSEMClient::GET_Response_Handler(const IAPDUPtr& pAPDU)
    {
        bool                  RetVal = false;
        Get_Response_Normal * pGetResponse = dynamic_cast<Get_Response_Normal *>(pAPDU.get());
        if (pGetResponse && (Get_Response::data == pGetResponse->result.which()))
        {
            BEGIN_TRANSITION_MAP
                TRANSITION_MAP_ENTRY(ST_INACTIVE, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_IDLE, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATION_PENDING, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATION_RELEASE_PENDING, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATED, ST_ASSOCIATED)
            END_TRANSITION_MAP(RetVal,
                                new GetResponseEventData(
                                    APPGetConfirmOrResponse(pGetResponse->GetSourceAddress(),
                                        pGetResponse->GetDestinationAddress(),
                                        pGetResponse->invoke_id_and_priority,
                                        pGetResponse->result.get<DLMSVector>())));
        }
        return RetVal;
    }

    bool COSEMClient::RLRQ_Handler(const IAPDUPtr& pAPDU)
    {
        return false;
    }
    
    bool COSEMClient::RLRE_Handler(const IAPDUPtr& pAPDU)
    {
        bool   RetVal = false;
        RLRE * pReleaseResponse = dynamic_cast<RLRE *>(pAPDU.get());
        if (pReleaseResponse)
        {
            BEGIN_TRANSITION_MAP
                TRANSITION_MAP_ENTRY(ST_INACTIVE, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_IDLE, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATION_PENDING, EVENT_IGNORED)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATION_RELEASE_PENDING, ST_ASSOCIATION_RELEASE_PENDING)
                TRANSITION_MAP_ENTRY(ST_ASSOCIATED, EVENT_IGNORED)
            END_TRANSITION_MAP(RetVal,
                                new ReleaseResponseEventData(APPReleaseConfirmOrResponse(pReleaseResponse->GetSourceAddress(),
                                    pReleaseResponse->GetDestinationAddress())));
        }
        return RetVal;
    }

}