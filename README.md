# node-gpgme [![Build Status](https://travis-ci.org/kiddouk/node-gpgme.svg?branch=master)](https://travis-ci.org/kiddouk/node-gpgme)
This module gives you access to GpgMe librairy and its underlying GPG backend. With this module, you can :

* Add and remove key to your keyring
* Cipher a payload valid for one or more recipients
* Decipher a payload
* Payloads can (but not necessary) be Ascii Armored if needed be

This module is aiming at (finally!) give access to gpg key manipulation for node user respecting a couple of JS good practices like:

* using promises for callbacks
* handling multiple "gpg context" (see the context as a session. One session could output everything in ASCII, while the other one can output everything in binary, or 2 sessions could use a different keyring if you want to).

## Requirements

* node-gyp
  * `npm install node-gyp -g`
* libgpgme11
  * linux: `apt-get install libgpgme11 libgpgme11-dev`
  * osx: `brew install gpgme`

## Installation

Simply run `npm install node-gpgme`. Make sure you have the requirements installed correctly.

## Configuration

The configuration objects allows you to specify the path of the keyring to use, if armored is to be used and the backend engine you want to use (so far, we only support OpenPGP).

```js
var GpgMe = require('gpgme')
var gpgme = new GpgMe({armored: true, keyring_path: '/tmp'});
```

## Key Manipulation ##

So far, you can only add a key (public or secret) and list those keys. Simple.

1. Adding a key

    var s = "-----BEGIN PGP PUBLIC KEY BLOCK ..."
    var fingerprint = gpgme.importKey(s)
    if (fingerprint === false) {
      console.log("Couldn't import key.")
    } else {
      console.log("Key fingerprint :" + fingerprint); 
    }

2. Listing keys
  ```js
  var keys = gpgme.listKeys();
  console.log(keys[0]);
  ```

  ```js
  { fingerprint: '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC',
      email: 'sebastien@requiem.fr',
      name: 'Sebastien Requiem',
      revoked: false,
      expired: false,
      disabled: false,
      invalid: false,
      can_encrypt: true,
      secret: false }
  ```

## Ciphering a message ##
You can cipher a message to *one* recipient at a time for the moment bu using the fingerprint of the key previously retrieved.

```js
var fingerprint = '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC';
var message = "Can you read this ?";
var cipher = gpgme.cipher(fingerprint, message);
```

```
-----BEGIN PGP MESSAGE-----
Version: GnuPG v2

hQEMA7xBZ+vX1VJHAQf/QpzEn8jwgWcuEgP/kF+NoihSOz5PDQbrf52EgykSajF4
XipaoqnceMrZpwkWTF9yZGcvCyMAX0pgiKNlThHloHsLkTjjq3L6/KFWk0odpG+C
UMer5X6yQsIjLsYGcWU2W8Qb6x4giX/v/yL4DGy6TYRb9tKf4r+0i2BD/1PrB2eN
qXhz6RFmbZg4qWjozyg2CYo5Bz2HDmF/mciRnejP/THCGKKmbf45LAZsS37Y07d6
cb35+YG0anwU/qZHDnrDsqlHTQ7+rdJui6KXobJpikAa873mziaqunDykl7Fve3l
26SzxiWhvgxk2+mhIW+syobFalLZCI40+ryAHvumhNJDATMw3MfGeZnBnRYZu+Ay
9EXXFCVn9A86Gli2B5gyYVk8kbAadfXAd8Vj+ysPw0in/HGoUH/NTDUp/C/SN4Nl
L4KxYQ==
=ACDC
-----END PGP MESSAGE-----
```



## Limitations
So far, this module not respecting the nature of node when fetching keys or encrypting large payload. If you have 1000 keys in your keyring, except things to block long enough to be noticed. Same goes for large messages to cipher.

As this is a very early release coded in few days only, I tried my best to deallocate memory blocks when possible but expect this module to leak for now.


## TODO
1. Use libuv for asynchronous keyfetching
2. Use libuv for asynchronous ciphering
3. Unit tests should be written both in JS and C++
4. Use key pattern for key a faster key retrieving



## Changelog
v0.0.6
------
* Fix an issue with a non inistialized memory allocated buffer for
encrypted payloads

v0.0.5
------
* Add travis-ci integration
* Change the attribute GpgMeContext to exports object (sorry folks)

v0.0.4
------
* Fix #1
* First version tracked by changelog
