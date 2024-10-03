SHIFTY
by deltaHzdelta a.k.a Gasmasked

This is a demo for my enigma-like text encryption software, SHIFTY.

When encrypted with any key, any text will become illegible and impossible to read.
The only way to access the original contents of the text is to reverse-shift the text with the exact same key and RNG config.

===
Features, implemented:

The software supports I/O with both console and filesystem, although I personally advise you use file-based I/O due to some fundamental limitations of console-based I/O.

The software can work with inputs of data and keys of arbitrary length, guaranteeing a possible 225^72,057,594,037,927,936 (so, a lot of) cryptographic sequences (in 1-byte encodings).

===
Features, planned:

The software currently only supports ASCII characters, or character sets within the 1-byte range. This will be made customisable, allowing for characters of Unicode format.

The software currently supports only one RNG config, which will be made partly customisable later.

===
Interesting applications and software limits:

A single file can be encrypted more than once, in different or the same direction using different keys. This makes it essentially impervious from any potential brute-force attempt.

You can 'two-way' encrypt a file - run a text file through a shift with one key, take some characters from it and put them into a text file, run the same original file with another key, 
take the rest of the data and append it to the file. You now have a file that partially decrypts when using one of two keys.

The encryption becomes less cryptographically secure when using over at least 72,057,594,037,927,936 (ASCII) characters in a key, as it causes an integer overflow in the seed used to make RNGs.
This can make the seed repeat previously taken values, making it possible to use a slightly different password to successfully decrypt data.

There's another file in this folder, titled README_dshft.dat; it's this file, encoded using a downshift via a key of "key". Try decoding it!
