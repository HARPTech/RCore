// Link with correct target glibc.
// https://stackoverflow.com/questions/2856438/how-can-i-link-to-a-specific-glibc-version
__asm__(".symver realpath,realpath@GLIBC_3.4.21");
