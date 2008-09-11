#ifndef MSGLAYERS_HPP_INCLUDED
#define MSGLAYERS_HPP_INCLUDED



#include <stdexcept>
#include <algorithm> // for min

#include "bytes.hpp"

namespace nms
{
namespace protocol
{


class BasicMessageLayer
{
public:

    virtual ~BasicMessageLayer()
    {}

    virtual byte_traits::byte_sequence& getPayload() throw() = 0;
    virtual const byte_traits::byte_sequence& getPayload() const  throw() = 0;

    virtual std::size_t getSerializedSize() const throw() = 0;
	virtual byte_traits::byte_sequence serialize() const throw () = 0;
};



} // namespace protocol
} // namespace nms

#endif // ifndef MSGLAYERS_HPP_INCLUDED
