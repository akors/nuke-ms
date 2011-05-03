#include "byteprinter.hpp"
#include "bytes.hpp"

#include <typeinfo>

// We need a little bit of #ifdef magic

#ifdef NUKE_MS_BIG_ENDIAN
    static const char big_end_symbol[] = "YES";
#else
    static const char big_end_symbol[] = "NO";
#endif //

#define PRETTY_INT_LONG 0x1A2B3C4D
#define NEGATIVE_LONG -0x0708090A

#define PRETTY_INT_SHORT 0x1122
#define NEGATIVE_SHORT -0x1020

using nuke_ms::byte_traits;

int main()
{
    std::cout<<" ----- Testing byte order conversion  ----- \n";
    std::cout<<"NUKE_MS_BIG_ENDIAN is defined: "<<big_end_symbol<<"\n\n";

    byte_traits::uint4b_t ulong = PRETTY_INT_LONG;
    byte_traits::uint4b_t ulong_reverse = nuke_ms::reversebytes(ulong);
    byte_traits::uint4b_t ulong_to_netbo = nuke_ms::to_netbo(ulong);


    std::cout<<"Unconverted unsigned long 0x"<<std::hex<<ulong<<": "<<
        hexprint(&ulong, &ulong+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&ulong_reverse, &ulong_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&ulong_to_netbo, &ulong_to_netbo+1)<<'\n';

    byte_traits::int4b_t slong = NEGATIVE_LONG;
    byte_traits::int4b_t slong_reverse = nuke_ms::reversebytes(slong);
    byte_traits::int4b_t slong_to_netbo = nuke_ms::to_netbo(slong);

    std::cout<<"Unconverted signed long 0x"<<std::hex<<slong<<": "<<
        hexprint(&slong, &slong+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&slong_reverse, &slong_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&slong_to_netbo, &slong_to_netbo+1)<<'\n';


    byte_traits::uint2b_t ushort = PRETTY_INT_SHORT;
    byte_traits::uint2b_t ushort_reverse = nuke_ms::reversebytes(ushort);
    byte_traits::uint2b_t ushort_to_netbo = nuke_ms::to_netbo(ushort);

    std::cout<<"Unconverted unsigned short 0x"<<std::hex<<ushort<<": "<<
        hexprint(&ushort, &ushort+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&ushort_reverse, &ushort_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&ushort_to_netbo,&ushort_to_netbo+1)<<'\n';

    byte_traits::int2b_t sshort = NEGATIVE_SHORT;
    byte_traits::int2b_t sshort_reverse = nuke_ms::reversebytes(sshort);
    byte_traits::int2b_t sshort_to_netbo = nuke_ms::to_netbo(sshort);

    std::cout<<"Unconverted signed short 0x"<<std::hex<<sshort<<": "<<
        hexprint(&sshort, &sshort+1)<<'\n'<<
        '\t'<<"Reversed: "<<hexprint(&sshort_reverse, &sshort_reverse+1)<<'\n'<<
        '\t'<<"to_netbo: "<<hexprint(&sshort_to_netbo,&sshort_to_netbo+1)<<'\n';

    return 0;
}
