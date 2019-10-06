set(GCC_WERROR "-Werror")
set(MSVC_WERROR "/Wx")

set(GCC_WARNINGS
        -Wall
        -Wextra
        -Wno-attributes
        #-Wshadow
        -Wnon-virtual-dtor
        -Wdouble-promotion
        #-Wuseless-cast
        -Wnull-dereference
        -Wlogical-op
        -Wduplicated-branches
        -Wduplicated-cond
        -Wmisleading-indentation
        #-Wsign-conversion
        -Woverloaded-virtual
        -Wunused
        -Wpedantic
        -Wcast-align
        -Wconversion
        #-Wsuggest-override
        -Wno-switch)

set(MSVC_WARNINGS
        /permissive
        /W4
        /w14640
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /we4289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14928)

option(ENABLE_WARNINGS "Enable warnings" ON)

if(ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(${MSVC_WARNINGS})
    else(MSVC)
        add_compile_options(${GCC_WARNINGS})
    endif(MSVC)
endif(ENABLE_WARNINGS)
