#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// minimal protobuf wire reader/writer for the ludus live protocol. only what
// the codecs in Generated/ need: varints, fixed32/64, length-delimited fields
// and unknown-field skipping. groups are rejected as malformed.

namespace SnoreSaber::Live::Wire
{
    class ProtoWriter
    {
      public:
        void WriteVarintField(uint32_t fieldNumber, uint64_t value);
        void WriteFixed32Field(uint32_t fieldNumber, uint32_t value);
        void WriteFixed64Field(uint32_t fieldNumber, uint64_t value);
        void WriteFloatField(uint32_t fieldNumber, float value);
        void WriteDoubleField(uint32_t fieldNumber, double value);
        void WriteStringField(uint32_t fieldNumber, std::string_view value);
        void WriteBytesField(uint32_t fieldNumber, const uint8_t* data, size_t size);

        // raw varint without a tag, for packed repeated payloads
        void WriteRawVarint(uint64_t value);

        const std::vector<uint8_t>& Buffer() const
        {
            return _buffer;
        }

        std::vector<uint8_t> TakeBuffer()
        {
            return std::move(_buffer);
        }

      private:
        void WriteTag(uint32_t fieldNumber, uint32_t wireType);

        std::vector<uint8_t> _buffer;
    };

    class ProtoReader
    {
      public:
        ProtoReader(const uint8_t* data, size_t size);

        // false at end of input or on error; check HasError to tell them apart
        bool ReadTag(uint32_t& fieldNumber, uint32_t& wireType);
        bool ReadVarint(uint64_t& value);
        bool ReadFixed32(uint32_t& value);
        bool ReadFixed64(uint64_t& value);
        bool ReadLengthDelimited(const uint8_t*& data, size_t& size);
        bool ReadString(std::string& value);
        bool ReadBytes(std::vector<uint8_t>& value);
        bool SkipField(uint32_t wireType);

        bool HasError() const
        {
            return _error;
        }

        bool AtEnd() const
        {
            return _position >= _size;
        }

      private:
        bool Fail();

        const uint8_t* _data;
        size_t _size;
        size_t _position = 0;
        bool _error = false;
    };
} // namespace SnoreSaber::Live::Wire
