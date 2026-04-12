#pragma once

#include <string.h>
#include <stdint.h>
#include "IOPluginDefs.h"
#include "IOPluginProps.h"

#include <map>
#include <string>
#include <vector>

using namespace IOPlugin;

APIContext* GetHostAPI();
void SetHostAPI(const APIContext* p_pAPI);

void g_Log(uint32_t p_LogLevel, const char* p_pFmt, ...);

namespace IOPlugin
{
    class IHostObjRef
    {
    public:
        explicit IHostObjRef(ObjectRef p_pObj);
        virtual ~IHostObjRef();

        bool IsValid() const
        {
            return (m_pOpaque != NULL);
        }

        ObjectRef GetOpaque() const
        {
            return m_pOpaque;
        }

        ObjectRef Detach()
        {
            ObjectRef pRetVal = m_pOpaque;
            m_pOpaque = NULL;
            return pRetVal;
        }

    private:
        // disable assignment and copy constructor
        IHostObjRef& operator=(const IHostObjRef& p_Other);
        IHostObjRef(const IHostObjRef& p_Other);

    protected:
        ObjectRef m_pOpaque = NULL;
    };

    class IPropertyProvider
    {
    public:
        virtual StatusCode GetProperty(PropertyID p_PropID, PropertyType* p_pPropType, const void** p_ppValue, int* p_pNumValues) = 0;
        virtual StatusCode SetProperty(PropertyID p_PropID, PropertyType p_PropType, const void* p_pValue, int p_NumValues) = 0;

    public:
        bool GetINT32(PropertyID p_ID, int32_t& p_Val);
        bool GetUINT32(PropertyID p_ID, uint32_t& p_Val);
        bool GetUINT8(PropertyID p_ID, uint8_t& p_Val);
        bool GetINT64(PropertyID p_ID, int64_t& p_Val);
        bool GetINT16(PropertyID p_ID, int16_t& p_Val);
        bool GetDouble(PropertyID p_ID, double& p_Val);
        bool GetString(PropertyID p_ID, std::string& p_Str);
    };

    class HostPropertyCollectionRef : public IHostObjRef, public IPropertyProvider
    {
    public:
        HostPropertyCollectionRef();
        explicit HostPropertyCollectionRef(ObjectRef p_pObj)
            : IHostObjRef(p_pObj) // with ownership
        {
        }

        virtual ~HostPropertyCollectionRef()
        {
        }

        // IPropertyProvider Imps
        virtual StatusCode GetProperty(PropertyID p_PropID, PropertyType* p_pPropType, const void** p_ppValue, int* p_pNumValues) override;
        virtual StatusCode SetProperty(PropertyID p_PropID, PropertyType p_PropType, const void* p_pValue, int p_NumValues) override;
    };

    class HostBufferRef : public IHostObjRef, public IPropertyProvider
    {
    public:
        explicit HostBufferRef(bool p_IsPinned = false);
        explicit HostBufferRef(ObjectRef p_pObj)
            : IHostObjRef(p_pObj) // with ownership
        {
        }

        virtual ~HostBufferRef()
        {
        }

        bool Resize(size_t p_NewSize);
        bool LockBuffer(char** p_ppBuf, size_t* p_pSize);
        bool UnlockBuffer();

        // IPropertyProvider Imps
        virtual StatusCode GetProperty(PropertyID p_PropID, PropertyType* p_pPropType, const void** p_ppValue, int* p_pNumValues) override;
        virtual StatusCode SetProperty(PropertyID p_PropID, PropertyType p_PropType, const void* p_pValue, int p_NumValues) override;
    };

    class HostCodecCallbackRef : public IHostObjRef
    {
    public:
        explicit HostCodecCallbackRef(ObjectRef p_pObj)
            : IHostObjRef(p_pObj)
        {
        }

        virtual ~HostCodecCallbackRef()
        {
        }

        StatusCode SendOutput(HostBufferRef* p_pBuf);
        bool IsAcceptingFrame(int64_t p_PTS);
    };

    class HostListRef : public IHostObjRef
    {
    public:
        explicit HostListRef(ObjectRef p_pObj)
            : IHostObjRef(p_pObj)
        {
        }

        ~HostListRef()
        {
        }

        bool Append(IHostObjRef* p_pObj);
    };

    class HostCodecConfigCommon
    {
    public:
        HostCodecConfigCommon() = default;
        ~HostCodecConfigCommon() = default;

        void Load(IPropertyProvider* p_pOptions);

        uint32_t GetWidth() const
        {
            return m_Width;
        }

        uint32_t GetHeight() const
        {
            return m_Height;
        }

        uint32_t GetFrameRateNum() const
        {
            return m_FrameRateNum;
        }

        uint32_t GetFrameRateDen() const
        {
            return m_FrameRateDen;
        }

        bool IsDropFrame() const
        {
            return m_IsDropFrame;
        }

        uint8_t IsFullRange() const
        {
            return (m_DataRange == 1);
        }

        bool HasAlpha() const
        {
            return m_HasAlpha;
        }

        uint8_t GetFieldOrder() const
        {
            return m_FieldOrder;
        }

        const std::string GetPath() const
        {
            return m_Path;
        }

        const std::string GetContainer() const
        {
            return m_Container;
        }

    private:
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint8_t m_FieldOrder = 0; // 0 - progressive, 1 - top first, 2 - btm forst
        uint32_t m_FrameRateNum = 0;
        uint32_t m_FrameRateDen = 0;
        bool m_IsDropFrame = false;
        uint8_t m_DataRange = 0;
        bool m_HasAlpha = false;
        std::string m_Path;
        std::string m_Container;
    };

    class HostUIConfigEntryRef : public HostPropertyCollectionRef
    {
    public:
        explicit HostUIConfigEntryRef(const std::string& p_Name);
        virtual ~HostUIConfigEntryRef()
        {
        }

        bool IsSuccess() const
        {
            return (m_StatusCode == errNone);
        }

    public:
        // UI Entries
        void MakeSeparator();
        void MakeLabel(const std::string& p_Text);
        void MakeSlider(const std::string p_Text, const std::string p_Units,
                        int32_t p_Val, int32_t p_MinVal, int32_t p_MaxVal,
                        int32_t p_DefVal, int32_t p_Step = 1);
        void MakeButton(const std::string& p_Text, bool p_IsPressed = false);
        void MakeCheckBox(const std::string& p_Title, const std::string& p_Text, bool p_IsChecked = false);
        void MakeComboBox(const std::string& p_Text, const std::vector<std::string>& p_Texts, const std::vector<int32_t>& p_Values, int32_t p_Value, const std::string& p_Suffix = std::string());
        void MakeRadioBox(const std::string& p_Label, const std::vector<std::string>& p_TextVec, const std::vector<int32_t>& p_ValueVec, int32_t p_Value);
        void MakeTextBox(const std::string& p_Label, const std::string& p_Text, const std::string& p_Suffix);
        void MakeMarkerColorSelector(const std::string& p_Label, const std::string& p_Suffix, const std::string& p_Value = std::string());

        // common properties
        void SetDisabled(bool p_IsDisabled); // default enabled
        void SetHidden(bool p_IsHidden); // default visible
        void SetTriggersUpdate(bool p_IsTrigger); // default false

    private:
        StatusCode m_StatusCode;
    };

    class HostMarkerInfo
    {
    private:
        enum BlobKey
        {
            BLOB_KEY_POSITION = 0x00020001,
            BLOB_KEY_DURATION = 0x00020002,
            BLOB_KEY_NAME = 0x00020003,
            BLOB_KEY_COLOR = 0x00020004,
        };

    public:
        HostMarkerInfo() = default;
        ~HostMarkerInfo() = default;

        HostMarkerInfo(const std::string& p_Name, const std::string& p_Color, double p_PositionSeconds, double p_DurationSeconds)
            : m_Name(p_Name)
            , m_Color(p_Color)
            , m_PositionSeconds(p_PositionSeconds)
            , m_DurationSeconds(p_DurationSeconds)
        {
        }

        const std::string& GetName() const
        {
            return m_Name;
        }

        const std::string& GetColor() const
        {
            return m_Color;
        }

        double GetDurationSeconds() const
        {
            return m_DurationSeconds;
        }

        double GetPositionSeconds() const
        {
            return m_PositionSeconds;
        }

        bool IsValid() const
        {
            return ((m_PositionSeconds >= 0.0) && !m_Color.empty());
        }

        bool FromBuffer(const uint8_t* p_pBuf, uint32_t p_BufSize);

    private:
        std::string m_Name;
        std::string m_Color;
        double m_PositionSeconds = -1.0;
        double m_DurationSeconds = 0.0;
    };

    class HostMarkersMap
    {
    private:
        enum BlobKey
        {
            BLOB_KEY_MARKER = 0x00010001,
        };

    public:
        HostMarkersMap() = default;
        ~HostMarkersMap() = default;

        bool FromBuffer(const uint8_t* p_pBuf, uint32_t p_BufSize);
        const std::map<double, HostMarkerInfo>& GetMarkersMap() const
        {
            return m_MarkersMap;
        }

    private:
        std::map<double, HostMarkerInfo> m_MarkersMap;
    };
}
