#include "Features/Live/Protocol/ProtoWire.hpp"

#include <cstring>

namespace SnoreSaber::Live::Wire
{
    namespace
    {
        constexpr uint32_t WireTypeVarint = 0;
        constexpr uint32_t WireTypeFixed64 = 1;
        constexpr uint32_t WireTypeLengthDelimited = 2;
        constexpr uint32_t WireTypeFixed32 = 5;
    } // namespace

    void ProtoWriter::WriteTag(uint32_t fieldNumber, uint32_t wireType)
    {
        WriteRawVarint((uint64_t(fieldNumber) << 3) | wireType);
    }

    void ProtoWriter::WriteRawVarint(uint64_t value)
    {
        while (value >= 0x80)
        {
            _buffer.push_back(uint8_t(value) | 0x80);
            value >>= 7;
        }

        _buffer.push_back(uint8_t(value));
    }

    void ProtoWriter::WriteVarintField(uint32_t fieldNumber, uint64_t value)
    {
        WriteTag(fieldNumber, WireTypeVarint);
        WriteRawVarint(value);
    }

    void ProtoWriter::WriteFixed32Field(uint32_t fieldNumber, uint32_t value)
    {
        WriteTag(fieldNumber, WireTypeFixed32);
        for (int shift = 0; shift < 32; shift += 8)
            _buffer.push_back(uint8_t(value >> shift));
    }

    void ProtoWriter::WriteFixed64Field(uint32_t fieldNumber, uint64_t value)
    {
        WriteTag(fieldNumber, WireTypeFixed64);
        for (int shift = 0; shift < 64; shift += 8)
            _buffer.push_back(uint8_t(value >> shift));
    }

    void ProtoWriter::WriteFloatField(uint32_t fieldNumber, float value)
    {
        uint32_t bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        WriteFixed32Field(fieldNumber, bits);
    }

    void ProtoWriter::WriteDoubleField(uint32_t fieldNumber, double value)
    {
        uint64_t bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        WriteFixed64Field(fieldNumber, bits);
    }

    void ProtoWriter::WriteStringField(uint32_t fieldNumber, std::string_view value)
    {
        WriteBytesField(fieldNumber, reinterpret_cast<const uint8_t*>(value.data()), value.size());
    }

    void ProtoWriter::WriteBytesField(uint32_t fieldNumber, const uint8_t* data, size_t size)
    {
        WriteTag(fieldNumber, WireTypeLengthDelimited);
        WriteRawVarint(size);
        _buffer.insert(_buffer.end(), data, data + size);
    }

    ProtoReader::ProtoReader(const uint8_t* data, size_t size)
        : _data(data), _size(size)
    {
    }

    bool ProtoReader::Fail()
    {
        _error = true;
        return false;
    }

    bool ProtoReader::ReadTag(uint32_t& fieldNumber, uint32_t& wireType)
    {
        if (_error || AtEnd())
            return false;

        uint64_t tag = 0;
        if (!ReadVarint(tag))
            return false;

        fieldNumber = uint32_t(tag >> 3);
        wireType = uint32_t(tag & 0x7);
        if (fieldNumber == 0)
            return Fail();

        return true;
    }

    bool ProtoReader::ReadVarint(uint64_t& value)
    {
        value = 0;
        for (int shift = 0; shift < 64; shift += 7)
        {
            if (_position >= _size)
                return Fail();

            uint8_t byte = _data[_position++];
            value |= uint64_t(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0)
                return true;
        }

        return Fail();
    }

    bool ProtoReader::ReadFixed32(uint32_t& value)
    {
        if (_size - _position < 4)
            return Fail();

        value = 0;
        for (int shift = 0; shift < 32; shift += 8)
            value |= uint32_t(_data[_position++]) << shift;

        return true;
    }

    bool ProtoReader::ReadFixed64(uint64_t& value)
    {
        if (_size - _position < 8)
            return Fail();

        value = 0;
        for (int shift = 0; shift < 64; shift += 8)
            value |= uint64_t(_data[_position++]) << shift;

        return true;
    }

    bool ProtoReader::ReadLengthDelimited(const uint8_t*& data, size_t& size)
    {
        uint64_t length = 0;
        if (!ReadVarint(length))
            return false;

        if (length > _size - _position)
            return Fail();

        data = _data + _position;
        size = size_t(length);
        _position += size;
        return true;
    }

    bool ProtoReader::ReadString(std::string& value)
    {
        const uint8_t* data = nullptr;
        size_t size = 0;
        if (!ReadLengthDelimited(data, size))
            return false;

        value.assign(reinterpret_cast<const char*>(data), size);
        return true;
    }

    bool ProtoReader::ReadBytes(std::vector<uint8_t>& value)
    {
        const uint8_t* data = nullptr;
        size_t size = 0;
        if (!ReadLengthDelimited(data, size))
            return false;

        value.assign(data, data + size);
        return true;
    }

    bool ProtoReader::SkipField(uint32_t wireType)
    {
        switch (wireType)
        {
            case WireTypeVarint:
            {
                uint64_t skipped = 0;
                return ReadVarint(skipped);
            }
            case WireTypeFixed64:
            {
                uint64_t skipped = 0;
                return ReadFixed64(skipped);
            }
            case WireTypeLengthDelimited:
            {
                const uint8_t* data = nullptr;
                size_t size = 0;
                return ReadLengthDelimited(data, size);
            }
            case WireTypeFixed32:
            {
                uint32_t skipped = 0;
                return ReadFixed32(skipped);
            }
            default:
                // groups and reserved wire types are not part of this protocol
                return Fail();
        }
    }
} // namespace SnoreSaber::Live::Wire
