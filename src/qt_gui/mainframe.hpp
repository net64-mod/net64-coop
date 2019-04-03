#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <QWidget>
#include <spdlog/spdlog.h>
#include "core/emulator/emulator_handle.hpp"


namespace Ui {
class MainFrame;
}

namespace Frontend
{

using LoggerPtr = std::shared_ptr<spdlog::logger>;

class MainFrame : public QWidget
{
Q_OBJECT

public:
    explicit MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;

private slots:
    void on_tbx_emu_path_returnPressed();
    void on_btn_start_emu_clicked();
    void on_btn_base_addr_clicked();

private:
    Ui::MainFrame* ui;
    Core::Emulator emu_;
    mutable LoggerPtr logger_{spdlog::get("Frontend")};
};

} // Frontend

#endif // MAINFRAME_HPP
