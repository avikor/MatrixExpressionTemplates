#ifndef MAT_EXPRESSION_TEMPLATES
#define MAT_EXPRESSION_TEMPLATES


#include <stdexcept>
#include <utility>


namespace met
{
	struct MatrixSize
	{
		int rows;
		int cols;
	};

	// copy by value since sizeof(MatrixSize) is small
	bool operator==(const MatrixSize left, const MatrixSize right)
	{
		return left.rows == right.rows && left.cols == right.cols;
	}


	class mat_expers_size_diff_exception : public std::invalid_argument
	{
	public:
		mat_expers_size_diff_exception()
			: std::invalid_argument{ "matrix expressions must have the same size." }
		{ }
	};

	template<typename T, typename Expression>
	concept ExpressionConcept = requires(const Expression& expression, const int row, const int col)
	{
		{ expression.size() } -> std::same_as<MatrixSize>;
		{ expression(row, col) } -> std::same_as<T>;
	};

	template <std::semiregular T, typename Expression>
	class MatrixExpression
	{
	public:
		T operator()(int row, int col) const noexcept
		{
			return static_cast<const Expression&>(*this)(row, col);
		}

		MatrixSize size() const noexcept
		{
			return static_cast<const Expression&>(*this).size();
		}
	};


	template <std::semiregular T>
	class Matrix : public MatrixExpression<T, Matrix<T>>
	{
	public:
		explicit Matrix(const MatrixSize matSize);
		
		template <typename Expression>
			requires ExpressionConcept<T, Expression>
		Matrix(const MatrixExpression<T, Expression>& matrixExpr);

		template <typename Expression>
			requires ExpressionConcept<T, Expression>
		Matrix& operator=(const MatrixExpression<T, Expression>& matrixExpr) noexcept(false);

		Matrix(const Matrix& other);
		Matrix& operator=(const Matrix& other);

		Matrix(Matrix&& other) noexcept;

		Matrix& operator=(Matrix&& other) noexcept = delete;

		// move assignment alternative
		// since we throw an exception if other has different Matrixsize
		void steal(Matrix&& other) noexcept(false);

		~Matrix();

		T* begin() noexcept;
		T* end() noexcept;

		[[nodiscard]] T operator()(const int i, const int j) const noexcept;
		T& operator()(const int i, const int j) noexcept;

		[[nodiscard]] MatrixSize size() const noexcept;

		void fill(const T val) noexcept;

		[[nodiscard]] bool isEmpty() const noexcept;

	private:
		MatrixSize matSize_;
		T* arr_;


		template <typename Expression>
			requires ExpressionConcept<T, Expression>
		void assign_from_mat_expression(const MatrixExpression<T, Expression>& matrixExpr);
	};

	template<std::semiregular T>
	bool operator==(const Matrix<T>& left, const Matrix<T>& right)
	{
		const MatrixSize leftSize{ left.size() };
		if (leftSize != right.size())
		{
			return false;
		}

		for (int i{ 0 }; i != leftSize.rows; ++i)
		{
			for (int j{ 0 }; j != leftSize.cols; ++j)
			{
				if (left(i, j) != right(i, j))
				{
					return false;
				}
			}
		}

		return true;
	}

	template <std::semiregular T>
	template<typename Expression>
		requires ExpressionConcept<T, Expression>
	void Matrix<T>::assign_from_mat_expression(const MatrixExpression<T, Expression>& matrixExpr)
	{
		for (int row{ 0 }; row != matSize_.rows; ++row)
		{
			for (int col{ 0 }; col != matSize_.cols; ++col)
			{
				(*this)(row, col) = matrixExpr(row, col);
			}
		}
	}

	template <std::semiregular T>
	Matrix<T>::Matrix(const MatrixSize matSize)
		: matSize_{ matSize }
		, arr_{ (matSize_.rows == 0 || matSize_.cols == 0) ? nullptr : new T[matSize_.rows * matSize_.cols] }
	{ }

	template <std::semiregular T>
	template<typename Expression>
		requires ExpressionConcept<T, Expression>
	Matrix<T>::Matrix(const MatrixExpression<T, Expression>& matrixExpr)
		: Matrix<T>{ matrixExpr.size() }
	{
		assign_from_mat_expression(matrixExpr);
	}

	template <std::semiregular T>
	template<typename Expression>
		requires ExpressionConcept<T, Expression>
	Matrix<T>& Matrix<T>::operator=(const MatrixExpression<T, Expression>& matrixExpr) noexcept(false)
	{
		if (size() != matrixExpr.size()) [[unlikely]]
		{
			throw mat_expers_size_diff_exception{};
		}

		assign_from_mat_expression(matrixExpr);

		return *this;
	}

	template <std::semiregular T>
	Matrix<T>::Matrix(const Matrix& other)
		: Matrix{ other.size() }
	{
		assign_from_mat_expression(other);
	}

	template <std::semiregular T>
	Matrix<T>& Matrix<T>::operator=(const Matrix& other)
	{
		*this = static_cast<const MatrixExpression<T, Matrix<T>>&>(other);
		return *this;
	}

	template <std::semiregular T>
	Matrix<T>::Matrix(Matrix&& other) noexcept
		: matSize_{ std::exchange(other.matSize_, std::move(MatrixSize{ 0, 0 })) }
		, arr_{ std::exchange(other.arr_, nullptr) }
	{ }

	template <std::semiregular T>
	void Matrix<T>::steal(Matrix&& other) noexcept(false)
	{
		if (size() != other.size()) [[unlikely]]
		{
			throw mat_expers_size_diff_exception{};
		}

		delete[] arr_;
		arr_ = nullptr;
		std::swap(arr_, other.arr_);
		other.matSize_ = std::move(MatrixSize{ 0, 0});
	}

	template <std::semiregular T>
	Matrix<T>::~Matrix()
	{
		delete[] arr_;
	}

	template <std::semiregular T>
	T* Matrix<T>::begin() noexcept
	{
		return arr_;
	}

	template <std::semiregular T>
	T* Matrix<T>::end() noexcept
	{
		return arr_ + matSize_.rows * matSize_.cols;
	}

	template <std::semiregular T>
	T  Matrix<T>::operator()(const int i, const int j) const noexcept
	{
		return arr_[matSize_.cols * i + j];
	}

	template <std::semiregular T>
	T&  Matrix<T>::operator()(const int i, const int j) noexcept
	{
		return arr_[matSize_.cols * i + j];
	}

	template <std::semiregular T>
	MatrixSize Matrix<T>::size() const noexcept
	{
		return matSize_;
	}

	template <std::semiregular T>
	void Matrix<T>::fill(const T val) noexcept
	{
		for (int& elem : *this)
		{
			elem = val;
		}
	}

	template <std::semiregular T>
	bool Matrix<T>::isEmpty() const noexcept
	{
		return arr_ == nullptr;
	}

	template <std::semiregular T, typename Expression>
		requires ExpressionConcept<T, Expression>
	class MatrixTranspose : public MatrixExpression<T, MatrixTranspose<T, Expression>>
	{
	public:
		MatrixTranspose(const Expression& exper)
			: exper_{ exper }
		{ }

		T operator()(const int row, const int col) const noexcept 
		{ 
			return exper_(col, row); 
		}

		MatrixSize size() const noexcept
		{
			MatrixSize matSize{ exper_.size() };
			std::swap(matSize.rows, matSize.cols);
			return matSize;
		}


	private:
		const Expression& exper_;
	};

	template <std::semiregular T, typename Expression>
		requires ExpressionConcept<T, Expression>
	MatrixTranspose<T, Expression> transpose(const MatrixExpression<T, Expression>& matExpr)
	{
		return MatrixTranspose<T, Expression>{ static_cast<const Expression&>(matExpr) };
	}


	template <std::semiregular T, typename Expression1, typename Expression2>
		requires ExpressionConcept<T, Expression1> && ExpressionConcept<T, Expression2>
	class MatrixAdd : public MatrixExpression<T, MatrixAdd<T, Expression1, Expression2>>
	{
	public:
		MatrixAdd(const Expression1& exper1, const Expression2& exper2) noexcept(false)
			: exper1_{ exper1 }
			, exper2_{ exper2 }
		{
			if (exper1_.size() != exper2_.size()) [[unlikely]]
			{
				throw mat_expers_size_diff_exception{};
			}
		}

		T operator()(const int row, const int col) const noexcept
		{ 
			return exper1_(row, col) + exper2_(row, col); 
		}

		MatrixSize size() const noexcept
		{ 
			return exper1_.size(); 
		}

	private:
		const Expression1& exper1_;
		const Expression2& exper2_;
	};

	template <std::semiregular T, typename Expression1, typename Expression2>
		requires ExpressionConcept<T, Expression1>&& ExpressionConcept<T, Expression2>
	MatrixAdd<T, Expression1, Expression2> operator+(
		const MatrixExpression<T, Expression1>& matExpr1, 
		const MatrixExpression<T, Expression2>& matExpr2
		)
	{
		return MatrixAdd<T, Expression1, Expression2>(
			static_cast<const Expression1&>(matExpr1), 
			static_cast<const Expression2&>(matExpr2)
			);
	}
}

#endif // !MAT_EXPRESSION_TEMPLATES