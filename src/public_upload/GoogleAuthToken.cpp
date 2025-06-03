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
#include "GoogleAuthToken.h"

#include <ctime>
#include <exception>
#include <memory>
#include <type_traits>

#include "../core/JlweUtils.h"
#include "../core/HttpRequest.h"

// what the token is called in the database
#define TOKEN_ID_IN_MYSQL "google_drive"

std::unique_ptr<EVP_MD_CTX, GoogleAuthToken::OpenSslDeleter> GoogleAuthToken::GetDigestCtx() {
// The name of the function to create an EVP_MD_CTX changed in OpenSSL 1.1.0.
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)  // Older than version 1.1.0.
  return std::unique_ptr<EVP_MD_CTX, OpenSslDeleter>(EVP_MD_CTX_create());
#else
  return std::unique_ptr<EVP_MD_CTX, OpenSslDeleter>(EVP_MD_CTX_new());
#endif
}

std::string GoogleAuthToken::CaptureSslErrors() {
  std::string msg;
  char const* sep = "";
  while (auto code = ERR_get_error()) {
    // OpenSSL guarantees that 256 bytes is enough:
    //   https://www.openssl.org/docs/man1.1.1/man3/ERR_error_string_n.html
    //   https://www.openssl.org/docs/man1.0.2/man3/ERR_error_string_n.html
    // we could not find a macro or constant to replace the 256 literal.
    auto constexpr kMaxOpenSslErrorLength = 256;
    std::array<char, kMaxOpenSslErrorLength> buf{};
    ERR_error_string_n(code, buf.data(), buf.size());
    msg += sep;
    msg += buf.data();
    sep = ", ";
  }
  return msg;
}

std::vector<std::uint8_t> GoogleAuthToken::SignUsingSha256(
    std::string const& str, std::string const& pem_contents) {
  ERR_clear_error();
  auto pem_buffer = std::unique_ptr<BIO, OpenSslDeleter>(BIO_new_mem_buf(
      pem_contents.data(), static_cast<int>(pem_contents.length())));
  if (!pem_buffer) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials - "
        "could not create PEM buffer: " +
            CaptureSslErrors());
  }

  auto private_key =
      std::unique_ptr<EVP_PKEY, OpenSslDeleter>(PEM_read_bio_PrivateKey(
          pem_buffer.get(),
          nullptr,  // EVP_PKEY **x
          nullptr,  // pem_password_cb *cb -- a custom callback.
          // void *u -- this represents the password for the PEM (only
          // applicable for formats such as PKCS12 (.p12 files) that use
          // a password, which we don't currently support.
          nullptr));
  if (!private_key) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials - "
        "could not parse PEM to get private key: " +
            CaptureSslErrors());
  }

  auto digest_ctx = GetDigestCtx();
  if (!digest_ctx) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials - "
        "could not create context for OpenSSL digest: " +
            CaptureSslErrors());
  }

  auto constexpr kOpenSslSuccess = 1;
  if (EVP_DigestSignInit(digest_ctx.get(), nullptr, EVP_sha256(), nullptr,
                         private_key.get()) != kOpenSslSuccess) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials - "
        "could not initialize signing digest: " +
            CaptureSslErrors());
  }

  if (EVP_DigestSignUpdate(digest_ctx.get(), str.data(), str.size()) !=
      kOpenSslSuccess) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials - could not sign blob: " +
            CaptureSslErrors());
  }

  // The signed SHA256 size depends on the size (the experts say "modulus") of
  // they key.  First query the size:
  std::size_t actual_len = 0;
  if (EVP_DigestSignFinal(digest_ctx.get(), nullptr, &actual_len) !=
      kOpenSslSuccess) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials - could not sign blob: " +
            CaptureSslErrors());
  }

  // Then compute the actual signed digest. Note that OpenSSL requires a
  // `unsigned char*` buffer, so we feed it that.
  std::vector<unsigned char> buffer(actual_len);
  if (EVP_DigestSignFinal(digest_ctx.get(), buffer.data(), &actual_len) !=
      kOpenSslSuccess) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials - could not sign blob: " +
            CaptureSslErrors());
  }

  return std::vector<std::uint8_t>(
      {buffer.begin(), std::next(buffer.begin(), actual_len)});
}


void GoogleAuthToken::Base64Encoder::Flush() {
  unsigned int const v = buf_[0] << 16 | buf_[1] << 8 | buf_[2];
  rep_.push_back(kIndexToChar[v >> 18]);
  rep_.push_back(kIndexToChar[v >> 12 & 0x3f]);
  rep_.push_back(kIndexToChar[v >> 6 & 0x3f]);
  rep_.push_back(kIndexToChar[v & 0x3f]);
  len_ = 0;
}

std::string GoogleAuthToken::Base64Encoder::FlushAndPad() && {
  switch (len_) {  // NOLINT(bugprone-switch-missing-default-case)
    case 2: {
      unsigned int const v = buf_[0] << 16 | buf_[1] << 8;
      rep_.push_back(kIndexToChar[v >> 18]);
      rep_.push_back(kIndexToChar[v >> 12 & 0x3f]);
      rep_.push_back(kIndexToChar[v >> 6 & 0x3f]);
      rep_.push_back(kPadding);
      break;
    }
    case 1: {
      unsigned int const v = buf_[0] << 16;
      rep_.push_back(kIndexToChar[v >> 18]);
      rep_.push_back(kIndexToChar[v >> 12 & 0x3f]);
      rep_.append(2, kPadding);
      break;
    }
    case 0:
      break;
  }
  return std::move(rep_);
}

std::string GoogleAuthToken::UrlsafeBase64Encode(std::string const& bytes) {
  Base64Encoder encoder;
  for (auto c : bytes) encoder.PushBack(c);
  std::string b64str = std::move(encoder).FlushAndPad();
  std::replace(b64str.begin(), b64str.end(), '+', '-');
  std::replace(b64str.begin(), b64str.end(), '/', '_');
  auto end_pos = b64str.find_last_not_of('=');
  if (end_pos != std::string::npos) {
    b64str.resize(end_pos + 1);
  }
  return b64str;
}

std::string GoogleAuthToken::MakeJWTAssertion(std::string const& header, std::string const& payload, std::string const& pem_contents) {
  std::string const body = UrlsafeBase64Encode(header) + '.' + UrlsafeBase64Encode(payload);
  std::vector<std::uint8_t> pem_signature = SignUsingSha256(body, pem_contents);
  return body + "." + UrlsafeBase64Encode(std::string(pem_signature.begin(), pem_signature.end()));
}

std::pair<std::string, std::string> GoogleAuthToken::AssertionComponentsFromInfo(
    GoogleAuthToken::ServiceAccountCredentialsInfo const& info) {
  nlohmann::json assertion_header = {{"alg", "RS256"}, {"typ", "JWT"}};
  if (!info.private_key_id.empty()) {
    assertion_header["kid"] = info.private_key_id;
  }

  // Scopes must be specified in a space separated string:
  //    https://google.aip.dev/auth/4112
  auto scopes = [&info]() -> std::string {
    if (!info.scopes) return "https://www.googleapis.com/auth/cloud-platform"; //GoogleOAuthScopeCloudPlatform();
    std::string result;
    for (const std::string& element : *(info.scopes)) {
      if (result.size())
        result += " ";
      result += element;
    }
    return result;
  }();

  time_t time_now = time(nullptr);
  time_t expiration = time_now + 3600; // GoogleOAuthAccessTokenLifetime();
  nlohmann::json assertion_payload = {
      {"iss", info.client_email},
      {"scope", scopes},
      {"aud", info.token_uri},
      {"iat", time_now},
      // Resulting access token should expire after one hour.
      {"exp", expiration}};
  if (info.subject) {
    assertion_payload["sub"] = *(info.subject);
  }

  // Note: we don't move here as it would prevent copy elision.
  return std::make_pair(assertion_header.dump(), assertion_payload.dump());
}


GoogleAuthToken::ServiceAccountCredentialsInfo GoogleAuthToken::ParseServiceAccountCredentials(
    std::string const& content, std::string const& default_token_uri) {
  auto credentials = nlohmann::json::parse(content, nullptr, false);
  if (credentials.is_discarded()) {
    throw std::invalid_argument(
        "Invalid ServiceAccountCredentials, parsing failed");
  }

  ServiceAccountCredentialsInfo info;
  info.client_email = credentials.at("client_email");
  info.private_key = credentials.at("private_key");
  info.private_key_id = credentials.at("private_key_id");
  info.token_uri = credentials.value("token_uri", default_token_uri);
  info.universe_domain = credentials.value("universe_domain", "googleapis.com");
  info.project_id = credentials.at("project_id");
  return info;
}

nlohmann::json GoogleAuthToken::getToken(JlweCore *jlwe) {
    sql::PreparedStatement *prep_stmt;
    sql::ResultSet *res;

    nlohmann::json token = nullptr;
    prep_stmt = jlwe->getMysqlCon()->prepareStatement("SELECT token FROM auth_tokens WHERE id = ? AND expires > DATE_ADD(NOW(), INTERVAL 1 MINUTE);");
    prep_stmt->setString(1, TOKEN_ID_IN_MYSQL);
    res = prep_stmt->executeQuery();
    if (res->next())
        token = nlohmann::json::parse(std::string(res->getString(1)));

    delete res;
    delete prep_stmt;

    return token;
}

nlohmann::json GoogleAuthToken::getNewToken(const std::string &json_auth_file_contents, JlweCore *jlwe) {
    GoogleAuthToken::ServiceAccountCredentialsInfo info = ParseServiceAccountCredentials(json_auth_file_contents);
    info.scopes = {"https://www.googleapis.com/auth/drive"};

    std::pair<std::string, std::string> components = AssertionComponentsFromInfo(info);
    std::string jwt_assertion = MakeJWTAssertion(components.first, components.second, info.private_key);

    nlohmann::json token = nullptr;
    HttpRequest request(info.token_uri);
    std::string post_data = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + jwt_assertion;
    if (request.post(post_data, "application/x-www-form-urlencoded")) {
        token = nlohmann::json::parse(request.responseAsString());
    } else {
        throw std::runtime_error("HTTP request to " + info.token_uri + " failed");
    }

    if (!token.contains("access_token") || !token.contains("expires_in") || !token.contains("token_type"))
        throw std::runtime_error("Invalid JSON response: " + token.dump());

    time_t expires_in = token.at("expires_in");
    time_t expire_time = time(nullptr) + expires_in;

    sql::PreparedStatement *prep_stmt = jlwe->getMysqlCon()->prepareStatement("SELECT setAuthToken(?,?,?);");
    prep_stmt->setString(1, TOKEN_ID_IN_MYSQL);
    prep_stmt->setString(2, token.dump());
    prep_stmt->setInt64(3, expire_time);
    sql::ResultSet *res = prep_stmt->executeQuery();
    res->next();
    delete res;
    delete prep_stmt;

    return token;
}

std::string GoogleAuthToken::getAuthorizationHeader(JlweCore *jlwe) {
    nlohmann::json token = getToken(jlwe);
    if (!token.contains("access_token") || !token.contains("token_type")) {
        token = getNewToken(JlweUtils::readFileToString(std::string(jlwe->config.at("google_drive_json_auth_file")).c_str()), jlwe);
    }
    std::string auth_header = std::string(token.at("token_type")) + " " + std::string(token.at("access_token"));
    return auth_header;
}
