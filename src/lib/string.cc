#include <cstring>

namespace coral { 

  class ByteString {
  public:
    int length;
    char * chars;

    ByteString(char * chars, int length) : chars(chars), length(length) { }
  };

  extern "C" ByteString * ByteString_create(char * chars, int length) {
    char * buf = new char[length + 1];
    strncpy(buf, chars, length);
    buf[length] = 0;
    return new ByteString(buf, length);
  }

  extern "C" int ByteString_len(ByteString * str) { return str->length; }

  extern "C" char * ByteString_unsafe_cstr(ByteString * str) {
    return str->chars;
  }

}
