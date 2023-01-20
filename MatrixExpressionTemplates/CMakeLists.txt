add_executable (MatrixExpressionTemplates "MatrixExpressionTemplatesTests.cpp" "MatrixExpressionTemplates.h" "catch.hpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MatrixExpressionTemplates PROPERTY CXX_STANDARD 20)
endif()