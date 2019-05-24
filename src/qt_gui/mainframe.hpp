#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <future>
#include <optional>
#include <thread>
#include <QWidget>
#include "core/emulator/m64plus.hpp"
#include "core/logging.hpp"


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

private:
    Ui::MainFrame* ui;
    std::optional<Core::Emulator::M64Plus> emu_;
    std::future<void> execution_thread_;

    CLASS_LOGGER_("frontend");
};

} // Frontend

#endif // MAINFRAME_HPP
