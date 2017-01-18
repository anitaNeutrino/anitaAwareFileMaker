# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.



foreach(loop_prog addRunToMapFile               makeG12PosJsonFiles           makeOtherMonitorHkJsonFiles makeAcqdStartRunJsonFiles     makeG12SatJsonFiles           makeSSHkJsonFiles makeAdu5PatJsonFiles          makeGpsGgaJsonFiles           makeSlowRateHkJsonFiles makeAdu5SatJsonFiles          makeGpuSourceJsonFiles        makeSumTurfRateJsonFiles makeAdu5VtgJsonFiles          makeHeaderJsonFiles           makeSurfHkJsonFiles makeAvgSurfHkJsonFiles        makeHkJsonFiles               makeTurfRateJsonFiles makeEventJsonFiles            makeMonitorHkJsonFiles        makeWaveformSummaryJsonFiles makeTuffStatusJsonFiles makeGpuPowerSpectrumImages
)
add_executable (${loop_prog} ${loop_prog}.cxx)
target_link_libraries (${loop_prog} LINK_PUBLIC AnitaAware AnitaEvent AnitaCorrelator RootFftwWrapper ${ROOT_LIBRARIES} ${ZLIB_LIBRARIES} )
install (TARGETS ${loop_prog} DESTINATION ${UTIL_BIN_DIR})
endforeach(loop_prog )

option ( FORCE_OLD_GPP_ABI " Force old g++ ABI; this might be necessary if using new g++ with ROOT compiled with older g++ or other similar situations" OFF )
if (FORCE_OLD_GPP_ABI)
  add_definitions( -D_GLIBCXX_USE_CXX11_ABI=0 )
endif()
