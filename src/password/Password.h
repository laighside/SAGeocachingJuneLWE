/**
  @file    Password.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A collection of functions that are used for creating and checking password hashes
  All functions are static so there is no need to create instances of the Password object
 */
#ifndef PASSWORD_H
#define PASSWORD_H

#include <string>

// Minimum password length
#define MIN_PASSWORD_LENGTH  8

class Password {

public:

    /*!
     * \brief Checks a password aginst a hash and returns if is correct or not
     *
     * \param attempt The password entered by the user
     * \param hash The password hash from the database
     * \return true if the password is correct, false otherwise
     */
    static bool checkPassword(std::string attempt, std::string hash);

    /*!
     * \brief Creates a new hash for a new password
     *
     * \param password The new password
     * \return The hash of the new password
     */
    static std::string makeNewHash(std::string password);

};

#endif // PASSWORD_H
