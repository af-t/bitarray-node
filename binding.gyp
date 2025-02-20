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
    }
  ]
}
