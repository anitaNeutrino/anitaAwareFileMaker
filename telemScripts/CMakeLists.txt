# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
foreach(script processRunTelem.sh processConfigTelem.sh  )
  install(PROGRAMS ${script} DESTINATION ${UTIL_BIN_DIR})
endforeach(script)

 
