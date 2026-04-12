#include "host_api.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include <string>
#include <vector>

static APIContext s_HostAPI = { 0 };

APIContext* GetHostAPI()
{
    assert(s_HostAPI.version != 0);
    return &s_HostAPI;
}

void SetHostAPI(const APIContext* p_pAPI)
{
    s_HostAPI = *p_pAPI;
}

void g_Log(uint32_t p_LogLevel, const char* p_pFmt, ...)
{
    char pMsg[0xFFF];
    va_list args;
    va_start(args, p_pFmt);
    vsnprintf(pMsg, 0xFFF, p_pFmt, args);
    va_end(args);

    GetHostAPI()->pHandleMessage(msgResolveLog, p_LogLevel, pMsg);
}

namespace IOPlugin
{
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// IHostObjRef
    ///
    ////////////////////////////////////////////////////////////////////////////////
    IHostObjRef::IHostObjRef(ObjectRef p_pObj)
    {
        if (p_pObj != NULL)
        {
            int newRef = 0;
            StatusCode err = GetHostAPI()->pHandleMessage(msgRetain, p_pObj, &newRef);
            assert(err == errNone);
        }
        m_pOpaque = p_pObj;
    }

    IHostObjRef::~IHostObjRef()
    {
        if (m_pOpaque != NULL)
        {
            int newRef = 0;
            StatusCode err = GetHostAPI()->pHandleMessage(msgRelease, m_pOpaque, &newRef);
            assert(err == errNone);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// IPropertyProvider
    ///
    ////////////////////////////////////////////////////////////////////////////////
    bool IPropertyProvider::GetINT32(PropertyID p_ID, int32_t& p_Val)
    {
        PropertyType type = propTypeNull;
        const void* pVal = NULL;
        int numVals = 0;
        const StatusCode err = GetProperty(p_ID, &type, &pVal, &numVals);
        if ((err != errNone) || (type != propTypeInt32) || (numVals != 1))
        {
            return false;
        }

        p_Val = *static_cast<const int32_t*>(pVal);
        return true;
    }

    bool IPropertyProvider::GetUINT32(PropertyID p_ID, uint32_t& p_Val)
    {
        PropertyType type = propTypeNull;
        const void* pVal = NULL;
        int numVals = 0;
        const StatusCode err = GetProperty(p_ID, &type, &pVal, &numVals);
        if ((err != errNone) || (type != propTypeUInt32) || (numVals != 1))
        {
            return false;
        }

        p_Val = *static_cast<const uint32_t*>(pVal);
        return true;
    }

    bool IPropertyProvider::GetUINT8(PropertyID p_ID, uint8_t& p_Val)
    {
        PropertyType type = propTypeNull;
        const void* pVal = NULL;
        int numVals = 0;
        const StatusCode err = GetProperty(p_ID, &type, &pVal, &numVals);
        if ((err != errNone) || (type != propTypeUInt8) || (numVals != 1))
        {
            return false;
        }

        p_Val = *static_cast<const uint8_t*>(pVal);
        return true;
    }

    bool IPropertyProvider::GetINT64(PropertyID p_ID, int64_t& p_Val)
    {
        PropertyType type = propTypeNull;
        const void* pVal = NULL;
        int numVals = 0;
        const StatusCode err = GetProperty(p_ID, &type, &pVal, &numVals);
        if ((err != errNone) || (type != propTypeInt64) || (numVals != 1))
        {
            return false;
        }

        p_Val = *static_cast<const int64_t*>(pVal);
        return true;
    }

    bool IPropertyProvider::GetINT16(PropertyID p_ID, int16_t& p_Val)
    {
        PropertyType type = propTypeNull;
        const void* pVal = NULL;
        int numVals = 0;
        const StatusCode err = GetProperty(p_ID, &type, &pVal, &numVals);
        if ((err != errNone) || (type != propTypeInt16) || (numVals != 1))
        {
            return false;
        }

        p_Val = *static_cast<const int16_t*>(pVal);
        return true;
    }

    bool IPropertyProvider::GetDouble(PropertyID p_ID, double& p_Val)
    {
        PropertyType type = propTypeNull;
        const void* pVal = NULL;
        int numVals = 0;
        const StatusCode err = GetProperty(p_ID, &type, &pVal, &numVals);
        if ((err != errNone) || (type != propTypeDouble) || (numVals != 1))
        {
            return false;
        }

        p_Val = *static_cast<const double*>(pVal);
        return true;
    }

    bool IPropertyProvider::GetString(PropertyID p_ID, std::string& p_Str)
    {
        PropertyType type = propTypeNull;
        const void* pVal = NULL;
        int numVals = 0;
        const StatusCode err = GetProperty(p_ID, &type, &pVal, &numVals);
        if ((err != errNone) || (type != propTypeString))
        {
            return false;
        }

        if (numVals <= 0)
        {
            p_Str.clear();
        }
        else
        {
            p_Str.resize(numVals);
            memcpy(&p_Str[0], pVal, numVals);
        }

        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostPropertyCollectionRef
    ///
    ////////////////////////////////////////////////////////////////////////////////
    HostPropertyCollectionRef::HostPropertyCollectionRef()
        : IHostObjRef(NULL)
    {
        const StatusCode err = GetHostAPI()->pHandleMessage(msgCreate, UUID_PropertyCollection, &m_pOpaque);
        if (err != errNone)
        {
            g_Log(logLevelError, "Plugin Failed to create a property bag");
        }
    }

    StatusCode HostPropertyCollectionRef::GetProperty(PropertyID p_PropID, PropertyType* p_pPropType, const void** p_ppValue, int* p_pNumValues)
    {
        return (IsValid() ? GetHostAPI()->pHandleMessage(msgPropGet, m_pOpaque, p_PropID, p_pPropType, p_ppValue, p_pNumValues) : errInvalidOperation);
    }

    StatusCode HostPropertyCollectionRef::SetProperty(PropertyID p_PropID, PropertyType p_PropType, const void* p_pValue, int p_NumValues)
    {
        return (IsValid() ? GetHostAPI()->pHandleMessage(msgPropSet, m_pOpaque, p_PropID, p_PropType, p_pValue, p_NumValues) : errInvalidOperation);
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostPropertyCollectionRef
    ///
    ////////////////////////////////////////////////////////////////////////////////
    HostBufferRef::HostBufferRef(bool p_IsPinned /* = false */)
        : IHostObjRef(NULL)
    {
        const StatusCode err = GetHostAPI()->pHandleMessage(msgCreate, p_IsPinned ? UUID_PinnedBuffer : UUID_UnpinnedBuffer, &m_pOpaque);
        if (err != errNone)
        {
            g_Log(logLevelError, "Plugin Failed to create a buffer");
            m_pOpaque = NULL;
        }
    }

    bool HostBufferRef::Resize(size_t p_NewSize)
    {
        return (IsValid() && (GetHostAPI()->pHandleMessage(msgBufferResize, m_pOpaque, p_NewSize) == errNone));
    }

    bool HostBufferRef::LockBuffer(char** p_ppBuf, size_t* p_pSize)
    {
        return (IsValid() && (GetHostAPI()->pHandleMessage(msgBufferLock, m_pOpaque, p_ppBuf, p_pSize) == errNone));
    }

    bool HostBufferRef::UnlockBuffer()
    {
        return (IsValid() && (errNone == GetHostAPI()->pHandleMessage(msgBufferUnlock, m_pOpaque)));
    }

    StatusCode HostBufferRef::GetProperty(PropertyID p_PropID, PropertyType* p_pPropType, const void** p_ppValue, int* p_pNumValues)
    {
        return (IsValid() ? GetHostAPI()->pHandleMessage(msgPropGet, m_pOpaque, p_PropID, p_pPropType, p_ppValue, p_pNumValues) : errInvalidOperation);
    }

    StatusCode HostBufferRef::SetProperty(PropertyID p_PropID, PropertyType p_PropType, const void* p_pValue, int p_NumValues)
    {
        return (IsValid() ? GetHostAPI()->pHandleMessage(msgPropSet, m_pOpaque, p_PropID, p_PropType, p_pValue, p_NumValues) : errInvalidOperation);
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostCodecCallbackRef
    ///
    ////////////////////////////////////////////////////////////////////////////////
    StatusCode HostCodecCallbackRef::SendOutput(HostBufferRef* p_pBuf)
    {
        return ((IsValid() && (p_pBuf != NULL)) ? GetHostAPI()->pHandleMessage(msgCodecProcessData, m_pOpaque, p_pBuf->GetOpaque()) : errInvalidOperation);
    }

    bool HostCodecCallbackRef::IsAcceptingFrame(int64_t p_PTS)
    {
        if (!IsValid())
        {
            return true;
        }

        uint8_t isAccepting = 1;
        GetHostAPI()->pHandleMessage(msgCodecAcceptFramePTS, m_pOpaque, p_PTS, &isAccepting);

        return isAccepting;
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostListRef
    ///
    ////////////////////////////////////////////////////////////////////////////////
    bool HostListRef::Append(IHostObjRef* p_pObj)
    {
        return (IsValid() && (errNone == GetHostAPI()->pHandleMessage(msgListAppend, m_pOpaque, p_pObj->GetOpaque())));
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostCodecConfigCommon
    ///
    ////////////////////////////////////////////////////////////////////////////////

    void HostCodecConfigCommon::Load(IPropertyProvider* p_pOptions)
    {
        if (p_pOptions == NULL)
        {
            return;
        }

        p_pOptions->GetString(pIOPropContainerList, m_Container);
        p_pOptions->GetString(pIOPropPath, m_Path);
        p_pOptions->GetUINT32(pIOPropWidth, m_Width);
        p_pOptions->GetUINT32(pIOPropHeight, m_Height);

        p_pOptions->GetUINT8(pIOPropFieldOrder, m_FieldOrder);

        PropertyType propType;
        const void* pVal = NULL;
        int numVals = 0;
        if ((errNone == p_pOptions->GetProperty(pIOPropFrameRate, &propType, &pVal, &numVals)) &&
            (propType == propTypeUInt32) && (pVal != NULL) && (numVals == 2))
        {
            const uint32_t* pFrameRate = static_cast<const uint32_t*>(pVal);
            m_FrameRateNum = pFrameRate[0];
            m_FrameRateDen = pFrameRate[1];
        }

        uint8_t val8 = 0;
        p_pOptions->GetUINT8(pIOPropFrameRateIsDrop, val8);
        m_IsDropFrame = (val8 != 0);

        p_pOptions->GetUINT8(pIOPropDataRange, m_DataRange);

        val8 = 0;
        p_pOptions->GetUINT8(pIOPropHasAlpha, val8);

        m_HasAlpha = (val8 != 0);
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostUIConfigEntryRef
    ///
    ////////////////////////////////////////////////////////////////////////////////
    HostUIConfigEntryRef::HostUIConfigEntryRef(const std::string& p_Name)
    {
        m_StatusCode = SetProperty(pIOPropName, propTypeString, p_Name.c_str(), p_Name.size());
    }

    void HostUIConfigEntryRef::MakeSeparator()
    {
        if (m_StatusCode != errNone)
        {
            return;
        }

        uint32_t uint32val = uiEntrySeparator;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);
    }

    void HostUIConfigEntryRef::MakeLabel(const std::string& p_Text)
    {
        if (m_StatusCode != errNone)
        {
            return;
        }

        const uint32_t uint32val = uiEntryLabel;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);
        if (m_StatusCode != errNone)
        {
            return;
        }

        m_StatusCode = SetProperty(pIOPropUIValue, propTypeString, p_Text.c_str(), p_Text.size());
    }

    void HostUIConfigEntryRef::MakeSlider(const std::string p_Text, const std::string p_Units,
                                          int32_t p_Val, int32_t p_MinVal, int32_t p_MaxVal,
                                          int32_t p_DefVal, int32_t p_Step /*= 1*/)
    {
        if (m_StatusCode != errNone) return;

        const uint32_t uint32val = uiEntrySlider;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUIValue, propTypeInt32, &p_Val, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUIMinValue, propTypeInt32, &p_MinVal, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUIMaxValue, propTypeInt32, &p_MaxVal, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUIDefaultValue, propTypeInt32, &p_DefVal, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUIStep, propTypeInt32, &p_Step, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUILabel, propTypeString, p_Text.c_str(), p_Text.size());
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUISuffix, propTypeString, p_Units.c_str(), p_Units.size());
    }

    void HostUIConfigEntryRef::MakeButton(const std::string& p_Text, bool p_IsPressed /*= false*/)
    {
        if (m_StatusCode != errNone) return;

        const uint32_t uint32val = uiEntryButton;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUILabel, propTypeString, p_Text.c_str(), p_Text.size());
        if (m_StatusCode != errNone) return;

        const uint8_t val = p_IsPressed ? 1 : 0;
        m_StatusCode = SetProperty(pIOPropUIValue, propTypeUInt8, &val, 1);
    }

    void HostUIConfigEntryRef::MakeCheckBox(const std::string& p_Title, const std::string& p_Text, bool p_IsChecked /*= false*/)
    {
        if (m_StatusCode != errNone) return;

        const uint32_t uint32val = uiEntryCheckbox;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);

        m_StatusCode = SetProperty(pIOPropUILabel, propTypeString, p_Title.c_str(), p_Title.size());
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUISuffix, propTypeString, p_Text.c_str(), p_Text.size());
        if (m_StatusCode != errNone) return;

        const uint8_t val = p_IsChecked ? 1 : 0;
        m_StatusCode = SetProperty(pIOPropUIValue, propTypeUInt8, &val, 1);
    }

    void HostUIConfigEntryRef::MakeComboBox(const std::string& p_Text, const std::vector<std::string>& p_Texts, const std::vector<int32_t>& p_Values, int32_t p_Value, const std::string& p_Suffix /*= std::string()*/)
    {
        if (m_StatusCode != errNone) return;

        const size_t numItems = p_Texts.size();
        if ((numItems == 0) || (numItems != p_Values.size()))
        {
            m_StatusCode = errNoParam;
            return;
        }

        const uint32_t uint32val = uiEntryCombobox;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);

        m_StatusCode = SetProperty(pIOPropUIValuesList, propTypeInt32, p_Values.data(), p_Values.size());

        std::string valStrings;
        for (size_t i = 0; i < numItems; ++i)
        {
            valStrings.append(p_Texts[i]);
            if (i < (numItems - 1))
            {
                valStrings.append(1, '\0');
            }
        }

        m_StatusCode = SetProperty(pIOPropUITextsList, propTypeString, valStrings.c_str(), valStrings.size());
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUIValue, propTypeInt32, &p_Value, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUILabel, propTypeString, p_Text.c_str(), p_Text.size());
        if (m_StatusCode != errNone) return;

        if (!p_Suffix.empty())
        {
            m_StatusCode = SetProperty(pIOPropUISuffix, propTypeString, p_Suffix.c_str(), p_Suffix.size());
        }
    }

    void HostUIConfigEntryRef::MakeRadioBox(const std::string& p_Label, const std::vector<std::string>& p_TextVec, const std::vector<int32_t>& p_ValueVec, int32_t p_Value)
    {
        if (m_StatusCode != errNone) return;

        const size_t numItems = p_TextVec.size();
        if ((numItems == 0) || (numItems != p_ValueVec.size()))
        {
            m_StatusCode = errNoParam;
            return;
        }

        const uint32_t uint32val = uiEntryRadiobox;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);

        m_StatusCode = SetProperty(pIOPropUIValuesList, propTypeInt32, p_ValueVec.data(), p_ValueVec.size());

        std::string valStrings;
        for (size_t i = 0; i < numItems; ++i)
        {
            valStrings.append(p_TextVec[i]);
            if (i < (numItems - 1))
            {
                valStrings.append(1, '\0');
            }
        }

        m_StatusCode = SetProperty(pIOPropUITextsList, propTypeString, valStrings.c_str(), valStrings.size());
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUIValue, propTypeInt32, &p_Value, 1);
        if (m_StatusCode != errNone) return;

        m_StatusCode = SetProperty(pIOPropUILabel, propTypeString, p_Label.c_str(), p_Label.size());
    }

    void HostUIConfigEntryRef::MakeTextBox(const std::string& p_Label, const std::string& p_Text, const std::string& p_Suffix)
    {
        if (m_StatusCode != errNone) return;

        const uint32_t uint32val = uiEntryTextbox;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);

        if (!p_Label.empty())
        {
            m_StatusCode = SetProperty(pIOPropUILabel, propTypeString, p_Label.c_str(), p_Label.size());
            if (m_StatusCode != errNone) return;
        }

        if (!p_Suffix.empty())
        {
            m_StatusCode = SetProperty(pIOPropUISuffix, propTypeString, p_Suffix.c_str(), p_Suffix.size());
            if (m_StatusCode != errNone) return;
        }

        m_StatusCode = SetProperty(pIOPropUIValue, propTypeString, p_Text.c_str(), p_Text.size());
    }

    void HostUIConfigEntryRef::MakeMarkerColorSelector(const std::string& p_Label, const std::string& p_Suffix, const std::string& p_Value)
    {
        if (m_StatusCode != errNone) return;

        const uint32_t uint32val = uiEntryMarkers;
        m_StatusCode = SetProperty(pIOPropUIType, propTypeUInt32, &uint32val, 1);

        if (!p_Label.empty())
        {
            m_StatusCode = SetProperty(pIOPropUILabel, propTypeString, p_Label.c_str(), p_Label.size());
            if (m_StatusCode != errNone) return;
        }

        if (!p_Suffix.empty())
        {
            m_StatusCode = SetProperty(pIOPropUISuffix, propTypeString, p_Suffix.c_str(), p_Suffix.size());
            if (m_StatusCode != errNone) return;
        }

        if (!p_Value.empty())
        {
            m_StatusCode = SetProperty(pIOPropUIValue, propTypeString, p_Value.c_str(), p_Value.size());
        }
    }

    void HostUIConfigEntryRef::SetDisabled(bool p_IsDisabled)
    {
        if (m_StatusCode != errNone) return;

        const uint8_t val = p_IsDisabled ? 1 : 0;
        m_StatusCode = SetProperty(pIOPropUIDisabled, propTypeUInt8, &val, 1);
    }

    void HostUIConfigEntryRef::SetHidden(bool p_IsHidden)
    {
        if (m_StatusCode != errNone) return;

        const uint8_t val = p_IsHidden ? 1 : 0;
        m_StatusCode = SetProperty(pIOPropUIHidden, propTypeUInt8, &val, 1);
    }

    void HostUIConfigEntryRef::SetTriggersUpdate(bool p_IsTrigger)
    {
        if (m_StatusCode != errNone) return;

        const uint8_t val = p_IsTrigger ? 1 : 0;
        m_StatusCode = SetProperty(pIOPropUITriggerUpdate, propTypeUInt8, &val, 1);
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostMarkerInfo
    ///
    ////////////////////////////////////////////////////////////////////////////////
    bool HostMarkerInfo::FromBuffer(const uint8_t* p_pBuf, uint32_t p_BufSize)
    {
        m_Name.clear();
        m_Color.clear();
        m_PositionSeconds = -1.0;
        m_DurationSeconds = 0.0;

        uint32_t bytesLeft = p_BufSize;
        const uint8_t* pBuf = p_pBuf;

        while (bytesLeft > 8)
        {
            uint32_t key = 0;
            uint32_t len = 0;

            memcpy(&key, pBuf, 4);
            memcpy(&len, pBuf + 4, 4);
            pBuf += 8;
            bytesLeft -= 8;

            if (bytesLeft < len)
            {
                return false;
            }

            if (len == 0)
            {
                continue;
            }

            switch (key)
            {
                case BLOB_KEY_POSITION:
                {
                    if (len != sizeof(double))
                    {
                        return false;
                    }

                    memcpy(&m_PositionSeconds, pBuf, len);
                    break;
                }
                case BLOB_KEY_DURATION:
                {
                    if (len != sizeof(double))
                    {
                        return false;
                    }

                    memcpy(&m_DurationSeconds, pBuf, len);
                    break;
                }
                case BLOB_KEY_NAME:
                {
                    std::string str(len, '\0');
                    memcpy(&str[0], pBuf, len);
                    m_Name = str.c_str();
                    break;
                }
                case BLOB_KEY_COLOR:
                {
                    std::string str(len, '\0');
                    memcpy(&str[0], pBuf, len);
                    m_Color = str.c_str();
                    break;
                }
                default:
                    break;
            }

            pBuf += len;
            bytesLeft -= len;
        }

        return IsValid();
    }

    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// HostMarkersMap
    ///
    ////////////////////////////////////////////////////////////////////////////////
    bool HostMarkersMap::FromBuffer(const uint8_t* p_pBuf, uint32_t p_BufSize)
    {
        m_MarkersMap.clear();
        if (p_BufSize <= 8)
        {
            return true;
        }

        const uint8_t* pPtr = p_pBuf;
        size_t bytesLeft = p_BufSize;

        uint32_t ver = 0;
        memcpy(&ver, pPtr, 4);
        pPtr += 4;
        bytesLeft -= 4;

        if (ver != 1)
        {
            return false;
        }

        while (bytesLeft >= 8)
        {
            uint32_t key = 0;
            uint32_t len = 0;
            memcpy(&key, pPtr, 4);
            memcpy(&len, pPtr + 4, 4);
            pPtr += 8;
            bytesLeft -= 8;

            if (len > bytesLeft)
            {
                return false;
            }

            if (key == BLOB_KEY_MARKER)
            {
                HostMarkerInfo newMarker;
                if (!newMarker.FromBuffer(pPtr, len))
                {
                    return false;
                }

                m_MarkersMap.emplace(newMarker.GetPositionSeconds(), newMarker);
            }

            pPtr += len;
            bytesLeft -= len;
        }

        return true;
    }
};
