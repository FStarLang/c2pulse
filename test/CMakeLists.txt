enable_testing()

# Register a test runner that uses lit directly
add_test(NAME run-lit
  COMMAND lit -v ${CMAKE_CURRENT_SOURCE_DIR}
)
