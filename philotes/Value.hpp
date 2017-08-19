#pragma once

#include <cstddef>

using std::size_t;

union Value
{
     int64_t numeric_val;
     double float_val;
     struct
     {
          size_t length;
          void* ptr;
     } ary;

     const char* as_string() { return static_cast<char*>(ary.ptr); }
};
