#pragma once

#include <HMUI/ModalView.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <UnityEngine/Transform.hpp>
#include <bsml/shared/BSML/Components/Keyboard/ModalKeyboard.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <functional>
#include <memory>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Players::UI, QuestPairingModal, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ModalView>, modal);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, codeText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, statusText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<BSML::ModalKeyboard>, pairingKeyboard);

    DECLARE_INSTANCE_METHOD(StringW, get_pairingCode);
    DECLARE_INSTANCE_METHOD(void, set_pairingCode, StringW value);
    DECLARE_INSTANCE_METHOD(void, CodeEntered, StringW value);
    DECLARE_INSTANCE_METHOD(void, PasteCode);
    DECLARE_INSTANCE_METHOD(void, SubmitCode);
    DECLARE_INSTANCE_METHOD(void, OpenPairingPage);
    DECLARE_INSTANCE_METHOD(void, Close);
    DECLARE_CTOR(ctor);

  public:
    static QuestPairingModal* Create(UnityEngine::Transform* parent);
    void Show();
    void Hide();

    // fired on the main thread after a successful pairing + sign-in
    std::function<void()> onPaired;

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    std::string _pairingCode;
    bool _busy = false;

    void SetCode(const std::string& value);
    void SetStatus(const std::string& value);
    void ApplyCodeDisplay();
};
