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
      "conditions": [
        ["OS=='win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "AdditionalOptions": ["/O2", "/arch:AVX2"]
            }
          }
        }, {
          "cflags": ["-O3", "-march=native"],
          "cflags_cc": ["-O3", "-march=native"]
        }]
      ]
    }
  ]
}
