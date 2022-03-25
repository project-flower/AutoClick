#include "Settings.h"
#include <vector>
#include "Constants.h"
#include "PrivateProfileManager.h"

typedef std::vector<PrivateProfile::Section> SectionCollection;
typedef std::vector<PrivateProfile::KeyAndValue> ValueCollection;

Settings::Settings() :
    bCheckClassName(false)
{
}

Settings Settings::Load(const TCHAR* szFileName)
{
    PrivateProfile profile = PrivateProfileManager::Load(szFileName);
    Settings result;
    SectionCollection* pSections = &profile.Sections;

    for (SectionCollection::iterator itSection = pSections->begin(); itSection != pSections->end(); ++itSection) {
        if (itSection->Name != Constants::Settings::SECTION_NAME) {
            continue;
        }

        ValueCollection* pValues = &itSection->Values;

        for (ValueCollection::iterator itValue = pValues->begin(); itValue != pValues->end(); ++itValue) {
            if ((itValue->Key == Constants::Settings::CHECK_CLASS_NAME) &&
                (itValue->Value == Constants::Settings::BOOL_TRUE)) {
                result.bCheckClassName = true;
            }
        }
    }

    return result;
}
