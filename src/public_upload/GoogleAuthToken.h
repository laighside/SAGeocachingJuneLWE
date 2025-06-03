/**
  @file    GoogleAuthToken.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Functions for getting an authentication token for Google Drive
  Requires linking to OpenSSL
  Most of this code comes from https://github.com/googleapis/google-cloud-cpp
  All functions are static so there is no need to create instances of the GoogleAuthToken object

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef GOOGLEAUTHTOKEN_H
#define GOOGLEAUTHTOKEN_H

#include <array>
#include <string>
#include <set>
#include <optional>
#include <vector>

// OpenSSL
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>

#include "../core/JlweCore.h"

#include "../ext/nlohmann/json.hpp"

class GoogleAuthToken
{
public:

    /*!
     * \brief Gets the token from the MySQL database
     *
     * Will return null if there is no valid token in the database
     *
     * \param jlwe JlweCore object
     * \return The token as a JSON object
     */
    static nlohmann::json getToken(JlweCore *jlwe);

    /*!
     * \brief Gets a new token from Google
     *
     * Will also save the new token to the database
     *
     * \param json_auth_file_contents The contains of the JSON authentication file, this file comes from the Google Service Account
     * \param jlwe JlweCore object
     * \return The token as a JSON object
     */
    static nlohmann::json getNewToken(const std::string &json_auth_file_contents, JlweCore *jlwe);

    /*!
     * \brief Gets the value for the HTTP Authorization Header
     *
     * Trys to fetch it from the database, and if that fails, gets a new one from Google
     *
     * \param jlwe JlweCore object
     * \return The value to put in the HTTP Authorization Header
     */
    static std::string getAuthorizationHeader(JlweCore *jlwe);

private:

    struct OpenSslDeleter {
        void operator()(EVP_MD_CTX* ptr) {
            // The name of the function to free an EVP_MD_CTX changed in OpenSSL 1.1.0.
        #if (OPENSSL_VERSION_NUMBER < 0x10100000L)  // Older than version 1.1.0.
            EVP_MD_CTX_destroy(ptr);
        #else
            EVP_MD_CTX_free(ptr);
        #endif
        }

        void operator()(EVP_PKEY* ptr) { EVP_PKEY_free(ptr); }
        void operator()(BIO* ptr) { BIO_free(ptr); }
    };

    static std::vector<std::uint8_t> SignUsingSha256(std::string const& str, std::string const& pem_contents);
    static std::string CaptureSslErrors();
    static std::unique_ptr<EVP_MD_CTX, OpenSslDeleter> GetDigestCtx();


    class Base64Encoder {
    public:
        explicit Base64Encoder() = default;
        void PushBack(unsigned char c) {
            buf_[len_++] = c;
            if (len_ == buf_.size()) Flush();
        }
        std::string FlushAndPad() &&;

    private:
        void Flush();

        std::string rep_;      // encoded
        std::size_t len_ = 0;  // buf_[0 .. len_-1] pending encode
        std::array<unsigned char, 3> buf_;

        // The extra braces are working around an old CLang bug that was fixed in 6.0
        // https://bugs.llvm.org/show_bug.cgi?id=21629
        static constexpr std::array<char, 64> kIndexToChar = {{
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
        }};
        static constexpr char kPadding = '=';
    };
    static std::string UrlsafeBase64Encode(std::string const& bytes);

    static std::string MakeJWTAssertion(std::string const& header, std::string const& payload, std::string const& pem_contents);

    /**
     * Object to hold information used to instantiate an ServiceAccountCredentials.
     *
     * @deprecated Prefer using the unified credentials documented in @ref guac
     */
    struct ServiceAccountCredentialsInfo {
      std::string client_email;
      std::string private_key_id;
      std::string private_key;
      std::string token_uri;
      // If no set is supplied, a default set of scopes will be used.
      std::optional<std::set<std::string>> scopes;
      // See https://developers.google.com/identity/protocols/OAuth2ServiceAccount.
      std::optional<std::string> subject;
      std::optional<std::string> universe_domain;
      std::optional<std::string> project_id;
    };

    static std::pair<std::string, std::string> AssertionComponentsFromInfo(ServiceAccountCredentialsInfo const& info);
    static ServiceAccountCredentialsInfo ParseServiceAccountCredentials(std::string const& content, std::string const& default_token_uri = "https://oauth2.googleapis.com/token");
};

#endif // GOOGLEAUTHTOKEN_H
