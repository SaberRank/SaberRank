#include "Features/Players/UI/QuestPairingModal.hpp"

#include "Core/Api/SnoreSaberUrls.hpp"
#include "Features/Players/Services/DevicePairingService.hpp"
#include "Utils/StrippedMethods.hpp"
#include "assets.hpp"

#include <UnityEngine/GUIUtility.hpp>
#include <UnityEngine/Vector2.hpp>
#include <beatsaber-hook/shared/utils/il2cpp-utils.hpp>
#include <bsml/shared/BSML-Lite.hpp>
#include <bsml/shared/BSML.hpp>

DEFINE_TYPE(SnoreSaber::Features::Players::UI, QuestPairingModal);

namespace SnoreSaber::Features::Players::UI
{
    namespace
    {
        constexpr auto ErrorColor = "<color=#fc8181>";
        constexpr auto SuccessColor = "<color=#89fc81>";
    }

    void QuestPairingModal::ctor()
    {
        INVOKE_CTOR();
    }

    QuestPairingModal* QuestPairingModal::Create(UnityEngine::Transform* parent)
    {
        auto modal = BSML::Lite::CreateModal(parent, UnityEngine::Vector2(80.0f, 62.0f), nullptr);
        auto pairingModal = modal->gameObject->AddComponent<QuestPairingModal*>();
        pairingModal->modal = modal;
        pairingModal->_parser = BSML::parse_and_construct(IncludedAssets::QuestPairingModal_bsml, pairingModal->transform, pairingModal);

        // a modal-keyboard nested inside an HMUI modal never shows; host it at the
        // view root like the chat and compete keyboards
        if (pairingModal->pairingKeyboard)
        {
            pairingModal->pairingKeyboard->transform->SetParent(parent, false);
        }

        return pairingModal;
    }

    void QuestPairingModal::Show()
    {
        SetStatus("");
        modal->Show(true, true, nullptr);
    }

    void QuestPairingModal::Hide()
    {
        modal->Hide(true, nullptr);
    }

    StringW QuestPairingModal::get_pairingCode()
    {
        return _pairingCode;
    }

    void QuestPairingModal::set_pairingCode(StringW value)
    {
        SetCode(value ? static_cast<std::string>(value) : "");
    }

    void QuestPairingModal::CodeEntered(StringW value)
    {
        SetCode(value ? static_cast<std::string>(value) : "");
    }

    void QuestPairingModal::PasteCode()
    {
        StringW clipboard = UnityEngine::GUIUtility::get_systemCopyBuffer();
        std::string code = clipboard ? static_cast<std::string>(clipboard) : "";
        if (code.empty())
        {
            SetStatus(std::string(ErrorColor) + "Clipboard is empty");
            return;
        }

        SetCode(code);
        SetStatus("");
    }

    void QuestPairingModal::OpenPairingPage()
    {
        StrippedMethods::UnityEngine::Application::OpenURL(Core::Api::SnoreSaberUrls::QuestPairing());
        SetStatus("Sign in on the page that just opened, then enter the code it shows");
    }

    void QuestPairingModal::Close()
    {
        Hide();
    }

    void QuestPairingModal::SubmitCode()
    {
        if (_busy)
        {
            return;
        }

        auto code = Services::DevicePairing::NormalizeCode(_pairingCode);
        if (!code.has_value())
        {
            SetStatus(std::string(ErrorColor) + "Pairing codes are 12 letters and digits. Check the code and try again");
            return;
        }

        _busy = true;
        SetStatus("Pairing with SnoreSaber...");

        SafePtrUnity<QuestPairingModal> self(this);
        Services::DevicePairing::PairWithCode(code.value(), [self](Services::DevicePairing::PairingResult result) {
            if (!self)
            {
                return;
            }

            self->_busy = false;
            if (!result.success)
            {
                self->SetStatus(std::string(ErrorColor) + result.message);
                return;
            }

            self->SetCode("");
            self->SetStatus(std::string(SuccessColor) + result.message);
            if (self->onPaired)
            {
                self->onPaired();
            }
        });
    }

    void QuestPairingModal::SetCode(const std::string& value)
    {
        _pairingCode = value;
        ApplyCodeDisplay();
    }

    void QuestPairingModal::SetStatus(const std::string& value)
    {
        if (statusText)
        {
            statusText->text = value;
        }
    }

    void QuestPairingModal::ApplyCodeDisplay()
    {
        if (codeText)
        {
            codeText->text = _pairingCode.empty() ? "------------" : _pairingCode;
        }
    }
}
