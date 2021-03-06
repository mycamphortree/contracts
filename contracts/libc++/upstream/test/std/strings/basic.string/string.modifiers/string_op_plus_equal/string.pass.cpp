//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string<charT,traits,Allocator>&
//   operator+=(const basic_string<charT,traits,Allocator>& str);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, S str, S expected)
{
    s += str;
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

int main()
{
    {
    typedef std::string S;
    test(S(), S(), S());
    test(S(), S("12345"), S("12345"));
    test(S(), S("1234567890"), S("1234567890"));
    test(S(), S("12345678901234567890"), S("12345678901234567890"));

    test(S("12345"), S(), S("12345"));
    test(S("12345"), S("12345"), S("1234512345"));
    test(S("12345"), S("1234567890"), S("123451234567890"));
    test(S("12345"), S("12345678901234567890"), S("1234512345678901234567890"));

    test(S("1234567890"), S(), S("1234567890"));
    test(S("1234567890"), S("12345"), S("123456789012345"));
    test(S("1234567890"), S("1234567890"), S("12345678901234567890"));
    test(S("1234567890"), S("12345678901234567890"), S("123456789012345678901234567890"));

    test(S("12345678901234567890"), S(), S("12345678901234567890"));
    test(S("12345678901234567890"), S("12345"), S("1234567890123456789012345"));
    test(S("12345678901234567890"), S("1234567890"), S("123456789012345678901234567890"));
    test(S("12345678901234567890"), S("12345678901234567890"),
         S("1234567890123456789012345678901234567890"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(), S(), S());
    test(S(), S("12345"), S("12345"));
    test(S(), S("1234567890"), S("1234567890"));
    test(S(), S("12345678901234567890"), S("12345678901234567890"));

    test(S("12345"), S(), S("12345"));
    test(S("12345"), S("12345"), S("1234512345"));
    test(S("12345"), S("1234567890"), S("123451234567890"));
    test(S("12345"), S("12345678901234567890"), S("1234512345678901234567890"));

    test(S("1234567890"), S(), S("1234567890"));
    test(S("1234567890"), S("12345"), S("123456789012345"));
    test(S("1234567890"), S("1234567890"), S("12345678901234567890"));
    test(S("1234567890"), S("12345678901234567890"), S("123456789012345678901234567890"));

    test(S("12345678901234567890"), S(), S("12345678901234567890"));
    test(S("12345678901234567890"), S("12345"), S("1234567890123456789012345"));
    test(S("12345678901234567890"), S("1234567890"), S("123456789012345678901234567890"));
    test(S("12345678901234567890"), S("12345678901234567890"),
         S("1234567890123456789012345678901234567890"));
    }
#endif
}
