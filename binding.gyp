{
  "targets": [
    {
      "target_name": "bitarray",
      "sources": [
        "src/bitarray.cpp",
        "src/binding.cpp"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "cflags": ["-O3", "-march=native"],
      "cflags_cc": ["-O3", "-march=native"]
    }
  ]
}
