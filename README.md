# Mail Server and Client

This repository contains the code for a simple mail server and client developed as part of the CS 39006: Networks Lab course by Sukhomay Patra(myself) and Abir Roy. The project implements SMTP (Simple Mail Transfer Protocol) and POP3 (Post Office Protocol version 3) to enable sending, receiving, and managing emails.

## Overview

The project comprises three main components:

1. **MailServer Machine Programs:**
    - `smtpmail.c`: SMTP server for receiving mails.
    - `popserver.c`: POP3 server for accessing and managing mailboxes.

2. **Home Machine Program:**
    - `mailclient.c`: Mail client for sending and receiving mails.

## Setup

1. **User Configuration:**
    - Create a file named `user.txt` with usernames and passwords.
   
2. **Directory Structure**:
    - Create subdirectories for each user.

## Usage

### MailServer Machine

#### smtpmail.c

- **Usage**: `./smtpmail <my_port>`
- Listens for incoming SMTP connections.
- Stores received mails in user mailboxes.

#### popserver.c

- **Usage**: `./popserver <pop3_port>`
- Runs a POP3 server to manage mailboxes.
- Supports commands like STAT, LIST, RETR, DELE, and RSET.

### Home Machine

#### mailclient.c

- **Usage**: `./mailclient <server_IP> <smtp_port> <pop3_port>`
- Provides options to manage and send mails.
- Supports managing mail, sending mail, and quitting.

---
