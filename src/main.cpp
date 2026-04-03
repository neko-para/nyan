extern "C" void kmain() {
  static char *vgaBase = reinterpret_cast<char *>(0xC03FF000);
  // vgaBase[0] = 'A';
  vgaBase[0] = 'A';
  vgaBase[2] = 'B';
  vgaBase[4] = 'C';
}