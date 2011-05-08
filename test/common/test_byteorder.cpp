#include "testutils.hpp"
#include "byteprinter.hpp"
#include "bytes.hpp"

#include <typeinfo>

DECLARE_TEST("byte order conversion");

// We need a little bit of #ifdef magic

#ifdef NUKE_MS_BIG_ENDIAN
    static const char big_end_symbol[] = "YES";
#else
    static const char big_end_symbol[] = "NO";
#endif


using nuke_ms::byte_traits;

int main()
{
    // display Endianness symbol
    std::cout<<"NUKE_MS_BIG_ENDIAN is defined: "<<big_end_symbol<<"\n\n";

    //
    byte_traits::uint4b_t ulong = 0x1A2B3C4D;
    byte_traits::uint4b_t ulong_reverse = nuke_ms::reversebytes(ulong);
    byte_traits::uint4b_t ulong_to_netbo = nuke_ms::to_netbo(ulong);

    TEST_ASSERT(ulong_reverse == 0x4D3C2B1A);
#ifdef NUKE_MS_BIG_ENDIAN
    TEST_ASSERT(ulong_to_netbo == 0x4D3C2B1A);
#else
    TEST_ASSERT(ulong_to_netbo == 0x1A2B3C4D);
#endif

    std::cout<<"Unconverted unsigned long 0x"<<std::hex<<ulong<<std::dec<<": "<<
        hexprint(&ulong, &ulong+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&ulong_reverse, &ulong_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&ulong_to_netbo, &ulong_to_netbo+1)<<'\n';

    byte_traits::int4b_t slong = -0x0708090A;
    byte_traits::int4b_t slong_reverse = nuke_ms::reversebytes(slong);
    byte_traits::int4b_t slong_to_netbo = nuke_ms::to_netbo(slong);

    TEST_ASSERT(slong_reverse == 0xF6F6F7F8);
#ifdef NUKE_MS_BIG_ENDIAN
    TEST_ASSERT(slong_to_netbo == 0xF6F6F7F8);
#else
    TEST_ASSERT(slong_to_netbo == 0xF8F7F6F6);
#endif

    std::cout<<"Unconverted signed long 0x"<<std::hex<<slong<<std::dec<<": "<<
        hexprint(&slong, &slong+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&slong_reverse, &slong_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&slong_to_netbo, &slong_to_netbo+1)<<'\n';


    byte_traits::uint2b_t ushort = 0x1122;
    byte_traits::uint2b_t ushort_reverse = nuke_ms::reversebytes(ushort);
    byte_traits::uint2b_t ushort_to_netbo = nuke_ms::to_netbo(ushort);

    TEST_ASSERT(ushort_reverse == 0x2211);
#ifdef NUKE_MS_BIG_ENDIAN
    TEST_ASSERT(ushort_to_netbo == 0x2211);
#else
    TEST_ASSERT(ushort_to_netbo == 0x1122);
#endif

    std::cout<<"Unconverted unsigned short 0x"<<std::hex<<ushort<<std::dec<<": "<<
        hexprint(&ushort, &ushort+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&ushort_reverse, &ushort_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&ushort_to_netbo,&ushort_to_netbo+1)<<'\n';

    byte_traits::int2b_t sshort = -0x1020;
    byte_traits::int2b_t sshort_reverse = nuke_ms::reversebytes(sshort);
    byte_traits::int2b_t sshort_to_netbo = nuke_ms::to_netbo(sshort);

    TEST_ASSERT(sshort_reverse == static_cast<byte_traits::int2b_t>(0xE0EF));
#ifdef NUKE_MS_BIG_ENDIAN
    TEST_ASSERT(sshort_to_netbo == static_cast<byte_traits::int2b_t>(0xE0EF));
#else
    TEST_ASSERT(sshort_to_netbo == static_cast<byte_traits::int2b_t>(0xEFE0));
#endif

    std::cout<<"Unconverted signed short 0x"<<std::hex<<sshort<<std::dec<<": "<<
        hexprint(&sshort, &sshort+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&sshort_reverse, &sshort_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&sshort_to_netbo,&sshort_to_netbo+1)<<'\n';


    return CONCLUDE_TEST();
}
