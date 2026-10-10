#include "Utils/WebUtils.hpp"

#include "assets.hpp"
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string_view>
#include <tuple>

#include <System/Action.hpp>
#include <UnityEngine/Networking/DownloadHandler.hpp>
#include <UnityEngine/Networking/DownloadHandlerTexture.hpp>
#include <UnityEngine/Networking/UnityWebRequest.hpp>
#include <UnityEngine/Networking/UnityWebRequestTexture.hpp>
#include <UnityEngine/Sprite.hpp>
#include <UnityEngine/Color32.hpp>
#include <UnityEngine/SpriteMeshType.hpp>
#include <UnityEngine/Texture2D.hpp>
#include "Utils/AsyncUtils.hpp"
#include "Utils/StringUtils.hpp"
#include <custom-types/shared/delegate.hpp>
#include <gif-lib/shared/gif_lib.h>
#include <libcurl/shared/curl.h>
#include <libcurl/shared/easy.h>
#include "logging.hpp"

// #include "gif_read.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace StringUtils;

// from
// https://github.com/darknight1050/SongDownloader/blob/master/src/Utils/WebUtils.cpp

#define TIMEOUT 10
#define USER_AGENT                               \
    (std::string("SnoreSaber-Quest/") + VERSION) \
        .c_str()

#include "gif-lib/shared/gif_lib.h"
struct Gif
{
    Gif(std::string& text)
        : data(text), datastream(&this->data){};
    Gif(std::vector<uint8_t>& vec)
        : data(reinterpret_cast<std::vector<char>&>(vec)), datastream(&this->data){};
    Gif(std::vector<char>& vec)
        : data(vec), datastream(&this->data){};
    Gif(Array<char>* array)
        : data(array), datastream(&this->data){};
    Gif(Array<uint8_t>* array)
        : Gif(reinterpret_cast<Array<char>*>(array)){};
    Gif(ArrayW<uint8_t> array)
        : Gif(static_cast<Array<uint8_t>*>(array)){};
    ~Gif()
    {
        int error = 0;
        DGifCloseFile(gif, &error);
    }
    int Parse()
    {
        int error = 0;
        gif = DGifOpen(this, &Gif::read, &error);
        return error;
    }

    int Slurp()
    {
        return DGifSlurp(gif);
    }

    static int read(GifFileType* pGifHandle, GifByteType* dest, int toRead)
    {
        Gif& dataWrapper = *(Gif*)pGifHandle->UserData;
        return dataWrapper.datastream.readsome(reinterpret_cast<char*>(dest), toRead);
    }

    Texture2D* get_frame(int idx)
    {
        if (!gif || idx < 0 || idx >= get_length())
            return nullptr;

        GifColorType* color;
        ExtensionBlock* ext = 0;
        int x, y, j, loc;

        SavedImage* frame = &(gif->SavedImages[idx]);
        GifImageDesc* frameInfo = &(frame->ImageDesc);
        ColorMapObject* colorMap = frameInfo->ColorMap ? frameInfo->ColorMap : gif->SColorMap;

        // hostile gifs can omit colormaps or raster data entirely
        if (!colorMap || !colorMap->Colors || colorMap->ColorCount <= 0 || !frame->RasterBits)
            return nullptr;

        // entire texture size;
        int width = get_width();
        int height = get_height();

        // avatar-sized sanity cap; also keeps all offset math far from int overflow
        constexpr long long MaxGifPixels = 4096LL * 4096LL;
        if (width <= 0 || height <= 0 || (long long)width * (long long)height > MaxGifPixels)
            return nullptr;

        // a frame must fit entirely inside the canvas or the pixel writes go oob
        if (frameInfo->Left < 0 || frameInfo->Top < 0 || frameInfo->Width <= 0 || frameInfo->Height <= 0 ||
            frameInfo->Left + frameInfo->Width > width || frameInfo->Top + frameInfo->Height > height)
            return nullptr;

        for (j = 0; j < frame->ExtensionBlockCount; ++j)
        {
            if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE && frame->ExtensionBlocks[j].ByteCount >= 4)
            {
                ext = &(frame->ExtensionBlocks[j]);
                break;
            }
        }

        auto texture = Texture2D::New_ctor(width, height);
        // entire texture
        auto pixelData = texture->GetPixels32();
        if (!pixelData)
            return nullptr;
        // top -> top + height

        // left -> left + width

        // if directly setting in the color array, the loc is y inverted
        // A frame only describes part of a picture
        long pixelDataOffset = frameInfo->Top * width + frameInfo->Left;
        for (y = 0; y < frameInfo->Height; ++y)
        {
            for (x = 0; x < frameInfo->Width; ++x)
            {
                loc = y * frameInfo->Width + x;
                int colorIdx = frame->RasterBits[loc];
                if (ext && colorIdx == ext->Bytes[3] && ext->Bytes[0])
                {
                    continue;
                }
                if (colorIdx >= colorMap->ColorCount)
                {
                    continue; // hostile palette index: skip pixel instead of oob read
                }

                color = &colorMap->Colors[colorIdx];
                long locWithinFrame = (frameInfo->Height - y - 1) * frameInfo->Width + x + pixelDataOffset;
                pixelData->_values[locWithinFrame]._ctor(color->Red, color->Green, color->Blue, 0xff);
            }
        }
        texture->SetAllPixels32(pixelData, 0);
        texture->Apply();
        return texture;
    }
    int get_width()
    {
        return gif ? gif->SWidth : 0;
    };
    int get_height()
    {
        return gif ? gif->SHeight : 0;
    };
    int get_length()
    {
        return gif ? gif->ImageCount : 0;
    };

  public:
    GifFileType* gif = nullptr;

  private:
    template <typename CharT, typename TraitsT = std::char_traits<CharT>>
    class vectorwrapbuf : public std::basic_streambuf<CharT, TraitsT>
    {
      public:
        vectorwrapbuf(std::string& text)
        {
            this->std::basic_streambuf<CharT, TraitsT>::setg(text.data(), text.data(), text.data() + text.size());
        }

        vectorwrapbuf(std::vector<CharT>& vec)
        {
            this->std::basic_streambuf<CharT, TraitsT>::setg(vec.data(), vec.data(), vec.data() + vec.size());
        }

        vectorwrapbuf(Array<CharT>*& arr)
        {
            this->std::basic_streambuf<CharT, TraitsT>::setg(arr->_values, arr->_values, arr->_values + arr->get_Length());
        }
    };

    std::istream datastream;
    vectorwrapbuf<char> data;
};

namespace WebUtils
{
    // https://stackoverflow.com/a/55660581

    std::mutex cookieMutex;
    std::string cookie; // guarded by cookieMutex

    static std::string GetCookie()
    {
        std::lock_guard<std::mutex> lock(cookieMutex);
        return cookie;
    }

    void FinishAsyncRequest(std::function<void(long, std::string)> finished, long responseCode, std::string response, bool pureCppCallback)
    {
        if (!finished)
            return;

        if (pureCppCallback)
        {
            finished(responseCode, std::move(response));
            return;
        }

        SnoreSaber::Utils::Async::Main([finished = std::move(finished), responseCode, response = std::move(response)] {
            finished(responseCode, response);
        });
    }

    std::string query_encode(const std::string& s)
    {
        std::string ret;
#define IS_BETWEEN(ch, low, high) (ch >= low && ch <= high)
#define IS_ALPHA(ch) (IS_BETWEEN(ch, 'A', 'Z') || IS_BETWEEN(ch, 'a', 'z'))
#define IS_DIGIT(ch) IS_BETWEEN(ch, '0', '9')
#define IS_HEXDIG(ch) \
    (IS_DIGIT(ch) || IS_BETWEEN(ch, 'A', 'F') || IS_BETWEEN(ch, 'a', 'f'))

        for (size_t i = 0; i < s.size();)
        {
            char ch = s[i++];

            if (IS_ALPHA(ch) || IS_DIGIT(ch))
            {
                ret += ch;
            }
            else if ((ch == '%') && i + 1 < s.size() && IS_HEXDIG(s[i + 0]) && IS_HEXDIG(s[i + 1]))
            {
                ret += s.substr(i - 1, 3);
                i += 2;
            }
            else
            {
                switch (ch)
                {
                    case '-':
                    case '.':
                    case '_':
                    case '~':
                    case '!':
                    case '$':
                    case '&':
                    case '\'':
                    case '(':
                    case ')':
                    case '*':
                    case '+':
                    case ',':
                    case ';':
                    case '=':
                    case ':':
                    case '@':
                    case '/':
                    case '?':
                    case '[':
                    case ']':
                        ret += ch;
                        break;

                    default: {
                        static const char hex[] = "0123456789ABCDEF";
                        char pct[] = "%  ";
                        pct[1] = hex[(ch >> 4) & 0xF];
                        pct[2] = hex[ch & 0xF];
                        ret.append(pct, 3);
                        break;
                    }
                }
            }
        }

        return ret;
    }

    std::size_t CurlWrite_CallbackFunc_StdString(void* contents, std::size_t size,
                                                 std::size_t nmemb,
                                                 std::string* s)
    {
        std::size_t newLength = size * nmemb;
        try
        {
            s->append((char*)contents, newLength);
        }
        catch (std::bad_alloc& e)
        {
            // handle memory problem
            CRITICAL("Failed to allocate string of size: {:d}", newLength);
            return 0;
        }
        return newLength;
    }

    std::size_t CurlWrite_CallbackFunc_VectorChar(void* contents, std::size_t size,
                                                  std::size_t nmemb,
                                                  std::vector<char>* v)
    {
        std::size_t newLength = size * nmemb;
        try
        {
            v->insert(v->end(), (char*)contents, (char*)contents + newLength);
        }
        catch (std::bad_alloc& e)
        {
            // handle memory problem
            CRITICAL("Failed to allocate vector of size: {:d}", newLength);
            return 0;
        }
        return newLength;
    }

    size_t hdf(char* b, size_t size, size_t nitems, void* userdata)
    {
        size_t numbytes = size * nitems;
        std::string header(b, numbytes);

        if (header.starts_with("Set-Cookie"))
        {
            replace(header, "Set-Cookie: ", "");
            header = split(header, ';')[0];
            std::lock_guard<std::mutex> lock(cookieMutex);
            cookie = header;
        }
        return numbytes;
    }

    // shared hardening for every curl easy handle: verify tls against the
    // bundled mozilla ca store and keep redirects bounded + https-only
    void ApplyTransportOptions(CURL* curl)
    {
        static const std::string_view caBundle = IncludedAssets::cacert_pem;
        static const curl_blob caBlob = {
            const_cast<char*>(caBundle.data()),
            caBundle.size(),
            CURL_BLOB_NOCOPY,
        };
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, const_cast<curl_blob*>(&caBlob));
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "https");
    }

    std::tuple<long, std::string> GetSync(std::string url, long timeout)
    {
        return GetSync(url, timeout, {});
    }

    std::tuple<long, std::string> GetSync(std::string url, long timeout, const std::vector<std::string>& requestHeaders)
    {
        std::string val;
        // Init curl
        auto* curl = curl_easy_init();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: */*");
        for (auto& header : requestHeaders)
        {
            headers = curl_slist_append(headers, header.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

        std::string requestCookie = GetCookie();
        if (!requestCookie.empty())
        {
            curl_easy_setopt(curl, CURLOPT_COOKIE, requestCookie.c_str());
        }
        // Set headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());

        // Don't wait forever, time out after TIMEOUT seconds.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

        // Follow HTTP redirects if necessary.
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         CurlWrite_CallbackFunc_StdString);

        long httpCode(0);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        ApplyTransportOptions(curl);

        auto res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            CRITICAL("curl_easy_perform() failed: {:d}: {:s}", (int)res, curl_easy_strerror(res));
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return std::make_tuple(httpCode, val);
    }

    void GetAsync(std::string url, std::function<void(long, std::string)> finished, bool pureCppCallback)
    {
        GetAsync(url, TIMEOUT, finished, pureCppCallback);
    }

    void GetAsync(std::string url, long timeout, std::function<void(long, std::string)> finished, bool pureCppCallback)
    {
        if (pureCppCallback) {
            SnoreSaber::Utils::Async::RunCpp([url = std::move(url), timeout, finished = std::move(finished)]() mutable {
                auto [responseCode, response] = GetSync(url, timeout);
                FinishAsyncRequest(std::move(finished), responseCode, std::move(response), true);
            });
        } else {
            SnoreSaber::Utils::Async::Run([url = std::move(url), timeout, finished = std::move(finished)] {
                auto [responseCode, response] = GetSync(url, timeout);
                FinishAsyncRequest(finished, responseCode, std::move(response), false);
            });
        }
    }

    std::tuple<long, std::string> PostJsonSync(std::string url, std::string jsonData, long timeout)
    {
        std::string val;
        auto* curl = curl_easy_init();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

        std::string requestCookie = GetCookie();
        if (!requestCookie.empty())
        {
            curl_easy_setopt(curl, CURLOPT_COOKIE, requestCookie.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

        long httpCode(0);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        ApplyTransportOptions(curl);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, hdf);

        auto res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            CRITICAL("curl_easy_perform() failed: {:d}: {:s}", (int)res, curl_easy_strerror(res));
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return std::make_tuple(httpCode, val);
    }

    std::tuple<long, std::string> PostWithReplaySync(std::string url, const std::vector<char>& replayData, std::string postData, long timeout, const std::vector<std::string>& requestHeaders)
    {
        std::string val;
        // Init curl
        auto* curl = curl_easy_init();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: */*");
        for (const std::string& requestHeader : requestHeaders)
            headers = curl_slist_append(headers, requestHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

        std::string requestCookie = GetCookie();
        if (!requestCookie.empty())
        {
            curl_easy_setopt(curl, CURLOPT_COOKIE, requestCookie.c_str());
        }

        // Set headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());

        // Don't wait forever, time out after TIMEOUT seconds.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

        // Follow HTTP redirects if necessary.
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_POST, 1);

        curl_mime* mime;
        curl_mimepart* part;
        mime = curl_mime_init(curl);
        part = curl_mime_addpart(mime);

        curl_mime_name(part, "data");
        curl_mime_data(part, postData.c_str(), CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);

        curl_mime_name(part, "zr");
        curl_mime_data(part, replayData.data(), replayData.size());
        curl_mime_filename(part, "zr.dat"); // DO NOT DELETE, otherwise the server will reject the replay
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);

        long httpCode(0);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        ApplyTransportOptions(curl);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, hdf);

        auto res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
        {
            CRITICAL("curl_easy_perform() failed: {:d}: {:s}", (int)res, curl_easy_strerror(res));
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        curl_mime_free(mime);
        curl_slist_free_all(headers);

        return std::make_tuple(httpCode, val);
    }

    long DownloadReplaySync(std::string url, std::vector<char>& replayData, long timeout)
    {
        // Init curl
        auto* curl = curl_easy_init();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: */*");

        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
        std::string requestCookie = GetCookie();
        if (!requestCookie.empty())
        {
            curl_easy_setopt(curl, CURLOPT_COOKIE, requestCookie.c_str());
        }
        // Set headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());

        // Don't wait forever, time out after TIMEOUT seconds.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

        // Follow HTTP redirects if necessary.
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_VectorChar);

        long httpCode(0);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &replayData);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        ApplyTransportOptions(curl);

        auto res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            CRITICAL("curl_easy_perform() failed: {:d}: {:s}", (int)res, curl_easy_strerror(res));
        }
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return httpCode;
    }

    std::tuple<long, std::string> DownloadFileSync(std::string url, const std::string& filePath, long timeout, long stallSeconds)
    {
        FILE* file = fopen(filePath.c_str(), "wb");
        if (!file)
        {
            return std::make_tuple(0L, std::string("failed to open download target file"));
        }

        auto* curl = curl_easy_init();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: */*");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, query_encode(url).c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        // abort when fewer than 1 byte/s flows for stallSeconds
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, stallSeconds);
        ApplyTransportOptions(curl);

        auto res = curl_easy_perform(curl);
        long httpCode(0);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        fclose(file);

        std::string error;
        if (res != CURLE_OK)
        {
            error = curl_easy_strerror(res);
        }
        return std::make_tuple(httpCode, error);
    }

    std::vector<unsigned char> Swap(std::vector<unsigned char> panda1, std::vector<unsigned char> panda2)
    {
        int N1 = 11;
        int N2 = 13;
        int NS = 257;

        for (int i = 0; i <= panda2.size() - 1; i++)
        {
            NS += NS % (panda2[i] + 1);
        }

        std::vector<unsigned char> T(panda1.size());
        for (int i = 0; i <= panda1.size() - 1; i++)
        {
            NS = panda2[i % panda2.size()] + NS;
            N1 = (NS + 5) * (N1 & 255) + (N1 >> 8);
            N2 = (NS + 7) * (N2 & 255) + (N2 >> 8);
            NS = ((N1 << 8) + N2) & 255;

            T[i] = static_cast<unsigned char>(panda1[i] ^ static_cast<unsigned char>(NS));
        }

        return T;
    }

    std::string ConvertToHex(const std::vector<unsigned char>& v)
    {
        std::stringstream buffer;
        for (int i = 0; i < v.size(); i++)
        {
            buffer << std::hex << std::setfill('0');
            buffer << std::setw(2) << static_cast<unsigned>(v[i]);
        }
        return buffer.str();
    }

    custom_types::Helpers::Coroutine WaitForImageDownload(std::string url, HMUI::ImageView* out)
    {
        UnityEngine::Networking::UnityWebRequest* www = UnityEngine::Networking::UnityWebRequestTexture::GetTexture(url);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(www->SendWebRequest());
        while(!www->isDone)
            co_yield nullptr;
        auto downloadHandlerTexture = il2cpp_utils::cast<UnityEngine::Networking::DownloadHandlerTexture>(www->get_downloadHandler());
        if(www->result != UnityEngine::Networking::UnityWebRequest::Result::Success)
        {
            ERROR("Failed to download image from url {:s} with error messages {:s} and {:s}", url, www->error, downloadHandlerTexture->GetErrorMsg());
            www->Dispose();
            co_return;
        }
        auto texture = downloadHandlerTexture->get_texture();
        auto sprite = Sprite::Create(texture, Rect(0.0f, 0.0f, (float)texture->get_width(), (float)texture->get_height()), Vector2(0.5f, 0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
        out->set_sprite(sprite);
        co_return;
    }

    custom_types::Helpers::Coroutine WaitForGifDownload(std::string url, HMUI::ImageView* out)
    {
        UnityEngine::Networking::UnityWebRequest* www = UnityEngine::Networking::UnityWebRequest::Get(url);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(www->SendWebRequest());
        while(!www->isDone)
            co_yield nullptr;
        if (www->result == UnityEngine::Networking::UnityWebRequest::Result::ProtocolError ||
            www->result == UnityEngine::Networking::UnityWebRequest::Result::ConnectionError ||
            !System::String::IsNullOrEmpty(www->error) ||
            !www->downloadHandler)
        {
            INFO("Failed to download gif");
            www->Dispose();
            co_return;
        }

        auto gifDataArr = www->downloadHandler->GetData();
        Gif gif(gifDataArr);
        int error = gif.Parse();
        co_yield nullptr;
        if (!error && gif.Slurp())
        {
            co_yield nullptr;
            auto texture = gif.get_frame(0);
            if (!texture)
            {
                INFO("Failed to read gif frame");
                www->Dispose();
                co_return;
            }

            auto sprite = Sprite::Create(texture, Rect(0.0f, 0.0f, (float)gif.get_width(), (float)gif.get_height()), Vector2(0.5f, 0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
            out->set_sprite(sprite);
        }
        else
        {
            INFO("Failed to read gif with error code {}", error);
        }
        www->Dispose();
        co_return;
    }

} // namespace WebUtils
