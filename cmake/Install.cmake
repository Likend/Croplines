# Disable CMP0207 Warning
install(CODE [[
    if(POLICY CMP0207)
        cmake_policy(SET CMP0207 NEW)
    endif()
]] INITIAL_CODE)

install(TARGETS Croplines RUNTIME)

# Collect dependencies in windows
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    install(TARGETS Croplines RUNTIME_DEPENDENCY_SET CroplinesDeps)

    install(RUNTIME_DEPENDENCY_SET CroplinesDeps
    DESTINATION bin
    DIRECTORIES
        $<TARGET_RUNTIME_DLL_DIRS:Croplines>
        $<PATH:GET_PARENT_PATH,${CMAKE_CXX_COMPILER}>
    PRE_EXCLUDE_REGEXES
        [[api-ms-win-.*]]
        [[ext-ms-.*]]
        [[kernel32\.dll]]
    POST_EXCLUDE_REGEXES
        [[.*[/\]system32/.*\.dll]]
)
endif()
