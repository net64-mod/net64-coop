#pragma once

#include <QDialog>
#include "qt_gui/vcdiff_patcher.hpp"


namespace Ui {
class PatchDialog;
}

namespace Frontend
{

struct PatchDialog : QDialog
{
    Q_OBJECT

public:
    explicit PatchDialog(QWidget* parent = nullptr);
    ~PatchDialog() override;

private slots:
    void on_browse_original();
    void on_browse_second();
    void on_finished(bool success, std::string result);
    void on_create_patch();
    void on_mode_changed(bool apply_patch);

private:
    Ui::PatchDialog* ui;
    VCdiffThread vcdiff_thread_;
    bool apply_patch_{true};
};

}
