#pragma once

#include "widgets/settingspages/SettingsPage.hpp"

#include <QStringListModel>

class QVBoxLayout;

namespace chatterino {

class FiltersPage : public SettingsPage
{
public:
    FiltersPage();

    void onShow() final;

private:
    QStringListModel userListModel_;
};

}  // namespace chatterino
