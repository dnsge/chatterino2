#include "KeyboardSettingsPage.hpp"

#include "util/LayoutCreator.hpp"

#include <QFormLayout>
#include <QLabel>

namespace chatterino {

namespace {
    QLabel *keyLabel(QString text)
    {
#ifdef Q_OS_MACOS
        text.replace("Ctrl", "⌘");
        text.replace("Alt", "⌥");
        text.replace("Meta", "⌃");
#endif
        return new QLabel(text);
    }
}  // namespace

KeyboardSettingsPage::KeyboardSettingsPage()
{
    auto layout =
        LayoutCreator<KeyboardSettingsPage>(this).setLayoutType<QVBoxLayout>();

    auto scroll = layout.emplace<QScrollArea>();

    this->setStyleSheet("QLabel, #container { background: #333 }");

    auto form = new QFormLayout(this);
    scroll->setWidgetResizable(true);
    auto widget = new QWidget();
    widget->setLayout(form);
    widget->setObjectName("container");
    scroll->setWidget(widget);

    form->addRow(keyLabel("Hold Ctrl"), new QLabel("Show resize handles"));
    form->addRow(keyLabel("Hold Ctrl + Alt"), new QLabel("Show split overlay"));

    form->addItem(new QSpacerItem(16, 16));
    form->addRow(keyLabel("Ctrl + ScrollDown/-"), new QLabel("Zoom out"));
    form->addRow(keyLabel("Ctrl + ScrollUp/+"), new QLabel("Zoom in"));
    form->addRow(keyLabel("Ctrl + 0"), new QLabel("Reset zoom size"));

    form->addItem(new QSpacerItem(16, 16));
    form->addRow(new QLabel("Ctrl + T"), new QLabel("Create new split"));
    form->addRow(new QLabel("Ctrl + W"), new QLabel("Close current split"));
    form->addRow(new QLabel("Ctrl + N"),
                 new QLabel("Open current split as a popup"));
    form->addRow(new QLabel("Ctrl + K"), new QLabel("Jump to split"));
    form->addRow(new QLabel("Ctrl + G"),
                 new QLabel("Reopen last closed split"));

    form->addRow(new QLabel("Ctrl + Shift + T"), new QLabel("Create new tab"));
    form->addRow(new QLabel("Ctrl + Shift + W"),
                 new QLabel("Close current tab"));
#ifdef Q_OS_MACOS
    form->addRow(keyLabel("Meta + H"),
#else
    form->addRow(keyLabel("Ctrl + H"),
#endif
                 new QLabel("Hide/Show similar messages (See General->R9K)"));

    form->addItem(new QSpacerItem(16, 16));
    form->addRow(keyLabel("Ctrl + 1/2/3/..."),
                 new QLabel("Select tab 1/2/3/..."));
    form->addRow(keyLabel("Ctrl + 9"), new QLabel("Select last tab"));
#ifdef Q_OS_MACOS
    form->addRow(keyLabel("Meta + Tab"), new QLabel("Select next tab"));
    form->addRow(keyLabel("Meta + Shift + Tab"),
                 new QLabel("Select previous tab"));
#else
    form->addRow(keyLabel("Ctrl + Tab"), new QLabel("Select next tab"));
    form->addRow(keyLabel("Ctrl + Shift + Tab"),
                 new QLabel("Select previous tab"));
#endif

    form->addRow(keyLabel("Alt + ←/↑/→/↓"),
                 new QLabel("Select left/upper/right/bottom split"));

    form->addItem(new QSpacerItem(16, 16));
    form->addRow(keyLabel("Ctrl + R"), new QLabel("Change channel"));
    form->addRow(keyLabel("Ctrl + F"), new QLabel("Search in current channel"));
    form->addRow(keyLabel("Ctrl + E"), new QLabel("Open Emote menu"));
    form->addRow(keyLabel("Ctrl + P"), new QLabel("Open Settings menu"));
    form->addRow(keyLabel("F5"),
                 new QLabel("Reload subscriber and channel emotes"));
    form->addRow(new QLabel("Ctrl + F5"), new QLabel("Reconnect channels"));

    form->addItem(new QSpacerItem(16, 16));
    form->addRow(new QLabel("PageUp"), new QLabel("Scroll up"));
    form->addRow(new QLabel("PageDown"), new QLabel("Scroll down"));
}

}  // namespace chatterino
