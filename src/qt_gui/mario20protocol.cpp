#include "mario20protocol.hpp"
#define ZLIB_CONST
#include <zlib/zlib.h>

namespace Mario20
{

// https://github.com/mapbox/gzip-hpp/blob/832d6262cecaa3b85c3c242e3617b4cfdbf3de23/include/gzip/compress.hpp#L26
template <typename InputType>
void compress(InputType& output,
    const void* data,
    std::size_t size)
{
    const std::size_t max_bytes = 2000000000;
#ifdef DEBUG
    // Verify if size input will fit into unsigned int, type used for zlib's avail_in
    if (size > std::numeric_limits<unsigned int>::max())
    {
        throw std::runtime_error("size arg is too large to fit into unsigned int type");
    }
#endif
    if (size > max_bytes)
    {
        throw std::runtime_error("size may use more memory than intended when decompressing");
    }

    z_stream deflate_s;
    deflate_s.zalloc = Z_NULL;
    deflate_s.zfree = Z_NULL;
    deflate_s.opaque = Z_NULL;
    deflate_s.avail_in = 0;
    deflate_s.next_in = Z_NULL;

    // The windowBits parameter is the base two logarithm of the window size (the size of the history buffer).
    // It should be in the range 8..15 for this version of the library.
    // Larger values of this parameter result in better compression at the expense of memory usage.
    // This range of values also changes the decoding type:
    //  -8 to -15 for raw deflate
    //  8 to 15 for zlib
    // (8 to 15) + 16 for gzip
    // (8 to 15) + 32 to automatically detect gzip/zlib header (decompression/inflate only)
    constexpr int window_bits = 15 + 16; // gzip with windowbits of 15

    constexpr int mem_level = 8;
    // The memory requirements for deflate are (in bytes):
    // (1 << (window_bits+2)) +  (1 << (mem_level+9))
    // with a default value of 8 for mem_level and our window_bits of 15
    // this is 128Kb

    if (deflateInit2(&deflate_s, Z_DEFAULT_COMPRESSION, Z_DEFLATED, window_bits, mem_level, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        throw std::runtime_error("deflate init failed");
    }

    deflate_s.next_in = reinterpret_cast<z_const Bytef*>(data);
    deflate_s.avail_in = static_cast<unsigned int>(size);

    std::size_t size_compressed = 0;
    do
    {
        size_t increase = size / 2 + 1024;
        if ((std::size_t)output.size() < (size_compressed + increase))
        {
            output.resize(size_compressed + increase);
        }
        // There is no way we see that "increase" would not fit in an unsigned int,
        // hence we use static cast here to avoid -Wshorten-64-to-32 error
        deflate_s.avail_out = static_cast<unsigned int>(increase);
        deflate_s.next_out = reinterpret_cast<Bytef*>((output.data() + size_compressed));
        // From http://www.zlib.net/zlib_how.html
        // "deflate() has a return value that can indicate errors, yet we do not check it here.
        // Why not? Well, it turns out that deflate() can do no wrong here."
        // Basically only possible error is from deflateInit not working properly
        deflate(&deflate_s, Z_FINISH);
        size_compressed += (increase - deflate_s.avail_out);
    } while (deflate_s.avail_out == 0);

    deflateEnd(&deflate_s);
    output.resize(size_compressed);
}

QByteArray compress(const void* data, size_t size)
{
    QByteArray result;
    compress(result, data, size);
    return result;
}

// https://github.com/mapbox/gzip-hpp/blob/832d6262cecaa3b85c3c242e3617b4cfdbf3de23/include/gzip/decompress.hpp#L24
template <typename OutputType>
void decompress(OutputType& output,
    const void* data,
    std::size_t size)
{
    z_stream inflate_s;

    inflate_s.zalloc = Z_NULL;
    inflate_s.zfree = Z_NULL;
    inflate_s.opaque = Z_NULL;
    inflate_s.avail_in = 0;
    inflate_s.next_in = Z_NULL;

    // The windowBits parameter is the base two logarithm of the window size (the size of the history buffer).
    // It should be in the range 8..15 for this version of the library.
    // Larger values of this parameter result in better compression at the expense of memory usage.
    // This range of values also changes the decoding type:
    //  -8 to -15 for raw deflate
    //  8 to 15 for zlib
    // (8 to 15) + 16 for gzip
    // (8 to 15) + 32 to automatically detect gzip/zlib header
    constexpr int window_bits = 15 + 32; // auto with windowbits of 15

    if (inflateInit2(&inflate_s, window_bits) != Z_OK)
    {
        throw std::runtime_error("inflate init failed");
    }
    inflate_s.next_in = reinterpret_cast<z_const Bytef*>(data);

#ifdef DEBUG
    // Verify if size (long type) input will fit into unsigned int, type used for zlib's avail_in
    std::uint64_t size_64 = size * 2;
    if (size_64 > std::numeric_limits<unsigned int>::max())
    {
        inflateEnd(&inflate_s);
        throw std::runtime_error("size arg is too large to fit into unsigned int type x2");
    }
#endif
    const std::size_t max_bytes = 1000000000;
    if (size > max_bytes || (size * 2) > max_bytes)
    {
        inflateEnd(&inflate_s);
        throw std::runtime_error("size may use more memory than intended when decompressing");
    }
    inflate_s.avail_in = static_cast<unsigned int>(size);
    std::size_t size_uncompressed = 0;
    do
    {
        std::size_t resize_to = size_uncompressed + 2 * size;
        if (resize_to > max_bytes)
        {
            inflateEnd(&inflate_s);
            throw std::runtime_error("size of output string will use more memory then intended when decompressing");
        }
        output.resize(resize_to);
        inflate_s.avail_out = static_cast<unsigned int>(2 * size);
        inflate_s.next_out = reinterpret_cast<Bytef*>(output.data() + size_uncompressed);
        int ret = inflate(&inflate_s, Z_FINISH);
        if (ret != Z_STREAM_END && ret != Z_OK && ret != Z_BUF_ERROR)
        {
            std::string error_msg = inflate_s.msg;
            inflateEnd(&inflate_s);
            throw std::runtime_error(error_msg);
        }

        size_uncompressed += (2 * size - inflate_s.avail_out);
    } while (inflate_s.avail_out == 0);
    inflateEnd(&inflate_s);
    output.resize(size_uncompressed);
}

QByteArray decompress(const void* data, size_t size)
{
    QByteArray result;
    decompress(result, data, size);
    return result;
}

std::string tohex(const QByteArray& b)
{
    std::string hexstr;
    for (int i = 0; i < b.length(); i++)
    {
        if (i)
            hexstr += ' ';
        char tmp[3] = "";
        sprintf(tmp, "%02X", (uint8_t)b[i]);
        hexstr += tmp;
    }
    return hexstr;
}

}