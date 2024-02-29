DEFINE_MODULE(spectr.utils)

# Retreive git commit hash and put it to the Version header:
execute_process(
    COMMAND echo 10000
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE revisionOutput)

# Git commit hash length is 40 characters:
# https://git-scm.com/docs/hash-function-transition/
set(COMMIT_HASH_LENGTH 40)

string(SUBSTRING ${revisionOutput} 0 ${COMMIT_HASH_LENGTH} commitHash)
set(BUILD_COMMIT_HASH "\"${commitHash}\"")

set(CODEGEN_INCLUDE_DIR "${PROJECT_BINARY_DIR}/gen/")
configure_file("include/spectr/utils/VersionConstants.h.in" "${CODEGEN_INCLUDE_DIR}spectr/utils/VersionConstants.h")
target_include_directories(spectr.utils PUBLIC ${CODEGEN_INCLUDE_DIR})

add_subdirectory(test)
