/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set ts=8 sts=4 et sw=4 tw=80: */
/*
 * Copyright (C) 2015. Jefferson Lab, xMsg framework (JLAB). All Rights Reserved.
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement.
 *
 * Contact Vardan Gyurjyan
 * Department of Experimental Nuclear Physics, Jefferson Lab.
 *
 * IN NO EVENT SHALL JLAB BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF
 * THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF JLAB HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * JLAB SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE CLARA SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
 * HEREUNDER IS PROVIDED "AS IS". JLAB HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef XMSG_PROTO_META_H_
#define XMSG_PROTO_META_H_

#include "meta.pb.h"

#include <memory>
#include <stdexcept>

namespace xmsg {

/**
 * Protobuf data classes and helpers.
 */
namespace proto {

namespace internal {

inline void set_datatype(Meta& meta, const char* datatype)
{
    if (datatype) {
        meta.set_datatype(datatype);
    } else {
        throw std::invalid_argument{"null mime-type"};
    }
}

inline void set_datatype(Meta& meta, const std::string& datatype)
{
    meta.set_datatype(datatype);
}

} // end namespace internal


inline std::unique_ptr<Meta> make_meta()
{
    return std::make_unique<Meta>();
}

inline std::unique_ptr<Meta> copy_meta(const Meta& meta)
{
    return std::make_unique<Meta>(meta);
}


inline bool operator==(const Meta& lhs, const Meta& rhs)
{
    return lhs.SerializeAsString() == rhs.SerializeAsString();
}

inline bool operator!=(const Meta& lhs, const Meta& rhs)
{
    return !(lhs == rhs);
}

} // end namespace proto
} // end namespace xmsg

#endif // XMSG_PROTO_META_H_
