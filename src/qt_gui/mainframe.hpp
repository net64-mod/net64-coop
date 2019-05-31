#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#ifndef Q_MOC_RUN
#include <experimental/filesystem>
#include <future>
#include <optional>
#include <thread>
#include <QStandardPaths>
#include <QWidget>
#include "core/emulator/m64plus.hpp"
#include "core/logging.hpp"
#endif


namespace Ui {
class MainFrame;
}

namespace Frontend
{

namespace fs = std::experimental::filesystem;

class MainFrame : public QWidget
{
Q_OBJECT

public:
    explicit MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;

    inline static const char* M64P_PLUGIN_PATH{"../emulator/mupen64plus/"},
                            * M64P_CONFIG_SUB_PATH{"config/mupen64plus/config/"},
                            * M64P_DATA_SUB_PATH{"config/mupen64plus/data/"},
                            * MAIN_CONFIG_FILE_SUB_PATH{"config/config.json"};


private slots:
    void on_tbx_emu_path_returnPressed();
    void on_btn_start_emu_clicked();

private:
    Ui::MainFrame* ui;
    std::optional<Core::Emulator::Mupen64Plus> emu_;
    std::future<void> emulation_thread_;

    fs::path user_config_path{QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString()};

    CLASS_LOGGER_("frontend");
};

} // Frontend

#endif // MAINFRAME_HPP
