#include "Features/Live/Compete/UI/ViewControllers/CodeEntry/CompeteCodeEntryViewController.hpp"

#include "assets.hpp"

#include <UnityEngine/GUIUtility.hpp>
#include <bsml/shared/BSML.hpp>

#include <algorithm>
#include <cctype>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::ViewControllers::CodeEntry, CompeteCodeEntryViewController);

namespace SnoreSaber::Features::Live::Compete::UI::ViewControllers::CodeEntry
{
    namespace
    {
        std::string TrimToLower(const std::string& value)
        {
            auto begin = value.find_first_not_of(" \t\r\n");
            if (begin == std::string::npos)
            {
                return "";
            }

            auto end = value.find_last_not_of(" \t\r\n");
            std::string result = value.substr(begin, end - begin + 1);
            std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
            return result;
        }
    }

    void CompeteCodeEntryViewController::ctor()
    {
        INVOKE_CTOR();
    }

    void CompeteCodeEntryViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::CompeteCodeEntryViewController_bsml, transform, this);
        }

        ApplyJoinCodeDisplay();
        ApplyStatus();
    }

    StringW CompeteCodeEntryViewController::get_joinCode()
    {
        return _joinCode;
    }

    void CompeteCodeEntryViewController::set_joinCode(StringW value)
    {
        SetJoinCode(value ? static_cast<std::string>(value) : "");
    }

    void CompeteCodeEntryViewController::CodeEntered(StringW value)
    {
        SetJoinCode(TrimToLower(value ? static_cast<std::string>(value) : ""));
    }

    void CompeteCodeEntryViewController::PasteCode()
    {
        StringW clipboard = UnityEngine::GUIUtility::get_systemCopyBuffer();
        std::string code = TrimToLower(clipboard ? static_cast<std::string>(clipboard) : "");
        if (code.empty())
        {
            SetStatus("Clipboard is empty.");
            return;
        }

        SetJoinCode(code);
        SetStatus("");
    }

    void CompeteCodeEntryViewController::JoinCode()
    {
        if (_joinCode.empty())
        {
            SetStatus("Enter a room code first.");
            return;
        }

        SetStatus("");
        JoinRequested.Invoke(_joinCode);
    }

    void CompeteCodeEntryViewController::Reset()
    {
        SetJoinCode("");
        SetStatus("");
    }

    void CompeteCodeEntryViewController::SetStatus(const std::string& value)
    {
        _status = value;
        ApplyStatus();
    }

    void CompeteCodeEntryViewController::SetJoinCode(const std::string& value)
    {
        _joinCode = value;
        ApplyJoinCodeDisplay();
    }

    void CompeteCodeEntryViewController::ApplyJoinCodeDisplay()
    {
        if (joinCodeText)
        {
            if (_joinCode.empty())
            {
                joinCodeText->text = "--------";
            }
            else
            {
                joinCodeText->text = _joinCode;
            }
        }
    }

    void CompeteCodeEntryViewController::ApplyStatus()
    {
        if (statusText)
        {
            statusText->text = _status;
        }
    }
}
