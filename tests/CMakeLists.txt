include_directories(${SRC_DIR} ${CUPS_SRC_DIR} ${UNICODE_SRC_DIR})

set(TEST_FILES
        example.c
        test_unicode.c
        ${UNICODE_SRC_DIR}/ConvertUTF.c
        test_halfton.c
#        ${SRC_DIR}/halfton.c
        main.c)

# find_library(CUPS_LIB cups)
# find_library(CUPSIMAGE_LIB cupsimage)

add_executable(test-rastertokpsl-re ${TEST_FILES})
# add_definitions ("-Wundefined-internal")
# target_link_libraries(rastertokpsl-re ${CUPS_LIB} ${CUPSIMAGE_LIB})