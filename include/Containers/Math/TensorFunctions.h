#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <cmath>

#include "Containers/Math/TensorDefinitions.h"

namespace Containers::Math {

    // Arithmetic operations
    template<typename L, typename R>
    auto operator+(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Add>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator-(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Subtract>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator*(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Multiply>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator/(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Divide>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator%(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Modulo>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator+=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs + rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator-=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs - rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator*=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs * rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator/=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs / rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator%=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs % rhs;
        return lhs;
    }

    template<typename E>
    auto operator+(const TensorExpression<E>& expr) {
        return expr.derived();
    }

    template<typename E>
    auto operator-(const TensorExpression<E>& expr) {
        return TensorUnaryExpression<E, UnaryOperationType::Negate>(expr.derived());
    }

    // Pre increment and decrement
    template<typename E>
    auto operator++(TensorWritableExpression<E>& expr) {
        for(size_t i = 0; i < expr.size(); ++i) ++expr.derived().data()[i];
        return expr.derived();
    }

    template<typename E>
    auto operator--(TensorWritableExpression<E>& expr) {
        for(size_t i = 0; i < expr.size(); ++i) --expr.derived().data()[i];
        return expr.derived();
    }

    // Post increment and decrement
    template<typename E>
    auto operator++(TensorWritableExpression<E>& expr, int) {
        typename FromShape<typename E::ValueType, typename E::ShapeType>::Tensor copy = expr;
        ++expr;
        return copy;
    }

    template<typename E>
    auto operator--(TensorWritableExpression<E>& expr, int) {
        typename FromShape<typename E::ValueType, typename E::ShapeType>::Tensor copy = expr;
        --expr;
        return copy;
    }

    // Bitwise operations
    template<typename L, typename R>
    auto operator&(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::BitwiseAnd>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator|(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::BitwiseOr>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator^(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::BitwiseXor>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator<<(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::LeftShift>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator>>(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::RightShift>(lhs.derived(), rhs.derived());
    }

    template<typename T>
    auto operator~(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOperationType::BitwiseNegate>(expr.derived());
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator&=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator|=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator^=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator<<=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs << rhs;
        return lhs;
    }

    template<typename L, typename R>
    TensorWritableExpression<L>& operator>>=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs >> rhs;
        return lhs;
    }

    // Logical operations
    template<typename L, typename R>
    auto operator&&(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::And>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator||(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Or>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator==(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Equal>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator!=(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::NotEqual>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator<(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Less>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator<=(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::LessOrEqual>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator>(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::Greater>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    auto operator>=(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOperationType::GreaterOrEqual>(lhs.derived(), rhs.derived());
    }

    template<typename T>
    auto operator!(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOperationType::Not>(expr.derived());
    }

    // Scalar operations
    template<typename T, typename = void>
    struct isScalarArithmeticOperand : std::true_type {};

    template<typename T>
    struct isScalarArithmeticOperand<
        T,
        std::void_t<typename std::remove_cvref_t<T>::NotIncludedInScalarArithmetic>
    > : std::false_type {};

    template<typename T>
    concept ScalarArithmeticOperand = isScalarArithmeticOperand<T>::value;

    template<typename T, typename Other>
    using ScalarExprT = ScalarExpression<T, typename TensorExpression<Other>::Derived::ShapeType>;

    // Arithmetic scalar operations
    template<typename L, ScalarArithmeticOperand T>
    auto operator+(const TensorExpression<L>& lhs, T rhs) {
        return lhs + ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator+(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) + rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator+=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs + rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator-(const TensorExpression<L>& lhs, T rhs) {
        return lhs - ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator-(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) - rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator-=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs - rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator*(const TensorExpression<L>& lhs, T rhs) {
        return lhs * ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator*(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) * rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator*=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs * rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator/(const TensorExpression<L>& lhs, T rhs) {
        return lhs / ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator/(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) / rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator/=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs / rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator%(const TensorExpression<L>& lhs, T rhs) {
        return lhs % ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator%(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) % rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator%=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs % rhs;
        return lhs;
    }

    //Bitwise scalar operations
    template<typename L, ScalarArithmeticOperand T>
    auto operator&(const TensorExpression<L>& lhs, T rhs) {
        return lhs & ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator&(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) & rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator&=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator|(const TensorExpression<L>& lhs, T rhs) {
        return lhs | ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator|(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) | rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator|=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator^(const TensorExpression<L>& lhs, T rhs) {
        return lhs ^ ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator^(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) ^ rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator^=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator<<(const TensorExpression<L>& lhs, T rhs) {
        return lhs << ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator<<(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) << rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator<<=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs << rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator>>(const TensorExpression<L>& lhs, T rhs) {
        return lhs >> ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator>>(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) >> rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    TensorWritableExpression<L>& operator>>=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs >> rhs;
        return lhs;
    }

    // Logical scalar operations
    template<typename L, ScalarArithmeticOperand T>
    auto operator&&(const TensorExpression<L>& lhs, T rhs) {
        return lhs && ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator&&(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) && rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator||(const TensorExpression<L>& lhs, T rhs) {
        return lhs || ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator||(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) || rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator==(const TensorExpression<L>& lhs, T rhs) {
        return lhs == ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator==(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) == rhs;
    }
    
    template<typename L, ScalarArithmeticOperand T>
    auto operator!=(const TensorExpression<L>& lhs, T rhs) {
        return lhs != ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator!=(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) != rhs;
    }
    
    template<typename L, ScalarArithmeticOperand T>
    auto operator<(const TensorExpression<L>& lhs, T rhs) {
        return lhs < ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator<(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) < rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator<=(const TensorExpression<L>& lhs, T rhs) {
        return lhs <= ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator<=(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) <= rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator>(const TensorExpression<L>& lhs, T rhs) {
        return lhs > ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator>(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) > rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    auto operator>=(const TensorExpression<L>& lhs, T rhs) {
        return lhs >= ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    auto operator>=(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) >= rhs;
    }

    // Functions
    template<std::integral Result = float, typename L, typename R>
    Result dot(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        Result result{};
        if(&lhs == &rhs) {
            for(size_t i = 0; i < lhs.size(); ++i) {
                auto val = lhs.eval(i);
                result += val * val;
            }
        }
        else {
            for(size_t i = 0; i < lhs.size(); ++i) {
                result += lhs.eval(i) * rhs.eval(i);
            }
        }
        return result;
    }

    template<std::integral Result = float, typename T>
    Result length(const TensorExpression<T>& expr) {
        return std::sqrt(dot<float>(expr, expr));
    }

    template<typename T>
    auto normalize(const TensorExpression<T>& expr) {
        return expr / length(expr);
    }

    template<std::integral Result = float, typename A, typename B>
    Result distance(const TensorExpression<A>& a, const TensorExpression<B>& b) {
        if(&a == &b) return Result{};
        return length(b - a);
    }

    template<typename L, typename R>
    auto cross(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorComplexExpression<ComplexBinaryOperationType::Cross, L, R>(lhs.derived(), rhs.derived());
    }

    template<typename V, typename N>
    auto reflect(const TensorExpression<V>& v, const TensorExpression<N>& n) {
        return v - 2 * dot(v, n) * n;
    }

    template<typename V, typename N>
    auto refract(const TensorExpression<V>& v, const TensorExpression<N>& n, float eta) {
        auto dotProd = dot(v, n);
        auto etaSquared = eta * eta;
        return v * eta - (eta * dotProd + std::sqrt(1 - etaSquared + etaSquared * dotProd * dotProd)) * n;
    }

    // Project A on B
    template<typename A, typename B>
    auto project(const TensorExpression<A>& a, const TensorExpression<B>& b) {
        return (dot(a, b) / dot(b)) * b;
    }

    template<typename A, typename B>
    auto angle(const TensorExpression<A>& a, const TensorExpression<B>& b) {
        return std::acos(dot(a,b)/(length(a) * length(b)));
    }

    template<typename T>
    auto sum(const TensorExpression<T>& expr) {
        typename T::ValueType result{};
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            result += expr.eval(i);
        }
        return result;
    }

    template<typename T>
    auto product(const TensorExpression<T>& expr) {
        typename T::ValueType result = 1;
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            result *= expr.eval(i);
        }
        return result;
    }

    template<typename T>
    auto min(const TensorExpression<T>& expr) {
        auto min = std::numeric_limits<typename T::ValueType>::max();
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            min = std::min(min, expr.eval(i));
        }
        return min;
    }

    template<typename T>
    auto max(const TensorExpression<T>& expr) {
        auto max = std::numeric_limits<typename T::ValueType>::min();
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            max = std::max(max, expr.eval(i));
        }
        return max;
    }

    template<typename L, typename R>
    auto mul(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) requires (
        L::ShapeType::dimCount() == 2 && R::ShapeType::dimCount() == 2 && 
        L::ShapeType::dims()[1] == R::ShapeType::dims()[0]) {
        static constexpr size_t count = L::ShapeType::dims()[1];

        using MultResultType = GetValueTypeFromBinaryOperationType<BinaryOperationType::Multiply, L, R>::Type;
        using AddResultType = decltype(
            ApplyBinaryOp<BinaryOperationType::Add, MultResultType, MultResultType>{}(
                std::declval<MultResultType>(),
                std::declval<MultResultType>()
            )
        );
        using ValueType = AddResultType;
        Tensor<ValueType, L::ShapeType::dims()[0], R::ShapeType::dims()[1]> result;

        for(size_t rowIndex = 0; rowIndex < result.outerSize(); ++rowIndex) {
            auto row = result[rowIndex];
            for(size_t colIndex = 0; colIndex < row.outerSize(); ++colIndex) {
                ValueType& cell = row[colIndex];
                cell = ValueType{};
                for(size_t i = 0; i < count; ++i) {
                    cell += lhs.derived()(rowIndex, i) * rhs.derived()(i, colIndex);
                }
            }
        }

        return result;
    }

    template<typename L, typename R>
    auto mul(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) requires (
        L::ShapeType::dimCount() == 2 && R::ShapeType::dimCount() == 1 && 
        L::ShapeType::dims()[1] == R::ShapeType::dims()[0]) {
        static constexpr size_t count = L::ShapeType::dims()[1];

        using MultResultType = GetValueTypeFromBinaryOperationType<BinaryOperationType::Multiply, L, R>::Type;
        using AddResultType = decltype(
            ApplyBinaryOp<BinaryOperationType::Add, MultResultType, MultResultType>{}(
                std::declval<MultResultType>(),
                std::declval<MultResultType>()
            )
        );
        using ValueType = AddResultType;
        Tensor<ValueType, L::ShapeType::dims()[0]> result;

        for(size_t rowIndex = 0; rowIndex < result.outerSize(); ++rowIndex) {
            ValueType& cell = result[rowIndex];
            cell = ValueType{};
            for(size_t i = 0; i < count; ++i) {
                cell += lhs.derived()(rowIndex, i) * rhs.derived()(i);
            }
        }

        return result;
    }

    template<typename T>
    auto transpose(const TensorExpression<T>& expr) requires (T::ShapeType::dimCount() == 2) {
        Tensor<typename T::ValueType, T::ShapeType::dims()[1], T::ShapeType::dims()[0]> result;
        for(size_t rowIndex = 0; rowIndex < result.outerSize(); ++rowIndex) {
            auto row = expr[rowIndex];
            for(size_t colIndex = 0; colIndex < row.outerSize(); ++colIndex) {
                expr(colIndex, rowIndex) = row[colIndex];
            }
        }
        return result;
    }
}