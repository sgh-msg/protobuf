# Protocol Buffers Python

This directory contains the Protobuf library for Python.

In most cases you should install the library using `pip` or another
package manager:

```
$ pip install protobuf
```

The packages released on https://pypi.org/project/protobuf/#files include both
a source distribution and binary wheels.

For user documentation about how to use Protobuf Python, please see
https://protobuf.dev/getting-started/pythontutorial/

# Building packages from this repo

If for some reason you wish to build the packages directly from this repo, you can
use the following Bazel commands:

```
$ bazel build @upb//python/dist:source_wheel
$ bazel build @upb//python/dist:binary_wheel
```

The binary wheel will build against whatever version of Python is installed on
your system.  The source package is always the same and does not depend on a
local version of Python.

# Implementation backends

There are three separate implementations of Python Protobuf.  All of them offer
the same API and are thus functionally the same, though they have very different
performance characteristics.

The runtime library contains a switching layer that can choose between these
backends at runtime.  Normally it will choose between them automatically, using
priority-ordered list, and skipping any backends that are not available.  However
you can request a specific backend by setting the `PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION` environment variable to one of the
following values:



1. **upb**: Built on the [upb C
library](https://github.com/protocolbuffers/upb), this is a new extenion module
[released in 4.21.0](https://protobuf.dev/news/2022-05-06/).  It offers better
performance than any of the previous backends, and it is now the default.  The
code for this module lives in
[@upb/python](https://github.com/protocolbuffers/upb/tree/main/python).
1. **python**: The pure-Python backend, this does not require any extension
module to be present on the system.  The code for the pure-Python backend
lives in [google/protobuf/internal](google/protobuf/internal)
1. **cpp**: This backend wraps messages from the C++ protobuf library.  It is
deprecated and is no longer released in our PyPI packages, however it is still used
in some legacy cases where apps want to perform zero-copy message sharing between
Python and C++.  The code for this module lives in [google/protobuf/pyext](https://github.com/protocolbuffers/protobuf/tree/main/python/google/protobuf/pyext).

More information about sharing messages between Python and C++ is available here:
https://protobuf.dev/reference/python/python-generated/#sharing-messages

# Code generator

The code for the Protobuf Python code generator lives in [//src/google/protobuf/compiler/python](https://github.com/protocolbuffers/protobuf/tree/main/src/google/protobuf/compiler/python).  The code generator can output two different files for
each proto `foo.proto`:

* **foo_pb2.py**: The module you import to actually use the protos.
* **foo_pb2.pyi**: A stub file that describes the interface of the protos.

The pyi file is useful for IDEs or for users who want to read the output file.
The py file is optimized for fast loading and is not readable at all.

Please note that the pyi file is only generated if you pass the `pyi_out`
option to `protoc`:

```
$ protoc --python_out=pyi_out:output_dir
```
