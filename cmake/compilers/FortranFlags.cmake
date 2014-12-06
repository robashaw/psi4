if(NOT DEFINED DEFAULT_Fortran_FLAGS_SET OR RESET_FLAGS)

# custom flags are defined by setup --custom-fc-flags
if(DEFINED CUSTOM_Fortran_FLAGS)
    # set custom compiler flags (for every build type)
    set(CMAKE_Fortran_FLAGS "${CUSTOM_Fortran_FLAGS}")
    # special flags for build types will be empty
    set(CMAKE_Fortran_FLAGS_DEBUG "")
    set(CMAKE_Fortran_FLAGS_RELEASE "")
    set(CMAKE_Fortran_FLAGS_PROFILE "")
else()
    # custom flags are not defined
    if(CMAKE_Fortran_COMPILER_ID MATCHES GNU) # this is gfortran
	set(Fcheck_all "")
	# Try to compile with -fcheck=all if failing use 
	check_fortran_compiler_flag("-fcheck=all" has_check_all)
	if(has_check_all)
	   set(Fcheck_all "-fcheck=all")
	else()
	   set(Fcheck_all "-fbounds-check -fcheck-array-temporaries")
	endif()
	set(CMAKE_Fortran_FLAGS         "-fcray-pointer -DVAR_GFORTRAN -DVAR_MFDS -fno-range-check -fautomatic -fPIC -fimplicit-none -std=f2003 -ffree-form")
                                        # -fcray-pointer is for VAR_MPI2
	set(CMAKE_Fortran_FLAGS_DEBUG   "-O0 -g -fbacktrace -Wall -Wextra -pedantic-errors")
        set(CMAKE_Fortran_FLAGS_RELEASE "-O3 -funroll-all-loops -w -ftree-vectorize")
        set(CMAKE_Fortran_FLAGS_PROFILE "${CMAKE_Fortran_FLAGS_RELEASE} -g -pg")
        if(ENABLE_STATIC_LINKING)
           set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static")
        endif()
        if(ENABLE_BOUNDS_CHECK)
  	   set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${Fcheck_all}")
	endif()
        if(ENABLE_CODE_COVERAGE)
           set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fprofile-arcs -ftest-coverage")
        endif()
        if(ENABLE_VECTORIZATION)                                                    
	   set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${Fortran_ARCHITECTURE_FLAGS}")
        endif()	      
    elseif(CMAKE_Fortran_COMPILER_ID MATCHES Intel)
        set(CMAKE_Fortran_FLAGS         "-w -fpp -assume byterecl -DVAR_IFORT -fPIC -nosave -free")
        set(CMAKE_Fortran_FLAGS_DEBUG   "-O0 -g -warn all -traceback")
	# Check if xHost flag is available and add it CMAKE_Fortran_FLAGS_RELEASE
        set(xHost "")
        check_fortran_compiler_flag("-xHost" has_xHost)
        if(has_xHost)
	   set(xHost "-xHost")
        endif()
	set(CMAKE_Fortran_FLAGS_RELEASE "-O3 -ip ${xHost}")
        
        set(CMAKE_Fortran_FLAGS_PROFILE "${CMAKE_Fortran_FLAGS_RELEASE} -g -pg")
        if(DEFINED MKL_FLAG)
           set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${MKL_FLAG}")
        endif()
        if(ENABLE_STATIC_LINKING)
           set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static-libgcc -static-intel")
        endif()
        if(ENABLE_BOUNDS_CHECK)
           set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -ftrapuv -check all -fpstkchk")
        endif()
        if(ENABLE_VECTORIZATION)                                                    
	   set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${Fortran_ARCHITECTURE_FLAGS}")
        endif()	      
    else()
        message(FATAL_ERROR "Vendor of your Fortran compiler is not supported")
    endif()

    if(DEFINED EXTRA_Fortran_FLAGS)
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${EXTRA_Fortran_FLAGS}")
    endif()
endif()
    
save_compiler_flags(Fortran)
endif()
