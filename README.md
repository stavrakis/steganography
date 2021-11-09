# steganography
Embed ASCII characters in PPM image files, using the least significant bit. To decode, the application compares the altered PPM with the original. Encoding outputs altered PPM to stdout. Decoding takes both files as arguments and outputs decoded text to stdout.

## Usage
To embed text:

	$ ./steg e [input PPM]

To decode:

	$ ./steg d [original PPM] [encoded PPM]
