var test = require("tape");
var GpgMe = require("..");

test('import_keys', function (t) {
    t.plan(2);
    
    var ascii_key = '-----BEGIN PGP PUBLIC KEY BLOCK-----\n\
Version: GnuPG v1\n\
\n\
mQENBFKY4I8BCAC4iNRHrSovmkw12T7YV0l86E1E7TeuFC6zlv3o4+e1Yb9KlYW/\n\
+TZ6VTVOB4zmey/459MRnj2dRv/CWrAdAMEq1Btt08+enIbSmXcYsQkusI5EO/UJ\n\
7O3uhO7UCHfQqGWi3VyfI2PSCnJoNoiRQreAiRWi0Ooh/ZuK9jfw4u0Oy8+Wp76U\n\
j1uSp8a27p2Q0jtzFh7NrmK3y89dDwVmmLlUg3YAzOY4xW1NJaDwEoh1HSfpQTsa\n\
I8yaqgH3BlrsLyZmn9Jg7Qxwe8aWclLVDxjtgIDttgmzYNtWBB4NXg51iM4PS5ZZ\n\
I19yv7Lg5hbdDWKAPyePH3PNCfdSyYyPh53BABEBAAG0Q1NlYmFzdGllbiBSZXF1\n\
aWVtIChTZWJhc3RpZW4ncyBvZmZpY2lhbCBrZXkpIDxzZWJhc3RpZW5AcmVxdWll\n\
bS5mcj6JATgEEwECACIFAlKY4I8CGwMGCwkIBwMCBhUIAgkKCwQWAgMBAh4BAheA\n\
AAoJEJ2sMr2CocnclAsH/iwkhfGArMse12H2IyVlXsxNxyInEFon5BWIOsxpY3/O\n\
J5J1NdosT7k+A4OJFgSihV1G4Hil03FRyCu4GLdbPg5afba2+GU3OIiU2WkiISjf\n\
7Sku0QxTprIB13NOCa1/3NFV9bOBUwXY8lhh9YlPFT8xWwSrC3fTo44+0Kqz6tLe\n\
5vhobT4LehThIPzZsYEOburUQ0ej6O4dvzqrqD6A731fs+zQUdBcu5FKHlc7TSzG\n\
m+pl1MCboW5mnJnw+qzz0b76xjXK4keDhkMjBOv6pYcJWRddwUVpqKivU5aX6szd\n\
EJSv/OsCFmREmzlK4GYnNfg3Nz4NeIkR2phZr17JFwu5AQ0EUpjgjwEIAOqBm1RA\n\
a6638Rex90ldrpvXLIZqYt0BBgLU5TKMxKK/dpjYVDvyG8i6hFJmnmqnlafoui1q\n\
katWNhoh/3QNZZXeSRewDUSz2Uw5loycIGl0Fhk6anlnnVCOuL4Lh7PIy0MdvfVb\n\
daF/rTdyMo3rjVmTWC9Tl/WUQdEYECwvCJq0XDNNO5LHFvF/4YWUm79ix4TK9W97\n\
2GjCPG06rh1ygFvJqYwbJtZn2lcHYc8Kt4BAKKewzQcdSjldK/wGs0nGB0vPp0xD\n\
OFzJvNeeayXAUAEGOJN/dyMiu4UxPQPRsRaoqfP8HoeB9/FtHKlLaXROlzsP+cxW\n\
rKEuDx5HSbo7JScAEQEAAYkBHwQYAQIACQUCUpjgjwIbDAAKCRCdrDK9gqHJ3L7K\n\
B/91rZbbSl1DpvT1LsbK/RiCsxeE2mrqUS9eV60y7hWuwBLUC+kFdY7O53TStSjH\n\
xn2Fm6/SAu4hkWNHRDejaiqWaipfUt0C9SnPhoiYscfkbEMwAYJOs/19ZCgvcwuF\n\
A9iO1zl9apMomkE1CXjk0wxEsrwwvgXkYqj8BhQq74KZIJkZisHA1jUVo5uKqs2p\n\
+nYEpBBsna3jWIMo0t0nS7yShOXTPzz/numy6HwBkZMfS7JyKJUT8s363FyJ9DSB\n\
hRdoZcvrkhSh5RKiADM8RfW04UEkb/tDHSuvkH7HrgdIzd/WQwDsZHnIjNMQAEMl\n\
oezv1L1SiuNonPtwiYcbphGo\n\
=2bdq\n\
-----END PGP PUBLIC KEY BLOCK-----',

        fingerprint = '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC',
        import_fingerprint,
        gpgme;

    gpgme = new GpgMe();
    import_fingerprint = gpgme.importKey(ascii_key);
    t.equal(fingerprint, import_fingerprint);

    ascii_key = "This is not a key";
    import_fingerprint = gpgme.importKey(ascii_key);
    t.equal(false, import_fingerprint);

});

test('list_keys', function (t) {
    t.plan(2);
    
    var keys,
        gpgme,
        fingerprint = '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC',
        email = "sebastien@requiem.fr";

    gpgme = new GpgMe();
    keys = gpgme.listKeys();

    t.equal(fingerprint, keys[0].fingerprint);
    t.equal(email, keys[0].email);
});

test('cipher', function (t) {
    t.plan(2);
    
    var keys,
        gpgme,
        fingerprint = '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC',
        payload = 'Can you read this ?',
        needle = '-----BEGIN PGP MESSAGE-----',
        cipher;

    gpgme = new GpgMe();

    cipher = gpgme.cipher(fingerprint, payload);
    t.equal(0, cipher.indexOf(needle));


    cipher = gpgme.cipher('unknown-fingerprint', payload);
    t.equal(false, cipher);
});
