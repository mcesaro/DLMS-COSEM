#include "APDU/ACTION-Request.h"

namespace EPRI
{
        
    ASN_BEGIN_SCHEMA(Action_Request::Action_Request_Schema)
        ASN_BEGIN_CHOICE
            ASN_BEGIN_CHOICE_ENTRY_WITH_OPTIONS(Action_Request::Action_Request_Choice::action_request_normal, ASN::IMPLICIT)
                ASN_BEGIN_SEQUENCE
                    ASN_INVOKE_ID_AND_PRIORITY
                    ASN_BASE_TYPE(ASN::DT_Unsigned16)
                    ASN_FIXED_OCTET_STRING_TYPE(ASN::IMPLICIT, 6)
                    ASN_BASE_TYPE(ASN::DT_Integer8)
                    ASN_DATA_TYPE_WITH_OPTIONS(ASN::OPTIONAL)
                ASN_END_SEQUENCE
            ASN_END_CHOICE_ENTRY
            //
            // TODO - Other Request Types
            //
        ASN_END_CHOICE
    ASN_END_SCHEMA
        
    Action_Request::Action_Request()
        : Action_Request_Base::APDUSingleType(Action_Request::Action_Request_Schema)
    {

    }
    
    Action_Request::~Action_Request()
    {
    }
    
    bool Action_Request::IsValid() const
    {
        return true;
    }
    //
    // Action_Request_Normal
    //
    Action_Request_Normal::Action_Request_Normal()
    {
    }
    
    Action_Request_Normal::~Action_Request_Normal()
    {
    }
        
    bool Action_Request_Normal::Parse(COSEMAddressType SourceAddress,
        COSEMAddressType DestinationAddress,
        DLMSVector * pData)
    {
        // Perform the base parse, which just loads
        // the stream.
        //
        if (Action_Request::Parse(SourceAddress, DestinationAddress, pData))
        {
            int8_t       Choice;
            DLMSValue    Value;

            m_Type.Rewind();
            if (ASNType::GetNextResult::VALUE_RETRIEVED == m_Type.GetNextValue(&Value) &&
                m_Type.GetChoice(&Choice) && 
                (Choice == action_request_normal))   
            {
                try
                {
                    if (IsSequence(Value))
                    {
                        DLMSSequence& Sequence = DLMSValueGetSequence(Value);
                        invoke_id_and_priority = Sequence[0].get<uint8_t>();
                        cosem_method_descriptor.class_id = Sequence[1].get<uint16_t>();
                        cosem_method_descriptor.instance_id = Sequence[2].get<DLMSVector>();
                        cosem_method_descriptor.method_id = Sequence[3].get<int8_t>();
                        if (IsBlank(Sequence[4]))
                        {
                            method_invocation_parameters = DLMSOptionalNone;
                        }
                        else
                        {
                            method_invocation_parameters = Sequence[4].get<DLMSVector>();
                        }
                        return true;
                    }
                }
                catch (const std::exception&)
                {
                }
            }
        }
        return false;
    }
    
    std::vector<uint8_t> Action_Request_Normal::GetBytes()
    {
        m_Type.Clear();
        if (m_Type.SelectChoice(action_request_normal))
        {
            m_Type.Append(invoke_id_and_priority);
            m_Type.Append(cosem_method_descriptor.class_id);
            m_Type.Append(cosem_method_descriptor.instance_id);
            m_Type.Append(cosem_method_descriptor.method_id);
            m_Type.Append(MakeVariant(method_invocation_parameters));
            return Action_Request::GetBytes();
        }
        return std::vector<uint8_t>();
    }


}