//
// Created by henrik on 16.10.19.
// Copyright 2019 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <string>
#include <vector>
#include <QThread>
#include <google/format_extension_flags.h>


namespace Frontend
{

struct VCdiffPatcher : QObject
{
    Q_OBJECT

public slots:
    void apply_delta(std::vector<std::byte> original,
                     std::string delta);

    void create_delta(std::vector<std::byte> original,
                      std::vector<std::byte> target,
                      open_vcdiff::VCDiffFormatExtensionFlags flags = open_vcdiff::VCD_STANDARD_FORMAT);

signals:
    void finished(bool success, std::string);
};

struct VCdiffThread : QObject
{
    Q_OBJECT

public:
    VCdiffThread();
    ~VCdiffThread() override;

    void start_decode(const void* original,
                     std::size_t orig_size,
                     std::string delta);

    void start_encode(const void* original,
                      std::size_t orig_size,
                      const void* target,
                      std::size_t target_size,
                      open_vcdiff::VCDiffFormatExtensionFlags flags = open_vcdiff::VCD_STANDARD_FORMAT);

signals:
    void finished(bool success,
                  std::string result);

    void apply(std::vector<std::byte>,
               std::string);

    void create(std::vector<std::byte>,
                std::vector<std::byte>,
                open_vcdiff::VCDiffFormatExtensionFlags);

private slots:
    void handle_result(bool success, std::string result);

private:
    QThread thread_;
};

}
