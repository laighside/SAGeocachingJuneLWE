/**
  @file    Email.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class of building and sending an email, supports attachments and inline images
  The actual sending of the email is done by opening a pipe to /usr/lib/sendmail

  This file is part of the SA Geocaching JLWE website, full details (including licence) can be found on Github.
  https://github.com/laighside/SAGeocachingJuneLWE
 */
#include "Email.h"

#include <cstdio>

#include "../core/Encoder.h"
#include "../core/JlweUtils.h"

#include <regex>     // regex used to validating email addresses
#define EMAIL_REGEX  "^[a-zA-Z0-9.!#$%&'*+\\/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$"


Email::Email() {
    // do nothing
}

Email::~Email() {
    // do nothing
}

int Email::sendEmail(const char *to_address, const char *reply_address, const char *subject, const char *data, const char *mailer_address, const char *boundary_str, const char *from_name) {
    FILE *mailpipe = popen("/usr/lib/sendmail -t", "w");
    if (mailpipe) {
        fprintf(mailpipe, "To: %s\r\n", to_address);
        if (from_name && strlen(from_name)){
            fprintf(mailpipe, "From: %s <%s>\r\n", from_name, mailer_address);
            fprintf(mailpipe, "Reply-To: %s <%s>\r\n", from_name, reply_address);
        }else{
            fprintf(mailpipe, "From: %s\r\n", mailer_address);
            fprintf(mailpipe, "Reply-To: %s\r\n", reply_address);
        }
        fprintf(mailpipe, "Subject: %s\r\n", subject);
        fprintf(mailpipe, "Mime-Version: 1.0\r\n");
        fprintf(mailpipe, "Content-Type: multipart/mixed; boundary=\"%s\"\r\n\r\n", boundary_str);

        fwrite(data, 1, strlen(data), mailpipe);
        fwrite(".\n", 1, 2, mailpipe);
        pclose(mailpipe);
        return 0;
     }
     return 1;
}

int Email::sendEmail(std::string to_address, std::string reply_address, std::string subject, std::string mailer_address, std::string from_name) {
    if (!isValidEmail(to_address) || !isValidEmail(reply_address))
        return 2;

    /** The structure is
        - mixed (set in sendEmail)
          -alternative
            -text
            -related
              -html
              -inline image
              -inline image
          -attachment
          -attachment
    **/

    std::string boundary_root = makeBoundaryString();
    std::string boundary_alt = makeBoundaryString();
    std::string boundary_html = makeBoundaryString();

    std::string emailContent;

    emailContent += "--" + boundary_root + "\r\nContent-Type: multipart/alternative;boundary=\"" + boundary_alt + "\"\r\n\r\n";

    if (this->plainTextBase64.size()) {
        emailContent += "--" + boundary_alt + "\r\nContent-Type: text/plain;\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: inline\r\n\r\n";
        emailContent += splitLines(this->plainTextBase64) + "\r\n\r\n";
    }

    emailContent += "--" + boundary_alt + "\r\nContent-Type: multipart/related;boundary=\"" + boundary_html + "\"\r\n\r\n";

    if (this->htmlBase64.size()) {
        emailContent += "--" + boundary_html + "\r\nContent-Type: text/html;\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: inline\r\n\r\n";
        emailContent += splitLines(this->htmlBase64) + "\r\n\r\n";
    }

    // inline images go here
    for (unsigned int i = 0; i < this->inlineItems.size(); i++) {
        emailContent += "--" + boundary_html + "\r\nContent-Type: " + this->inlineItems.at(i).mimeType + ";\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: inline\r\nContent-ID: <" + Encoder::filterSafeCharsOnly(this->inlineItems.at(i).filename) + ">\r\n\r\n";
        emailContent += splitLines(this->inlineItems.at(i).base64data) + "\r\n\r\n";
    }

    emailContent += "--" + boundary_html + "--\r\n";

    emailContent += "--" + boundary_alt + "--\r\n";

    for (unsigned int i = 0; i < this->attachments.size(); i++) {
        emailContent += "--" + boundary_root + "\r\nContent-Type: " + this->attachments.at(i).mimeType + ";\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\"" + Encoder::filterSafeCharsOnly(this->attachments.at(i).filename) + "\"\r\n\r\n";
        emailContent += splitLines(this->attachments.at(i).base64data) + "\r\n\r\n";
    }

    emailContent += "--" + boundary_root + "--\r\n";

    return sendEmail(to_address.c_str(), reply_address.c_str(), subject.c_str(), emailContent.c_str(), mailer_address.c_str(), boundary_root.c_str(), (from_name.size() > 0 ? from_name.c_str() : nullptr));
}

std::string Email::makeBoundaryString() {
    return JlweUtils::makeRandomToken(20);
}

std::string Email::splitLines(const std::string &input, unsigned int lineLength) {
    unsigned int lineCount = input.size() / lineLength;
    std::string result = "";
    for (unsigned int i = 0; i <= lineCount; i++) {
        result += input.substr(lineLength * i, lineLength) + "\r\n";
    }
    return result;
}

bool Email::isValidEmail(const std::string &email) {
    std::regex regex(EMAIL_REGEX, std::regex_constants::icase);
    if (std::regex_search(email, regex)) {
        return true;
    }
    return false;
}

int Email::addAttachmentFile(const std::string &full_filename, const std::string &filename, const std::string &mime_type) {
    std::string mimeType = "";
    std::string base64data = "";
    if (mime_type.size()) {
        mimeType = mime_type;
    } else {
        mimeType = JlweUtils::getMIMEType(full_filename);
    }

    FILE *file = fopen(full_filename.c_str(), "rb");
    if (file){
        uint8_t buffer[3072]; // size needs to be multiple of three
        size_t size = 3072;
        while (size == 3072){
            size = fread(buffer, 1, 3072, file);
            base64data += Encoder::base64encode(buffer, size);
        }
        fclose(file);
    }

    if (mimeType.size() == 0 || base64data.size() == 0)
        return 1;

    return this->addAttachmentBase64(base64data, filename, mimeType);
}

int Email::addAttachmentBase64(const std::string &base64data, const std::string &filename, const std::string &mime_type) {
    if (this->emailContentSize() + base64data.size() <= EMAIL_MAX_SIZE) {
        this->attachments.push_back({filename, mime_type, base64data});
        return 0;
    }
    return 1;
}

int Email::addInlineBase64(const std::string &base64data, const std::string &filename, const std::string &mime_type) {
    if (this->emailContentSize() + base64data.size() <= EMAIL_MAX_SIZE) {
        this->inlineItems.push_back({filename, mime_type, base64data});
        return 0;
    }
    return 1;
}

void Email::setHtml(const std::string &html) {
    this->htmlBase64 = Encoder::base64encode(html);
}

void Email::setPlainText(const std::string &plain_text) {
    this->plainTextBase64 = Encoder::base64encode(plain_text);
}

unsigned int Email::numberOfAttachments() {
    return this->attachments.size();
}

size_t Email::emailContentSize() {
    size_t total = 0;

    for (unsigned int i = 0; i < this->attachments.size(); i++)
        total += this->attachments.at(i).base64data.size();

    for (unsigned int i = 0; i < this->inlineItems.size(); i++)
        total += this->inlineItems.at(i).base64data.size();

    return total + this->plainTextBase64.size() + this->htmlBase64.size();
}
