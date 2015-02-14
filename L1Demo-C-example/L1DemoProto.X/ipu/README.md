# IPU Tools

The PIC24F supports a subset of the RFC1951 DEFLATE compression algorithm.
These scripts can be used to convert an image to a compressed form that can be
decompressed by the IPU.

Note:  Currently produces 2bpp images.

## Converting an image into a raw format for the PIC24F

1.  First, take your image into GIMP (or equivalent).  Use `Image -> Mode ->
Indexed...` to set the image to only have 3 indexed colors.
2.  Now use `File -> Export As...` to export it as a C source file (in my case,
testimage.c).  Make sure to uncheck "use glib types".
3.  Modify makeraw.c to include the file you just saved.  Compile makeraw.c
(`gcc -o makeraw makeraw.c`) and run it, redirecting output (`./makeraw >
rawimg`).  It will print the index -> color mapping to stderr.
4.  The image is now suitable for our framebuffer!

## Compressing

If you want to see results immediately, you can skip this step and dump it
right into the framebuffer.

1.  Use deflate.pl to compress the raw image (`deflate.pl < rawimg >
rawimg.cmp`).

## Putting it in the PIC24F

1.  To get it into the source code, I made stringpack.pl to C array-ify a file.
So we hit it with that (`./stringpack.pl < rawimg`), and copy-paste the output
into our source file inbetween the braces:
```
__eds__ uint8_t gfx_compressed[] __attribute__((space(eds))) = {
    // HERE!
};
```
2.  Now it's ready for the IPU to decompress it onto the framebuffer, or
anywhere else.

