// ===========================================================================
// Copyright (c) 2018, Electric Power Research Institute (EPRI)
// All rights reserved.
//
// DLMS-COSEM ("this software") is licensed under BSD 3-Clause license.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// *  Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// *  Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// *  Neither the name of EPRI nor the names of its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.
//
// This EPRI software incorporates work covered by the following copyright and permission
// notices. You may not use these works except in compliance with their respective
// licenses, which are provided below.
//
// These works are provided by the copyright holders and contributors "as is" and any express or
// implied warranties, including, but not limited to, the implied warranties of merchantability
// and fitness for a particular purpose are disclaimed.
//
// This software relies on the following libraries and licenses:
//
// ###########################################################################
// Boost Software License, Version 1.0
// ###########################################################################
//
// * asio v1.10.8 (https://sourceforge.net/projects/asio/files/)
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 

#include "STM32CosemServer.h"
#include "COSEMAddress.h"

namespace EPRI
{
    //
    // Data
    //
    STM32Data::STM32Data()
        : IDataObject({ 0, 0, 96, 1, { 0, 9 }, 255 })
    {
        for (int Index = 0; Index < 10; ++Index)
        {
            m_Values[Index] = "STM32DATA" + std::to_string(Index);
        }
    }

    APDUConstants::Data_Access_Result STM32Data::InternalGet(const AssociationContext& Context,
        ICOSEMAttribute * pAttribute, 
        const Cosem_Attribute_Descriptor& Descriptor, 
        SelectiveAccess * pSelectiveAccess)
    {
        pAttribute->SelectChoice(COSEMDataType::VISIBLE_STRING);
        pAttribute->Append(m_Values[Descriptor.instance_id.GetValueGroup(EPRI::COSEMObjectInstanceID::VALUE_GROUP_E)]);
        return APDUConstants::Data_Access_Result::success;
    }
    
    APDUConstants::Data_Access_Result STM32Data::InternalSet(const AssociationContext& Context,
        ICOSEMAttribute * pAttribute, 
        const Cosem_Attribute_Descriptor& Descriptor, 
        const DLMSVector& Data,
        SelectiveAccess * pSelectiveAccess)
    {
        APDUConstants::Data_Access_Result RetVal = APDUConstants::Data_Access_Result::temporary_failure;
        try
        {
            DLMSValue Value;
            
            RetVal = ICOSEMObject::InternalSet(Context, pAttribute, Descriptor, Data, pSelectiveAccess);
            if (APDUConstants::Data_Access_Result::success == RetVal &&
                pAttribute->GetNextValue(&Value) == COSEMType::GetNextResult::VALUE_RETRIEVED)
            {
                m_Values[Descriptor.instance_id.GetValueGroup(EPRI::COSEMObjectInstanceID::VALUE_GROUP_E)] =
                    DLMSValueGet<std::string>(Value);
                RetVal = APDUConstants::Data_Access_Result::success;
            }
            else
            {
                RetVal = APDUConstants::Data_Access_Result::type_unmatched;
            }
        }
        catch (...) 
        {
            RetVal = APDUConstants::Data_Access_Result::type_unmatched;
        }
        return RetVal;
    }
    //
    // Clock
    //
    STM32Clock::STM32Clock()
        : IClockObject({ 0, 0, 1, 0, 0, 255 })
    {
    }

    APDUConstants::Data_Access_Result STM32Clock::InternalGet(const AssociationContext& Context,
        ICOSEMAttribute * pAttribute, 
        const Cosem_Attribute_Descriptor& Descriptor, 
        SelectiveAccess * pSelectiveAccess)
    {
        switch (pAttribute->AttributeID)
        {
        case ATTR_TIME:
        case ATTR_TIME_ZONE:
        case ATTR_STATUS:
        case ATTR_DST_BEGIN:
        case ATTR_DST_END:
        case ATTR_DST_DEVIATION:
        case ATTR_DST_ENABLED:
        case ATTR_CLOCK_BASE:
        default:
            break;
        }
        //
        // TODO
        //
        return APDUConstants::Data_Access_Result::object_unavailable;
    }

    APDUConstants::Action_Result STM32Clock::InternalAction(const AssociationContext& Context,
        ICOSEMMethod * pMethod, 
        const Cosem_Method_Descriptor& Descriptor, 
        const DLMSOptional<DLMSVector>& Parameters,
        DLMSVector * pReturnValue /*= nullptr*/)
    {
        switch (pMethod->MethodID)
        {
        case METHOD_ADJUST_TO_QUARTER:
        case METHOD_ADJUST_TO_MEAS_PERIOD:
        case METHOD_ADJUST_TO_MINUTE:
        case METHOD_ADJUST_TO_PRESET_TIME:
        case METHOD_PRESET_ADJUSTING_TIME:
        case METHOD_SHIFT_TIME:
        default:
            break;
        }
        //
        // TODO
        //
        return APDUConstants::Action_Result::object_unavailable;
    }
    //
    // Logical Device
    //
    STM32ManagementDevice::STM32ManagementDevice()
        : COSEMServer(ReservedAddresses::MANAGEMENT)
    {
        LOGICAL_DEVICE_BEGIN_OBJECTS
            LOGICAL_DEVICE_OBJECT(m_Clock)
            LOGICAL_DEVICE_OBJECT(m_Data)
        LOGICAL_DEVICE_END_OBJECTS
    }
    
    STM32ManagementDevice::~STM32ManagementDevice()
    {
    }
    //
    // COSEM Device
    //
    STM32COSEMDevice::STM32COSEMDevice()
    {
        SERVER_BEGIN_LOGICAL_DEVICES
            SERVER_LOGICAL_DEVICE(m_Management)
        SERVER_END_LOGICAL_DEVICES
    }

    STM32COSEMDevice::~STM32COSEMDevice()
    {
    }
    //
    // COSEM Engine
    //
    STM32COSEMServerEngine::STM32COSEMServerEngine(const Options& Opt, Transport * pXPort)
        : COSEMServerEngine(Opt, pXPort)
    {
        ENGINE_BEGIN_DEVICES
            ENGINE_DEVICE(m_Device)
        ENGINE_END_DEVICES    
    }
    
    STM32COSEMServerEngine::~STM32COSEMServerEngine()
    {
    }
    
}
