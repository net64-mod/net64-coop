//
// Created by henrik on 16.10.19.
// Copyright 2019 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "vcdiff_patcher.hpp"

#include <google/vcdecoder.h>
#include <google/vcencoder.h>


namespace Frontend
{

void VCdiffPatcher::apply_delta(std::vector<std::byte> original,
                                std::string delta)
{
    open_vcdiff::VCDiffDecoder decoder;
    std::string res;
    auto success{decoder.Decode(reinterpret_cast<const char*>(original.data()), original.size(), delta, &res)};
    finished(success, std::move(res));
}

void VCdiffPatcher::create_delta(std::vector<std::byte> original,
                                 std::vector<std::byte> target,
                                 open_vcdiff::VCDiffFormatExtensionFlags flags)
{
    open_vcdiff::VCDiffEncoder encoder(reinterpret_cast<const char*>(original.data()), original.size());
    encoder.SetFormatFlags(flags);
    encoder.SetTargetMatching(true);
    std::string res;
    auto success{encoder.Encode(reinterpret_cast<const char*>(target.data()), target.size(), &res)};
    finished(success, std::move(res));
}

VCdiffThread::VCdiffThread()
{
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<std::vector<std::byte>>("std::vector<std::byte>");
    qRegisterMetaType<open_vcdiff::VCDiffFormatExtensionFlags>("open_vcdiff::VCDiffFormatExtensionFlags");

    auto* patcher{new VCdiffPatcher};

    patcher->moveToThread(&thread_);
    connect(&thread_, &QThread::finished, patcher, &QObject::deleteLater);
    connect(this, &VCdiffThread::apply, patcher, &VCdiffPatcher::apply_delta);
    connect(this, &VCdiffThread::create, patcher, &VCdiffPatcher::create_delta);
    connect(patcher, &VCdiffPatcher::finished, this, &VCdiffThread::handle_result);

    thread_.start();
}

VCdiffThread::~VCdiffThread()
{
    thread_.quit();
    thread_.wait();
}

void VCdiffThread::start_decode(const void* original,
                                std::size_t orig_size,
                                std::string delta)
{
    apply({reinterpret_cast<const std::byte*>(original),
           reinterpret_cast<const std::byte*>(original) + orig_size},
          std::move(delta));
}

void VCdiffThread::start_encode(const void* original,
                                std::size_t orig_size,
                                const void* target,
                                std::size_t target_size,
                                open_vcdiff::VCDiffFormatExtensionFlags flags)
{
    create({reinterpret_cast<const std::byte*>(original),
            reinterpret_cast<const std::byte*>(original) + orig_size},
           {reinterpret_cast<const std::byte*>(target),
            reinterpret_cast<const std::byte*>(target) + target_size},
           flags);
}

void VCdiffThread::handle_result(bool success, std::string result)
{
    finished(success, std::move(result));
}

}
