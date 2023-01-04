#pragma once

#include "TCPCommon/Common.h"
#include <vector>
#include <string>

BEGIN_NAMESPACE_NET

class IOBuffer
{
public:

    /**
    * Returns the Buffer stream.
    */
    const std::vector<uint8_t>& GetData() const { return m_Buffer; }

    /**
    * Returns if the buffer stream contains any more data.
    */
    bool HasData() const { return Size() > 0; }

    /**
    * Returns the current size of the buffer.
    */
    std::size_t Size() const { return m_Buffer.size(); }

    /**
    * Clears all the data from the buffer. The IOBuffer can then be used to store new data.
    */
    void Clear() { m_Buffer.clear(); }

    /**
    * Writes data to the Stream.
    * 
    * @template DataType
    *       Data type of the object that will be written to the stream.
    * 
    * @param [in] data
    *       The data that will be written to the stream.
    * 
    * @return
    *       Reference to the current stream object to allow chaining.
    */
    template<typename DataType>
    IOBuffer& operator << (const DataType& data)
    {
        // Check that the type of the data being pushed is trivially copyable
        //static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

        // Cache current size of vector, as this will be the point we insert the data
        size_t i = m_Buffer.size();

        // Resize the vector by the size of the data being pushed
        m_Buffer.resize(m_Buffer.size() + sizeof(DataType));

        // Physically copy the data into the newly allocated vector space
        std::memcpy(m_Buffer.data() + i, &data, sizeof(DataType));

        return *this;
    }

    /**
    * Reads data from the Stream.
    * Order of the retrieving data from stream is the same as the order of inserting data into the stream.
    * 
    * @template DataType
    *       Data type of the object that will be read from the stream.
    * 
    * @param [out] data
    *       The data that will be Read from the stream.
    *
    * @return
    *       Reference to the current stream object to allow chaining.
    */
    template<typename DataType>
    IOBuffer& operator >> (DataType& data)
    {
        // Check that the type of the data being pushed is trivially copyable
        //static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

        std::size_t numBytesToRead = sizeof(DataType);

        // read the actual data
        std::memcpy(&data, m_Buffer.data(), numBytesToRead);

        // remove the read data from m_Buffer.
        std::size_t newSize = m_Buffer.size() - numBytesToRead;
        std::memcpy(m_Buffer.data(), m_Buffer.data() + numBytesToRead, newSize);

        m_Buffer.resize(newSize);

        return *this;
    }

    /**
    * Spelialized template for writing Strings to the stream.
    * First the size of the string is written, then the content of the string is written to the stream.
    * 
    * @param [in] str
    *       The string that needs to be written to the stream.
    * 
    * @return
    *       Reference to the current stream object to allow chaining.
    */
    template<>
    IOBuffer& operator << (const std::string& str)
    {
        std::size_t cs = m_Buffer.size();

        std::size_t strSize = str.size();
        m_Buffer.resize(cs + sizeof(str.size()));

        // write size
        std::memcpy(m_Buffer.data() + cs, reinterpret_cast<void*>(&strSize), sizeof(std::size_t));

        cs = m_Buffer.size();
        m_Buffer.resize(cs + strSize);

        // write the actual data
        std::memcpy(m_Buffer.data() + cs, str.data(), str.size());

        return *this;
    }

    /**
    * Spelialized template for reading Strings from the stream.
    * First the size of the string is read, then the content of the string is read from the stream.
    *
    * @param [in] data
    *       The string that will be read from the stream.
    *
    * @return
    *       Reference to the current stream object to allow chaining.
    */
    template<>
    IOBuffer& operator >> (std::string& data)
    {
        // read the size of the string.
        std::size_t sizeOfString = 0;
        std::memcpy(&sizeOfString, m_Buffer.data(), sizeof(std::size_t));

        // remove the 'size' part from the buffer.
        std::size_t newSize = m_Buffer.size() - sizeof(std::size_t);
        std::memcpy(m_Buffer.data(), m_Buffer.data() + sizeof(std::size_t), newSize);

        data.resize(sizeOfString);
        // read the actual string into 'data'
        std::memcpy(data.data(), m_Buffer.data(), sizeOfString);

        // remove the 'data' part of the string read from the buffer.
        std::memcpy(m_Buffer.data(), m_Buffer.data() + sizeOfString, newSize);

        newSize -= sizeOfString;
        m_Buffer.resize(newSize);

        return *this;
    }

private:

    /* This vector contains the actual byte data of the stream. */
    std::vector<uint8_t> m_Buffer;
};

END_NAMESPACE_NET

//#include "IOBuffer.hpp"

