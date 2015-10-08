include_directories(${CUPS_SRC_DIR})

set(SOURCE_FILES
        rastertokpsl.c
        halfton.c
        libjbig/jbig.c
        libjbig/jbig_ar.c
        unicode/ConvertUTF.c
        main.c)

add_executable(rastertokpsl-re ${SOURCE_FILES})
target_link_libraries(rastertokpsl-re ${CUPS_LIB} ${CUPSIMAGE_LIB})