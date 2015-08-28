# Node-Gpgme

This module gives you access to GpgMe librairy and its underlying GPG backend. With this module, you can :

* Add and remove key to your keyring
* Cipher a payload valid for one or more recipients
* Decipher a payload
* Payloads can (but not necessary) be Ascii Armored if needed be

This module is aiming at (finally!) give access to gpg key manipulation for node user respecting a couple of JS good practices like:

* using promises for callbacks
* handling multiple "gpg context" (see the context as a session. One session could output everything in ASCII, while the other one can output everything in binary, or 2 sessions could use a different keyring if you want to).
* 


## How does it work ? ##

Simply run `npm install node-gpgme`
Import the module with `var GpgMe = require "gpgme"`
Create a context with `var context = GpgMe({... options goes here ...})`
Profit !

