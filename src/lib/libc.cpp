static int errno;

extern "C" int* __llvm_libc_errno() {
    return &errno;
}
