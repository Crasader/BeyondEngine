#include "stdafx.h"
#include "Vec2fPropertyDescription.h"
#include "Utility/BeatsUtility/StringHelper.h"
#include "Utility/BeatsUtility/Serializer.h"
#include "Component/Component/ComponentProxyManager.h"
#include "Component/ComponentPublic.h"
#include "FloatPropertyDescription.h"

#define VEC_COUNT 2

SharePtr<SBasicPropertyInfo> CVec2PropertyDescription::m_pXBasicPropertyInfo = new SBasicPropertyInfo(true, 0xFFFFFFFF, _T("x"), NULL, NULL, _T("x"));
SharePtr<SBasicPropertyInfo> CVec2PropertyDescription::m_pYBasicPropertyInfo = new SBasicPropertyInfo(true, 0xFFFFFFFF, _T("y"), NULL, NULL, _T("y"));

CVec2PropertyDescription::CVec2PropertyDescription(CSerializer* pSerializer)
    : super(NULL)
{
    SetType(eRPT_Vec2F);
    SetMaxCount(VEC_COUNT);
    CFloatPropertyDescription* pFloatProperty = down_cast<CFloatPropertyDescription*>(CComponentProxyManager::GetInstance()->CreateProperty(eRPT_Float, NULL));
    pFloatProperty->SetSpinStep(0.1f);
    SetTemplateProperty(pFloatProperty);
    TString InitValue = _T("0,0");
    if (pSerializer != NULL)
    {
        InitValue.clear();
        for (uint32_t i = 0; i < VEC_COUNT; ++i)
        {
            float fValue = 0;
            (*pSerializer) >> fValue;
            InitValue.append(wxString::Format(_T("%f"),fValue));
            if (i != VEC_COUNT - 1)
            {
                InitValue.append(_T(","));
            }
            CPropertyDescriptionBase* pChild = InsertChild(NULL);
            pChild->InitializeValue(fValue);
        }
        ResetChildName();
    }
    InitializeValue(InitValue);
}

CVec2PropertyDescription::CVec2PropertyDescription(const CVec2PropertyDescription& rRef)
    : super(rRef)
{
    InitializeValue(*(TString*)rRef.GetValue(eVT_DefaultValue));
}

CVec2PropertyDescription::~CVec2PropertyDescription()
{
}

void CVec2PropertyDescription::Initialize()
{
    super::Initialize();
    CComponentProxy* pOwnerProxy = GetOwner();
    BEATS_ASSERT(pOwnerProxy != NULL);
    for (uint32_t i = 0; i < VEC_COUNT; ++i)
    {
        (*m_pChildren)[i]->SetOwner(pOwnerProxy);
    }
}

bool CVec2PropertyDescription::AnalyseUIParameterImpl( const std::vector<TString>& result)
{
    std::vector<TString> cache;
    for (uint32_t i = 0; i < result.size(); ++i)
    {
        cache.clear();
        CStringHelper::GetInstance()->SplitString(result[i].c_str(), PROPERTY_KEYWORD_SPLIT_STR, cache);
        BEATS_ASSERT(cache.size() == 2);
        if (_tcsicmp(cache[0].c_str(), UIParameterAttrStr[eUIPAT_DefaultValue]) == 0)
        {
            std::vector<TString> values;
            CStringHelper::GetInstance()->SplitString(cache[1].c_str(), _T("@"), values);
            BEATS_ASSERT(values.size() == VEC_COUNT);
            for (uint32_t j = 0; j < VEC_COUNT; ++j)
            {
                m_pChildren->at(j)->InitializeValue((float)_tstof(values[j].c_str()));
            }
            ResetName();
        }
        else
        {
            std::vector<TString> spinStepParamVec;
            spinStepParamVec.push_back(result[i]);

            for (uint32_t j = 0; j < VEC_COUNT; ++j)
            {
                ((CWxwidgetsPropertyBase*)m_pChildren->at(j))->AnalyseUIParameterImpl(spinStepParamVec);
            }
        }
    }
    return true;
}

bool CVec2PropertyDescription::IsDataSame( bool bWithDefaultOrXML )
{
    bool bRet = true;
    if (m_pChildren->size() == VEC_COUNT)
    {
        for (uint32_t i = 0; i < VEC_COUNT; ++i)
        {
            if (!m_pChildren->at(i)->IsDataSame(bWithDefaultOrXML))
            {
                bRet = false;
                break;
            }
        }
    }
    return bRet;
}

CPropertyDescriptionBase* CVec2PropertyDescription::CreateNewInstance()
{
    CPropertyDescriptionBase* pNewProperty = new CVec2PropertyDescription(*this);
    return pNewProperty;
}

void CVec2PropertyDescription::ResetChildName()
{
    if (m_pChildren->size() == VEC_COUNT)
    {
        for (uint32_t i = 0; i < VEC_COUNT; ++i)
        {
            if (i == 0)
            {
                if ((*m_pChildren)[0]->GetBasicInfo() != m_pXBasicPropertyInfo)
                {
                    (*m_pChildren)[0]->ResetBasicInfo(m_pXBasicPropertyInfo);
                }
            }
            else if (i == 1)
            {
                if ((*m_pChildren)[1]->GetBasicInfo() != m_pYBasicPropertyInfo)
                {
                    (*m_pChildren)[1]->ResetBasicInfo(m_pYBasicPropertyInfo);
                }
            }
            else
            {
                BEATS_ASSERT(false, _T("Unkonwn Count!"));
            }
        }
    }
}

TString CVec2PropertyDescription::GetCurrentName()
{
    wxString strName;
    if (m_pChildren->size() == VEC_COUNT)
    {
        char szBuffer[16];
        for (uint32_t i = 0; i < VEC_COUNT; ++i)
        {
            m_pChildren->at(i)->GetValueAsChar(eVT_CurrentValue, szBuffer);
            strName.Append(szBuffer);
            if (i != VEC_COUNT - 1)
            {
                strName.Append(_T(","));
            }
        }
    }
    return TString(strName);
}

CPropertyDescriptionBase* CVec2PropertyDescription::Clone(bool /*bCloneValue*/)
{
    CVec2PropertyDescription* pNewProperty = static_cast<CVec2PropertyDescription*>(super::Clone(true));
    return pNewProperty;
}

bool CVec2PropertyDescription::IsContainerProperty()
{
    return false;
}

void CVec2PropertyDescription::LoadFromXML( rapidxml::xml_node<>* pNode )
{
    CWxwidgetsPropertyBase::LoadFromXML(pNode);
    rapidxml::xml_node<>* pVarElement = pNode->first_node("VariableNode");
    while (pVarElement != NULL)
    {
        int iVarType = 0;
        iVarType = atoi(pVarElement->first_attribute("Type")->value());
        if (iVarType == GetTemplateProperty()->GetType())
        {
            BEATS_ASSERT(m_pChildren->size() == VEC_COUNT);
            for (uint32_t i = 0; i < m_pChildren->size(); ++i)
            {
                (*m_pChildren)[i]->LoadFromXML(pVarElement);
                pVarElement = pVarElement->next_sibling("VariableNode");
            }
        }
        else
        {
            BEATS_WARNING(false, _T("UnMatch type of property!"));
        }
        BEATS_ASSERT(pVarElement == NULL, _T("No more parameter for vec2!"));
    }
    ResetName();
}

void CVec2PropertyDescription::Serialize( CSerializer& serializer, EValueType eValueType/* = eVT_SavedValue*/ )
{
    for (uint32_t i = 0; i < m_pChildren->size(); ++i)
    {
        (*m_pChildren)[i]->Serialize(serializer, eValueType);
    }
}

void CVec2PropertyDescription::Deserialize(CSerializer& serializer, EValueType eValueType/* = eVT_CurrentValue*/)
{
    for (uint32_t i = 0; i < m_pChildren->size(); ++i)
    {
        (*m_pChildren)[i]->Deserialize(serializer, eValueType);
    }
    ResetName();
}

bool CVec2PropertyDescription::GetValueByTChar(const TCHAR* pIn, void* pOutValue)
{
    if (strlen(pIn) != 0)
    {
        std::vector<TString> cache;
        CStringHelper::GetInstance()->SplitString(pIn, _T(","), cache);
        BEATS_ASSERT(cache.size() == 2);
        ((TString*)pOutValue)->assign(pIn);
        for (uint32_t i = 0; i < m_pChildren->size(); ++i)
        {
            (*m_pChildren)[i]->SetValueByString(cache[i].c_str());
        }
    }
    return true;
}
