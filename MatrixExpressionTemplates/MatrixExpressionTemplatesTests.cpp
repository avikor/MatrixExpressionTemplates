#include "MatrixExpressionTemplates.h"


#define CATCH_CONFIG_MAIN
#include "catch.hpp"


TEST_CASE("size")
{
	constexpr met::MatrixSize matSize0{ 4, 3 };
	met::Matrix<int> mat0{ matSize0 };

	met::MatrixSize matSizeRes{ mat0.size() };
	REQUIRE(matSize0 == matSizeRes);

	met::MatrixSize matSizeRes2{ std::size(mat0) };
	REQUIRE(matSize0 == matSizeRes2);
}

TEST_CASE("fill")
{
	constexpr int val{ 2 };

	constexpr met::MatrixSize matSize{ 3, 4 };
	met::Matrix<int> mat1{ matSize };
	mat1.fill(val);

	for (int i{ 0 }; i != matSize.rows; ++i)
	{
		for (int j{ 0 }; j != matSize.cols; ++j)
		{
			REQUIRE(mat1(i, j) == val);
		}
	}
}

TEST_CASE("isEmpty")
{
	constexpr met::MatrixSize zeroMatSize{ 0, 0 };
	constexpr met::MatrixSize matSize{ 3, 4 };
	met::Matrix<int> mat1{ matSize };
	REQUIRE(!mat1.isEmpty());

	met::Matrix<int> mat2{ zeroMatSize };
	REQUIRE(mat2.isEmpty());
}

TEST_CASE("range-for")
{
	constexpr int val1{ 17 };
	constexpr int val2{ 3 };
	constexpr met::MatrixSize matSize{ 4, 3 };
	met::Matrix<int> mat1{ matSize };
	mat1.fill(val1);

	for (int elem : mat1)
	{
		REQUIRE(elem == val1);
	}

	for (int& elem : mat1)
	{
		elem = val2;
	}

	for (int elem : mat1)
	{
		REQUIRE(elem == val2);
	}
}

TEST_CASE("transpose && expression ctor && expression assignment")
{
	constexpr met::MatrixSize matSize0{ 4, 3 };
	met::Matrix<int> mat0{ matSize0 };

	int val{ 0 };
	for (int& elem : mat0)
	{
		elem = val++;
	}

	auto mat0Transposed = transpose(mat0);
	met::Matrix<int> mat0TransCopy{ mat0Transposed };

	met::MatrixSize matTransposedSize{ mat0Transposed.size() };
	REQUIRE(matTransposedSize.rows == matSize0.cols);
	REQUIRE(matTransposedSize.cols == matSize0.rows);

	for (int i{ 0 }; i != matTransposedSize.rows; ++i)
	{
		for (int j{ 0 }; j != matTransposedSize.cols; ++j)
		{
			REQUIRE(mat0Transposed(i, j) == mat0(j, i));
			REQUIRE(mat0TransCopy(i, j) == mat0(j, i));
		}
	}

	REQUIRE_THROWS_AS(mat0 = mat0Transposed, met::mat_expers_size_diff_exception);

	auto mat0TransposedTransposed = transpose(mat0Transposed);

	for (int i{ 0 }; i != matSize0.rows; ++i)
	{
		for (int j{ 0 }; j != matSize0.cols; ++j)
		{
			REQUIRE(mat0TransposedTransposed(i, j) == mat0(i, j));
		}
	}
}

TEST_CASE("copy ctor")
{
	constexpr met::MatrixSize matSize{ 4, 3 };
	met::Matrix<int> mat1{ matSize };
	mat1.fill(17);

	met::Matrix<int> mat2{ mat1 };

	REQUIRE(mat1 == mat2);
}

TEST_CASE("copy assignment")
{
	constexpr met::MatrixSize matSize{ 4, 3 };
	met::Matrix<int> mat1{ matSize };
	mat1.fill(2);

	met::Matrix<int> mat2{ matSize };
	mat2.fill(15);

	REQUIRE(mat1 != mat2);

	mat2 = mat1;

	REQUIRE(mat1 == mat2);

	constexpr met::MatrixSize matSize3{ 9, 10 };
	met::Matrix<int> mat3{ matSize3 };

	REQUIRE_THROWS_AS(mat1 = mat3, met::mat_expers_size_diff_exception);
	REQUIRE_THROWS_AS(mat3 = mat1, met::mat_expers_size_diff_exception);
}

TEST_CASE("move ctor")
{
	constexpr int val{ 13 };
	constexpr met::MatrixSize matSize{ 4, 3 };

	met::Matrix<int> mat1{ matSize };
	mat1.fill(val);

	met::Matrix<int> mat2{ std::move(mat1) };

	REQUIRE(mat1.isEmpty());
	REQUIRE(mat2.size() == matSize);

	for (int elem : mat2)
	{
		REQUIRE(elem == val);
	}
}

TEST_CASE("steal")
{
	constexpr int mat1Val{ 2 };
	constexpr int mat2Val{ 15 };
	constexpr met::MatrixSize matSize{ 4, 3 };
	met::Matrix<int> mat1{ matSize };
	mat1.fill(mat1Val);

	met::Matrix<int> mat2{ matSize };
	mat2.fill(mat2Val);

	REQUIRE(mat1 != mat2);

	mat2.steal(std::move(mat1));

	REQUIRE(mat1.isEmpty());

	for (int elem : mat2)
	{
		REQUIRE(elem == mat1Val);
	}

	constexpr met::MatrixSize matSize3{ 7, 8 };
	met::Matrix<int> mat3{ matSize3 };

	REQUIRE_THROWS_AS(mat3.steal(std::move(mat2)), met::mat_expers_size_diff_exception);

	REQUIRE(!mat2.isEmpty());
}

TEST_CASE("operator+")
{
	constexpr met::MatrixSize matSize{ 3, 4 };
	met::Matrix<int> mat1{ matSize };
	mat1.fill(2);

	met::Matrix<int> mat2{ matSize };
	mat2.fill(6);

	auto view = mat1 + mat1 + mat2;
	met::MatrixSize viewSize{ view.size() };

	for (int i{ 0 }; i != viewSize.rows; ++i)
	{
		for (int j{ 0 }; j != viewSize.cols; ++j)
		{
			REQUIRE(view(i, j) == mat1(i, j) + mat1(i, j) + mat2(i, j));
		}
	}

	constexpr met::MatrixSize matSize2{ 8, 7 };
	met::Matrix<int> mat13{ matSize2 };
	mat13.fill(16);

	REQUIRE_THROWS_AS(mat1 + mat13, met::mat_expers_size_diff_exception);
}
