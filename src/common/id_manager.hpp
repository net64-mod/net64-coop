//
// Created by henrik on 12.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <limits>
#include <stdexcept>
#include <vector>


template<typename Id>
struct IdManager
{
    Id acquire_id()
    {
        if(!free_ids_.empty())
        {
            Id id{free_ids_.back()};
            free_ids_.pop_back();
            return id;
        }

        if(new_id_ == std::numeric_limits<Id>::max())
            throw std::runtime_error("No ids left");

        return ++new_id_;
    }

    void return_id(Id id)
    {
        if(id == new_id_)
            --new_id_;
        else
            free_ids_.push_back(id);
    }

private:
    Id new_id_{};
    std::vector<Id> free_ids_;
};

template<typename Id>
struct IdHandle
{
    IdHandle() = default;

    explicit IdHandle(IdManager<Id>& mgr): manager_(&mgr), id_(manager_->acquire_id()) {}

    IdHandle(const IdHandle&) = delete;

    IdHandle(IdHandle&& other) noexcept: IdHandle() { swap(*this, other); }

    IdHandle& operator=(IdHandle&& other) noexcept
    {
        swap(*this, other);

        return *this;
    }

    ~IdHandle()
    {
        if(manager_)
            manager_->return_id(id_);
    }

    friend void swap(IdHandle& first, IdHandle& second)
    {
        using std::swap;

        swap(first.manager_, second.manager_);
        swap(first.id_, second.id_);
    }

    bool has_id() const { return (manager_ != nullptr && id_ != 0); }

    Id id() const { return id_; }

private:
    IdManager<Id>* manager_{};
    Id id_{0};
};
