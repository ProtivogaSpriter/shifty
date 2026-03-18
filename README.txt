SHIFTY
by deltaHzdelta a.k.a Gasmasked

This is a demo for my encryption software, SHIFTY.

When encrypted with any key, any data will become illegible and impossible to read.
The only way to access the original contents of the data is to reverse-shift the data with the exact same key and RNG config.

===
Features, implemented:

The software supports I/O with both console and filesystem, although I personally advise you use file-based I/O due to some fundamental limitations of console-based I/O.

The software can work with inputs of data and keys of arbitrary length, guaranteeing a possible 225^72,057,594,037,927,936 (so, a lot of) cryptographic sequences (in 1-byte encodings).

The software supports ASCII, UTF8, UTF16BE and UTF16LE encodings.
Warning: using this software with any Unicode standard will massively increase filesize due to the encryption algorithm. 
You can use ASCII mode on UTF8/UTF16 files to keep their original size, however that may result in invalid byte sequences.

The software can encrypt any file - however, be sure to use the ASCII encoding for such purposes. 
You can also use DATA or BYTE when declaring the standard for this, they're the same thing.

(to be completely honest, utf8/16 encodings are silly and meaningless. they're more of a torture i imposed on myself than an actual feature here. don't use them.)

===
Features, planned:

The software currently supports only one RNG config, which will be made partly customisable later.

The software currently only supports rudimentary command-line functionality. A full TUI will be made later.

The software currently ignores control characters, but increments the random number engine used in encryption per each such character regardless. 
Both disabling control character ignore and stopping the engine from incrementing under the control character limit will be made into options later.

===
Interesting applications and software limits:

A single file can be encrypted more than once, in different or the same direction using different keys. This makes it essentially impervious from any potential brute-force attempt.

You can 'two-way' encrypt a file - run a text file through a shift with one key, take some characters from it and put them into a text file, run the same original file with another key, 
take the rest of the data and append it to the file. You now have a file that partially decrypts when using one of two keys.

The encryption becomes less cryptographically secure when using over at least 72,057,594,037,927,936 (ASCII) characters in a key, as it causes an integer overflow in the seed used to make RNGs.
This can make the seed repeat previously taken values, making it possible to use a slightly different password to successfully decrypt data.
(actually, this would not actually naturally occur ever, because it would require 72 petabytes of ram to store the key.)

A specific edge case bug can be attributed to text file formatting; while Windows systems use CR/LF to declare newlines, Unix uses only LFs.
This makes any text encrypted on Windows (or using the CR/LF scheme in general) become utterly illegible on Unix past the first line, even if the correct key is used. 
Encrypt everything in Unix format, for your own sake.

There's another file in this folder, titled README_dshft.dat; it's this file, encoded using a downshift via a key of "key". Try decoding it!
