#include "Utils/lzma/lzma.hpp"
#include <mutex>

namespace LZMA
{
    const std::vector<char> *input_data;
    int input_index;
    
    std::vector<char> *output_data;
    size_t max_output_size = 0;

    std::mutex lock; // lazy threadsafety
    
    SRes Read(const ISeqInStream *pp, void *buf, size_t *size)
    {
        size_t orig_size = *size;
        for(*size = 0; *size < orig_size && input_index < input_data->size(); ++*size) {
            reinterpret_cast<char*>(buf)[*size] = (*input_data)[input_index++];
        }
        return SZ_OK;
    }

    size_t Write(const ISeqOutStream *pp, const void *data, size_t size)
    {
        if (max_output_size != 0 && (output_data->size() > max_output_size || size > max_output_size - output_data->size()))
        {
            return 0;
        }

        const char* bytes = reinterpret_cast<const char*>(data);
        output_data->insert(output_data->end(), bytes, bytes + size);
        return size;
    }

    void initialize_input(const std::vector<char> &in, ISeqInStream &stream) {
        input_data = &in;
        input_index = 0;
        stream.Read = Read;
    }

    void initialize_output(std::vector<char> &out, ISeqOutStream &stream, size_t maxOutputSize) {
        output_data = &out;
        max_output_size = maxOutputSize;
        stream.Write = Write;
    }

    bool lzmaDecompress(const std::vector<char> &in, std::vector<char> &out, size_t maxOutputSize) {
        std::lock_guard<std::mutex> guard(lock);

        ISeqInStream instream;
        ISeqOutStream outstream;
        initialize_input(in, instream);
        initialize_output(out, outstream, maxOutputSize);

        return Decode(&outstream, &instream) == SZ_OK;
    }

    bool lzmaDecompress(const std::vector<char> &in, std::vector<char> &out) {
        return lzmaDecompress(in, out, 0);
    }

    bool lzmaCompress(const std::vector<char> &in, std::vector<char> &out) {
        std::lock_guard<std::mutex> guard(lock);
        
        ISeqInStream instream;
        ISeqOutStream outstream;
        initialize_input(in, instream);
        initialize_output(out, outstream, 0);

        return Encode(&outstream, &instream, in.size()) == SZ_OK;
    }
}
