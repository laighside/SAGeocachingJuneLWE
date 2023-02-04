/**
  @file    Email.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class of building and sending an email, supports attachments and inline images
  The actual sending of the email is done by opening a pipe to /usr/lib/sendmail

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#ifndef EMAIL_H
#define EMAIL_H

#include <string>
#include <vector>

// 20MB max size of email
#define EMAIL_MAX_SIZE  20000000

class Email {

public:
    /*!
     * \brief Email Constructor.
     */
    Email();

    /*!
     * \brief Email Destructor.
     */
    ~Email();

    /*!
     * \brief Checks if a string contains a valid email address or not
     *
     * \param email The string with the email address to check
     * \return True if the email address is valid, false otherwise
     */
    static bool isValidEmail(const std::string &email);

    /*!
     * \brief Adds a file from the file system as an attachment to the email
     *
     * \param full_filename The full filename of the file to attach, used for reading the data from the file system
     * \param filename The filename of the file, this is what the recipient sees as the name of the file they received
     * \param mime_type The MIME type of the file, if omitted, it will be automatically worked out from the file
     * \return Returns 0 if the file was added successfully
     */
    int addAttachmentFile(const std::string &full_filename, const std::string &filename, const std::string &mime_type = "");

    /*!
     * \brief Adds a base64 encoded file as an attachment to the email
     *
     * \param base64data The base64 encoded data of the file to send
     * \param filename The filename of the file, this is what the recipient sees as the name of the file they received
     * \param mime_type The MIME type of the file
     * \return Returns 0 if the file was added successfully
     */
    int addAttachmentBase64(const std::string &base64data, const std::string &filename, const std::string &mime_type);

    /*!
     * \brief Adds a file from the file system as an inline item to the email
     *
     * Used for images, etc. that are shown within a HTML email content
     *
     * \param full_filename The full filename of the file, used for reading the data from the file system
     * \param filename The filename of the file, this is the name used to reference the file in the HTML
     * \param mime_type The MIME type of the file, if omitted, it will be automatically worked out from the file
     * \return Returns 0 if the file was added successfully
     */
    int addInlineFile(const std::string &full_filename, const std::string &filename, const std::string &mime_type = "");

    /*!
     * \brief Adds a base64 encoded file as an inline item to the email
     *
     * Used for images, etc. that are shown within a HTML email content
     *
     * \param base64data The base64 encoded data of the file to send
     * \param filename The filename of the file, this is the name used to reference the file in the HTML
     * \param mime_type The MIME type of the file
     * \return Returns 0 if the file was added successfully
     */
    int addInlineBase64(const std::string &base64data, const std::string &filename, const std::string &mime_type);

    /*!
     * \brief Sets the content of the email in HTML format
     *
     * \param html The HTML content
     */
    void setHtml(const std::string &html);

    /*!
     * \brief Sets the content of the email in plain text format
     *
     * \param plain_text The plain text content
     */
    void setPlainText(const std::string &plain_text);

    /*!
     * \brief Sends the email
     *
     * \param to_address The email address to send the email to
     * \param reply_address The email address to put in the Reply-To header
     * \param subject The subject line of the email (for the Subject header)
     * \param mailer_address The email address to put in the From header
     * \param from_name The name of the email sender
     * \return Returns 0 if the email was sent successfully(*)
     */
    int sendEmail(std::string to_address, std::string reply_address, std::string subject, std::string mailer_address, std::string from_name = "");

    /*!
     * \brief Gets the number of attachments for the email
     *
     * \return The total number of attachments
     */
    unsigned int numberOfAttachments();

    /*!
     * \brief Calculates the total size of the email
     *
     * \return The total size in bytes
     */
    size_t emailContentSize();

private:

    // stores a base64 encoded file
    struct emailFile{
        std::string filename;
        std::string mimeType;
        std::string base64data;
    };

    std::vector<emailFile> attachments;
    std::vector<emailFile> inlineItems;

    std::string htmlBase64;
    std::string plainTextBase64;

    /*!
     * \brief This opens the pipe to /usr/lib/sendmail and writes the email data
     *
     * \param to_address The email address to send the email to
     * \param reply_address The email address to put in the Reply-To header
     * \param subject The subject line of the email (for the Subject header)
     * \param data The body of the email, of type Content-Type: multipart/mixed
     * \param mailer_address The email address to put in the From header
     * \param boundary_str The boundary string for the Content-Type: multipart/mixed data
     * \param from_name The name of the email sender
     * \return Returns 0 if the email was sent successfully(*)
     */
    int sendEmail(const char *to_address, const char *reply_address, const char *subject, const char *data, const char *mailer_address, const char *boundary_str, const char *from_name);

    /*!
     * \brief Makes a boundary string for Content-Type: multipart/ *
     *
     * \return The boundary string
     */
    std::string makeBoundaryString();

    /*!
     * \brief Inserts line breaks into a string so no line is long than the length given
     *
     * \param input The string to insert line breaks into
     * \param lineLength The length of each line
     * \return The string with line breaks
     */
    std::string splitLines(const std::string &input, unsigned int lineLength = 76);

};

#endif // EMAIL_H
