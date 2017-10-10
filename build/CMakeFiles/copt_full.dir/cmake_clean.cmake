file(REMOVE_RECURSE
  "libcopt_full.pdb"
  "libcopt_full.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/copt_full.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
