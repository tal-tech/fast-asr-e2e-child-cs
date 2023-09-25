// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_H_
#define BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_H_

#include <stddef.h>
#include <stdint.h>

#include "base/base_export.h"
#include "base/containers/hash_tables.h"

namespace base {
namespace trace_event {

// When heap profiling is enabled, tracing keeps track of the allocation
// context for each allocation intercepted. It is generated by the
// |AllocationContextTracker| which keeps stacks of context in TLS.
// The tracker is initialized lazily.

// The backtrace in the allocation context is a snapshot of the stack. For now,
// this is the pseudo stack where frames are created by trace event macros. In
// the future, we might add the option to use the native call stack. In that
// case, |Backtrace| and |AllocationContextTracker::GetContextSnapshot| might
// have different implementations that can be selected by a compile time flag.

// The number of stack frames stored in the backtrace is a trade off between
// memory used for tracing and accuracy. Measurements done on a prototype
// revealed that:
//
// - In 60 percent of the cases, stack depth <= 7.
// - In 87 percent of the cases, stack depth <= 9.
// - In 95 percent of the cases, stack depth <= 11.
//
// See the design doc (https://goo.gl/4s7v7b) for more details.

using StackFrame = const char*;

struct BASE_EXPORT Backtrace {
  // Unused backtrace frames are filled with nullptr frames. If the stack is
  // higher than what can be stored here, the bottom frames are stored. Based
  // on the data above, a depth of 12 captures the full stack in the vast
  // majority of the cases.
  StackFrame frames[12];
};

bool BASE_EXPORT operator==(const Backtrace& lhs, const Backtrace& rhs);

// The |AllocationContext| is context metadata that is kept for every allocation
// when heap profiling is enabled. To simplify memory management for book-
// keeping, this struct has a fixed size. All |const char*|s here must have
// static lifetime.
struct BASE_EXPORT AllocationContext {
 public:
  // An allocation context with empty backtrace and unknown type.
  static AllocationContext Empty();

  Backtrace backtrace;

  // Type name of the type stored in the allocated memory. A null pointer
  // indicates "unknown type". Grouping is done by comparing pointers, not by
  // deep string comparison. In a component build, where a type name can have a
  // string literal in several dynamic libraries, this may distort grouping.
  const char* type_name;

 private:
  friend class AllocationContextTracker;

  // Don't allow uninitialized instances except inside the allocation context
  // tracker. Except in tests, an |AllocationContext| should only be obtained
  // from the tracker. In tests, paying the overhead of initializing the struct
  // to |Empty| and then overwriting the members is not such a big deal.
  AllocationContext();
};

bool BASE_EXPORT operator==(const AllocationContext& lhs,
                            const AllocationContext& rhs);

}  // namespace trace_event
}  // namespace base

namespace BASE_HASH_NAMESPACE {

template <>
struct BASE_EXPORT hash<base::trace_event::Backtrace> {
  size_t operator()(const base::trace_event::Backtrace& backtrace) const;
};

template <>
struct BASE_EXPORT hash<base::trace_event::AllocationContext> {
  size_t operator()(const base::trace_event::AllocationContext& context) const;
};

}  // BASE_HASH_NAMESPACE

#endif  // BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_H_
