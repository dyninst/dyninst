include(TestBigEndian)

test_big_endian(BIGENDIAN)
if(${BIGENDIAN})
    add_definitions(-DDYNINST_BIG_ENDIAN)
else()
    add_definitions(-DDYNINST_LITTLE_ENDIAN)
endif(${BIGENDIAN})
