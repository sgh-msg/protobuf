// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "google/protobuf/compiler/rust/generator.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/substitute.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace rust {

bool ExperimentalRustGeneratorEnabled(
    const std::vector<std::pair<std::string, std::string>>& options) {
  std::pair<std::string, std::string> magic_value = {"experimental-codegen",
                                                     "enabled"};
  return absl::c_any_of(
      options, [&magic_value](auto& pair) { return pair == magic_value; });
}

bool RustGenerator::Generate(const FileDescriptor* file,
                             const std::string& parameter,
                             GeneratorContext* generator_context,
                             std::string* error) const {
  std::vector<std::pair<std::string, std::string>> options;
  ParseGeneratorParameter(parameter, &options);

  if (!ExperimentalRustGeneratorEnabled(options)) {
    *error =
        "The Rust codegen is highly experimental. Future versions will break "
        "existing code. Use at your own risk. You can opt-in by passing "
        "'experimental-codegen=enabled' to '--rust_out'.";
    return false;
  }

  auto basename = StripProto(file->name());
  auto outfile = absl::WrapUnique(
      generator_context->Open(absl::StrCat(basename, ".pb.rs")));

  google::protobuf::io::Printer printer(outfile.get());
  // TODO(b/270138878): Remove `do_nothing` import once we have real logic. This
  // is there only to smoke test rustc actions in rust_proto_library.
  printer.Print(R"cc(
#[allow(unused_imports)]
    use protobuf::do_nothing;
  )cc");
  for (int i = 0; i < file->message_type_count(); i++) {
    // TODO(b/270138878): Implement real logic
    printer.Print(R"cc(
                    pub struct $type_name$ {}
                  )cc",
                  "type_name", file->message_type(i)->name());
  }
  // TODO(b/270124215): Delete the following "placeholder impl" of `import
  // public`. Also make sure to figure out how to map FileDescriptor#name to
  // Rust crate names (currently Bazel labels).
  for (int i = 0; i < file->public_dependency_count(); i++) {
    const FileDescriptor* dep = file->public_dependency(i);
    std::string basename(dep->name().substr(dep->name().rfind('/') + 1));
    std::string crate_name = absl::StrReplaceAll(basename, {
                                                               {".", "_"},
                                                               {"-", "_"},
                                                           });
    for (int j = 0; j < dep->message_type_count(); j++) {
      // TODO(b/270138878): Implement real logic
      printer.Print(R"cc(
                      pub use $crate_name$::$type_name$;
                    )cc",
                    "crate_name", crate_name, "type_name",
                    dep->message_type(j)->name());
    }
  }

  return true;
}

}  // namespace rust
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
