/**
  @file    Password.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used for creating and checking password hashes
  All functions are static so there is no need to create instances of the Password object
 */
#include "Password.h"

#include <cstring>

extern "C" {
#include "../ext/crypt_blowfish/ow-crypt.h"
}
#include "../ext/csprng/csprng.hpp"

// length of bcrypt hash used on passwords
#define BCRYPT_HASHSIZE    (64)

// number of iterations used by bcrypt, only applies to new passwords, exisiting ones cannot be updated
#define BCRYPT_ITERATIONS  (12)

bool Password::checkPassword(std::string attempt, std::string hash) {
    if (attempt.size() == 0 || hash.size() == 0) // make sure these trivial cases result in the password being rejected
        return false;

    char new_hash[BCRYPT_HASHSIZE];
    memset(new_hash, 0, BCRYPT_HASHSIZE);
    crypt_rn(attempt.c_str(), hash.c_str(), new_hash, BCRYPT_HASHSIZE);
    return (hash == std::string(new_hash));
}

std::string Password::makeNewHash(std::string password) {
    duthomhas::csprng rng;
    char random[100];
    rng(random);

    char new_hash[BCRYPT_HASHSIZE];
    memset(new_hash, 0, BCRYPT_HASHSIZE);
    crypt_gensalt_rn("$2a$", BCRYPT_ITERATIONS, random, 100, new_hash, BCRYPT_HASHSIZE);

    std::string salt(new_hash);
    crypt_rn(password.c_str(), salt.c_str(), new_hash, BCRYPT_HASHSIZE);

    return std::string(new_hash);
}
