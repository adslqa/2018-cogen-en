#include "Parser.h"

#include <cassert>
#include <iostream>

template<size_t N>
auto makeData(const char (&v)[N]) {
    auto p = reinterpret_cast<const Byte *>(&v[0]);
    return DataView{p, p + N - 1};
}

template<size_t IN, size_t CP>
void decoderTest(const char (&in)[IN],
                 const CodePoint (&cp)[CP]) {
    auto data = makeData(in);

    auto n = 0u;
    for (auto p : utf8Decode(data)) {
        assert(p.codePoint == cp[n]);
        n++;
    }
    assert(n == CP);
}

void testDecoder() { //
    decoderTest({(char)0xFF, 'a', 0}, {0, 'a'});
    decoderTest("a", {'a'});
    decoderTest(R"(🍌a)", {0x1f34c, 'a'});
}

template<size_t N>
auto makeCodePoints(const CodePoint (&cp)[N])
    -> Generator<FileChar> {
    auto p = reinterpret_cast<const CodePoint *>(cp);
    struct Wrapper {
        Wrapper(const CodePoint *p)
            : p(p) {}

        operator bool() const { return n < N; }
        auto next() -> FileChar {
            auto tmp = FileChar{
                p[n],
                reinterpret_cast<const Byte *>(&p[n])};
            n++;
            return tmp;
        }

        size_t n{};
        const CodePoint *p;
    };
    return {Wrapper{p}};
}

template<size_t CP, size_t TOK>
void scannerTest(const CodePoint (&cp)[CP],
                 const TokenVariant (&tok)[TOK]) {
    auto input = makeCodePoints(cp);

    auto n = 0u;
    for (auto t : scan({}, std::move(input))) {
        assert(t.kind == tok[n]);
        n++;
    }
    assert(n == TOK);
}

void testScanner() {
    scannerTest({'1', '2'}, {Number{12}});
    scannerTest({'4', ' ', '+', '\n', '5'},
                {Number{4}, WhiteSpace{}, Plus{}, NewLine{},
                 Number{5}});
}

auto main() -> int {
    testDecoder();
    testScanner();
}
