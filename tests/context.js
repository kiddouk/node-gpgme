var test = require('tape');
var GpgMe = require('..');
var tmpFolder = '/tmp/' + Date.now().toString(36).toUpperCase(); // To avoid other keys in test.
var fs = require('fs');

fs.mkdirSync(tmpFolder);
fs.writeFileSync(tmpFolder + '/gpg-agent.conf', 'allow-loopback-pinentry\n');

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
        privateKey = '-----BEGIN PGP PRIVATE KEY BLOCK-----\n\
Version: GnuPG v1\n\
\n\
lQdGBFpgCP8BEAC5mYNNJd9TiOKr8yAVcaQ0Ahs24c8gn5av036u+7Amj6mSPs56\n\
QDg3dHluHb4GCEN5FG8ClWEk4IhLwZ8yjwyw9cjQg6sK0Own1yfjijD+49fs6h8S\n\
OnrU4rclYz6DdCnEFFyRbABUMQD6EzK7G9lCU7XjJmitMd9AhzPFfdPecqf8Kgqz\n\
PLC2ZIHWUdLbT7PpaMTv6rhJWL9+KSGplKad84YAKYyHvsBsTyqKCpZRPShSDsLd\n\
WAoDwjxtUVSq/HlZzCHsk4HhIQ7RdHFOgFrx6W7F7YP1sb97+XCbFnyzy6fjQZ0D\n\
iSMYcu2DPOurLlrmtkPt1hR97m/k3iFKF5NtytILo4W3pzsfkMVZZSD+0+AvKyqn\n\
Anla1WkgVEMes1nF2T9bG7hyFYLpS6gk8vmCoyiKT8f+6V9uOzijxV6Wp1nZEfoK\n\
FZuZVWNbdaVBTwSXgUEPAMaBuhjUGwDu81cT3k5f69v3L2+IR/BXmCVwPuXmFZ/k\n\
WwmGIlppCX4mxWju6gImtMjH6ePeYdLWGLoIipqb4IswfSQoLHihOsFwmlqp091E\n\
/6HCy6zJRzCGTEFd4Xxy2OMTXW+Vdoj/Y7icqya1TYrk0hhegcP/LYcoNlaT3Ebk\n\
lMcWL6Q3kJwGqVI2ZVQ/kc5i/c9BAErYclgg+pVHowXFKB7cTFEu1wMZawARAQAB\n\
/gcDAmYADRTYR3BGYLZhY04DbzECldp6fjOuCh7bxi8oxFUQp0/PekefIdN/SWvb\n\
jyhg2BHSL32dQMdaWo5/O6Zr81PZht0f1hde8DvgxpDCEqRa1BLG1AVGVydJ6as/\n\
TO+MI8U60U5DQsORcGwcoKtzm28luEglEmTV5wdlS84k+jBtGBSWDUMGHpswsu1q\n\
Ttf84VN03JC1ZHXBp/Kf2ywCHJF/MUtcy6YGxeRx45pLGR45hb711qRy1/R3SKPw\n\
WlH97vk3hB4alaKrzbSpJ/zKv2566we7rGuHVZVK0e+qP+UGCVhok4+NMHDCM38W\n\
9ZCiJKqpT4SQEVXoWCAdOoCFU2kJ3z3ooA8Va5VkEE5UVpp48TdOD4w9hqQTCf5m\n\
aDJ82zP1/KcXecjTm8UxLGvskXiA4+lPaN4zyXJMhK2TDxeYqRukBR44Lvupysiu\n\
K7DbtvtXDj79k/4oAmTIpYHkAJN7Yl4egdsxQn5uf0h09QwWPK4+QCU9qzwQC7gp\n\
pKRdxMDD+BZVCUhTZv+nVIC2YlnVhnikf/fSjFhEVHE8nwbZUMwJdnESMmciPd4E\n\
Oll43XmYjZL46SfjzYrWrXLf4muE6LgfJCr4qS98TsVNLB+LlzCppv3I96NWKkQX\n\
u3Gp8Vbw3hHZtJ54DsvaPt1Rc3dGhNu5/Xgp604Hm96sy70TgV1zfsJTOl49JdSj\n\
6ls4cfxvSjZvovZV98W4sKYOygdZ91u+GtvfFLVp5MLHSNiPmiMvMhHX3AurSV0o\n\
dmFN++iyR0lUS5WonVzgGh1RqR5Rx4QQZX3qFnJNULZZUCulLZ35VNNgc8dN3Zsg\n\
FpfEg6le054SXycli6ycF2ay9KZjdfPcLhcBPv2BIsAnT2+ZYJxjS44g5JrJKWzK\n\
CvYg3OUJGf1f+mDCi/9bV7zVq4Cl13cjPCAk8C7AusDxAMRJ7aNIsE6snCtCZZZj\n\
41NeZn3CjCs7iVegH3QJ2ULZhUhmE6Klr/Zw0SCfnylLxklyG7dvcSOyw6jZJpNB\n\
PD2etGoeF3PtcMRP8vCc1kwn1VkZapUMBWcG9hoi0NiZmoRKngRkdRyNBqkPMef1\n\
RVBhtZ9Nr4lXmdo7eqEo0ptU24SPD3tSkLGE5XNQdHLMCRAHYR3lWVc2oBkHL44T\n\
t58KFBH3u4UU5AQcVtWyL0iGvlzBSng3uh0MP2E36+OAPE/RnhQkxO+g0clpliGM\n\
FabaQz0opRc3C2JJaXWDSzkMONCl0vP8GV6rYX+mfr93zDzsrWFeFPtmC1F2ObvW\n\
Eq6VCg/pw+H7tcJ8ecO/fhITiLgyR7IdKc+arNysuzdvJ9z5woWAqb/gQictvSEE\n\
RMBQCek8nQapOj5IXM46+6O5f2bxY3uUFanwD9+ZteWdwNzuQowmtD5Ax5KIb7fW\n\
fmcw49Z+HFQtdw4YvsG+KE6MQ9DyQliy17I/I8wmoutgBGxVYLogKaQjpcfaEuAO\n\
4puaC1TVHcXyQIPYKi6icP0T4zqVEENtv0d8Y7BRbtInv69rRGqYmMPK7dcQWFQq\n\
FX4OiN26FZgUZLiZM9sr+ORrq/Cdo6VgzdShj/35ga9MNa8ZDVh1elvLJCUtqpnL\n\
IkaFkONaxRStSK9TQ6sFkA4B0Q+iJV4I1OwW29ITF+9gV3Z0g5jMvu1Se6WevRC6\n\
83Z8F9KFjbiyRcq5cC2rIkYeXJg5mUymHTsFqOVCmaEdl9pV51Zpg4+5hKAPisrn\n\
b5sjk5g+I8u8aJbu8FmhZCLac6Kz8/Vs78/zFZd57AJD5vOTEccvNF60EkJlbmNo\n\
bWFyayBUZXN0IEtleYkCOAQTAQIAIgUCWmAI/wIbAwYLCQgHAwIGFQgCCQoLBBYC\n\
AwECHgECF4AACgkQ1zYrTMVG2xFKlA/+Pm66EiTdYKnOlWmufi2rynt+4ljGlkZ8\n\
ccikVoBKE4v0cdr2Imj4aQXSdmMLNn2XaCJH2824+qkBuSL6z87FjzgV9hIR1IZG\n\
LEI5CqA7AxdKT7AeDlvmt8NXXIhQTGahSF0uykK/Vw9tUF2qm2+i16tzDA6wiBYh\n\
hIrj39ZsYboeT2wWpK9SY0bE2wJKRXj6i+P2TFuM7eBx7nQvu7Zobk1aQQTs4OS8\n\
dOgXYt1SHAjEbjGDKv1JMseHMicvjVXUb111Sexw72TYLskIm/g0x7ucRFig7eln\n\
/lA+FU2QBcWTE2ccyYy5qteCCQEbmtSkkDinNQGI7H7nfYskf9cKCcc7uyel0FmE\n\
f0dzTMQtE02tVNiHmPsP9mOrNwXSbWwA5lVMSr1ZlxA/RcGxAKeH2ILyqY0q6aRg\n\
s8S3mwKIvNfTZy3QtJVtTIJdcnr++Yrmg/Z9ktefMnjXrNRAZVCIx59R8GlNfH1K\n\
XgkBJKhTd0eVtytgHZ+sK5JQysER2P6Xbj6cpxfoBlwKZlOTG1g7gm9sa1OO1uni\n\
Wuj1RMlQ+TiwqVEMG+xy+p9ORRJ71/WRlDycNCp+lISBh+aiBnNClYc2J4bG6e/z\n\
9fkIdhgk2eX6p42SemWE6wKu0rThgtaeyzIlpglP2ciB5TEojW+XBm0pfiSB0dv2\n\
RXlLMxx5ajKdB0YEWmAI/wEQAKthl4wxZrORhEKqhkivEYLieOu4/N38TSGD125+\n\
mHsuYt5QQBx3QOxtfcYDFabVdVN9NHt1/1y0g4gIqEjNO37VJcpmV0vFf2H4pUn1\n\
ybhhn9rJwpIJNM2ju6LSZMzwLQHFnAaag0cCaJl9hXFUa10zseJwcL86QMwVbhtu\n\
NdwaHTljppek887U/2AmiidoLJg7aT5Tgf0Lt5bQ2STjtQRnLy07VzefPjhNqMdQ\n\
mffA4YVehUoe/Z5RyNA7CnlRV8CetqngxEnM4IyW+j2J5/9be3WrFmKSlneNJkhW\n\
IQZEs8mC2YJ8xnz3lNxP2fqnQaBguzyQKVVBbmbbHi1fVewLrQOcsIEsT7wa7e8B\n\
rkrewMtCpoPM6xdDbn+nponFMPwkSP3rCSlJieKoKmpnQmrJJoTJGO0cHwpLXHMj\n\
wADf6u/Sz8IM2cerzwE2umfvVJu7vndnpJ58NsbydmN+8s2KULPHdBOMxoU2vLr+\n\
2OFBsR05KimEOGmBI1RohRku48x9lQtU/+qFPVwsOCuP2BlM+jNsnQnKouVxhCCn\n\
ZtjVL2aPE3oCn+fciAXFw5ynb9+c3t/u0Gqwb6Ky4quRLIEngZ21g/IwEBV4FqqX\n\
SErCGY1nlk0EJrBy6nAEeEHXqR59Z+H1xupJXpCbK6kss2P7PTqbtWNnLa6T2vWB\n\
KbzNABEBAAH+BwMCZgANFNhHcEZgK81to5HJjtWRdzCTX3SxzGyvroUbXtmiPq9l\n\
tN8AP50ImDrdT0SdGc9Jsek30fKNoC8vIN7uaf73GTRu0fgwu5f7/KW7i38Lq+cN\n\
VFPCuAjQUGKJVadUmqX3cW8gunrD59gvhqMS9i8lbmB95fjWhthvQPBZmVG4N/EX\n\
LfeaxRL0xeC/PNGfHoRYl7C+HaAdr5IY8DiVwlMoRHZfIp+5CSfvL9UBrhmBAPfz\n\
KXVS+hs+aCpur7CwxT8DvWJuinZTAMxW0MzJPGPucXC2kaoRVntsQhSh/nsY06ci\n\
0GMwGKqAHAXqRUCjRhyVTjgcQivyVP5WxpgM8ZWIfrx7riF+a5dzpTLIb6SaFSm+\n\
CUgHo57aXB6Xn4JldLyOpnzOCnsrg5Yu35ntfUtMEBX/U7h6eB0hogQsUmyUATRo\n\
dSAjhSWm8Vsnt9wKRCORzPcn7yzuV1sS7LA/CmPxu7GGGnGqrvXR1Imqml6MIZUt\n\
1nyiVXPzdiXI4dtA21XYAWnfw0ZpJ37To/GepnEH9QitOArkidNPFheTEfAWrPMr\n\
I2HzNbjI5VwWpH0++byZIiWX3vORh0iKO7sJdZUaAI90mffuSSz2g+AkSMU6OPf+\n\
aG6YqOyNhy/bbBrR0jzkwD+4NIRW0BUO6O+ofeE7swB5U8HHhXOxCv5rC/lSmV9n\n\
rACaSDE8GzKZv3BI2qqteE/fnLuw4/YL6+viKg4n32usvuP5NEQ+o4Fk4eNnhxXd\n\
75EHVVY5+QE9cUk+IOMKIFWV52n7af7KGbpz6zPTtUGA8CspiXDJ98Bj99sRT4Rg\n\
9Zbu3HhJthd+RirZ10uR3lFIj7KgXpIdGlVBsLrMcX62pn+9etHoxgNgnM75p0UI\n\
L6SfUxA47a/RAUrvWv9yzBrRojd6kRHdiA0XiEWOO/Rrm+T0YW+wDB76/b2WgN7Q\n\
Tualt2BXAIkKsIPIrs+lECPDl4oQh8m2OSJjX3/c2vTNdujvNBk6SZu1VsvohF/0\n\
kO+R68e8mthYBJAI8ro/BIafhSviPyeLPvdx8oaxpfUS0yKc+FClOiX8ZCYrliWH\n\
ssXA6HVJPmVu58Hw6y1NDF2GUlMo6o00fXw2QiyiMXFN4vIKR8/eCUvQJREVKH/o\n\
6HfkUDQ5a6z0THrLVNPaItYvlKC72qgoGRfKQuSCvnUjoFTZOjTPfCO+5fJhKRHE\n\
c8tZR9HEUtX3CbzWbyYzSYOxDsF5doVo0aSWDbbwX2sfW8OOUaUAjDAK6H56uSjA\n\
0lKFIluLdZQg2TvUzwCMTxWVpTkm4mmGNZrppwaUXRiE25MB/6BZbw9QhCNVPtZ7\n\
Si3fqK7ujJ2w0v15CSPmSVfbIeRbLv9epFF0ACPunGQjyrZrg9ZkKYRJAToZHEGC\n\
q1gk9B/PLDZo4mSxl3e3/HIzxB8rf3MW7V4NmZzJjLfYasFkXNs69iSQ4jP9Qdl0\n\
R518vs4RSkrlGJAHRhbgOfa3TWAARDBYAyQkoF/xcd4aD/sHRtlSbgXSkp11o6LU\n\
1d0oS+/iUWW+aK/PJX2ZbR/x05GpFiQ+YbJqDUPRL9TDGzA8E+5CY/U/s9UxQcpk\n\
28b3XQdFTBve6qCIsDRfRkv8e3rB8HLdbN1bkfoEIFt7SsyPQ31YKOfghNeROIsA\n\
f1tZTOqxAMkBhh6c+cd3o0MN43u+lUF8bvloiZ32nb3nHvjrD34duVfmgDvEVc6v\n\
+14J8lgVcE/dsXdDLHqGf0XVQFMGB70f1b276HHn55zUf1pTEXROBaf+YFjX7BHw\n\
5okCHwQYAQIACQUCWmAI/wIbDAAKCRDXNitMxUbbESd8D/wNjMzc5B2BkIZ+MWbX\n\
b56plIfFlzbZyA4EHqDf3lyazh2YmDCyWQSlPz3CSXnrCmEfPY/PyppE8nScEUwf\n\
lBoEjCFh3Q7EypplQYQb7NRYjebi64Ie40NXc1PmqbW49nRyQS2xnFZO5NZyoBvt\n\
Qm3s/WUxoirxIADWNNBUSHZkH9XLCJc+8ppEVoW2oqeykVnTqG1qI2jz3ZinVpCU\n\
NmxZsKGT4dA3VjEsC5lzfXPArbUL35ZLyk+oG73JBKZqRWzXcyyb79cQ3LVXkSrl\n\
Mfuuw0fPbX//q6De6bFvEqpheES68PrZs3zM2ZbMx4hl3yS3qJl9C1gEZRC8XX2k\n\
iMS3NX0+m1RKWE8FFwLMhw6yi+lTaKA1rT2Jk+GHDTxTp0McwP/Fg2SeDgigH9FF\n\
lsbm2MRWD2OxqrA9qPIPI1Ti1KlQDDSY1/0b10EPaA6VFV0YTFOSZYBZ0MktL+Z+\n\
KB5hFqQSZWYfzzjwW8PSeS2IcV8yIrosim19tb9mLplyULZJXZ8X2TIsXLRelMBq\n\
8bZW75dcgT3z8iFtFMyHACnBexjXatIgzHHZMYJSlmyLSuTHXzeKo72hP40y3Xkf\n\
uugqVOWHeE7v7ARMu1mhXS6qWzZmxsjixV1d0kXSo9LzUyFqNtkasUiL2aoXQ70z\n\
lbMia0X7KJbYnbG5XLEDiMjzDQ==\n\
=SC0f\n\
-----END PGP PRIVATE KEY BLOCK-----',
        fingerprint = '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC',
        privateKeyFingerPrint = '629307840E6D187E6388192FD7362B4CC546DB11',
        import_fingerprint,
        gpgme;

    gpgme = new GpgMe({
        keyring_path: tmpFolder,
    });
    import_fingerprint = gpgme.importKey(ascii_key);
    t.equal(fingerprint, import_fingerprint);

    ascii_key = 'This is not a key';
    import_fingerprint = gpgme.importKey(ascii_key);
    t.equal(false, import_fingerprint);

    private_key_fp = gpgme.importKey(privateKey);
});

test('list_keys', function (t) {
    t.plan(2);

    var keys,
        gpgme,
        fingerprint = '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC',
        email = 'sebastien@requiem.fr';


    gpgme = new GpgMe({
        keyring_path: tmpFolder,
    });
    keys = gpgme.listKeys();

    t.equal(fingerprint, keys[0].fingerprint);
    t.equal(email, keys[0].email);
});

test('cipher', function (t) {
    t.plan(2);

    var gpgme,
        fingerprint = '3B2302E57CC7AA3D8D4600E89DAC32BD82A1C9DC',
        payload = 'Can you read this ?',
        needle = '-----BEGIN PGP MESSAGE-----',
        cipher;

    gpgme = new GpgMe({
        keyring_path: tmpFolder,
    });

    cipher = gpgme.cipher(fingerprint, payload);
    t.equal(0, cipher.indexOf(needle));


    cipher = gpgme.cipher('unknown-fingerprint', payload);
    t.equal(false, cipher);
});

test('signing', function (t) {
    t.plan(1);

        var gpgme,
        fingerprint = '629307840E6D187E6388192FD7362B4CC546DB11',
        payload = 'Can you read this ?',
        needle = '-----BEGIN PGP MESSAGE-----',
        cipher;

    gpgme = new GpgMe({
        keyring_path: tmpFolder,
    });

    var signed = gpgme.sign(fingerprint, payload, "123456");
    t.equal(0, signed.indexOf(needle));
})
